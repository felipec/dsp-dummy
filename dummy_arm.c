/*
 * Copyright (C) 2009-2010 Felipe Contreras
 * Copyright (C) 2009-2010 Nokia Corporation
 * Copyright (C) 2009 Igalia S.L
 *
 * Author: Felipe Contreras <felipe.contreras@nokia.com>
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1, a copy of which is found in LICENSE included in the
 * packaging of this file.
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include "dmm_buffer.h"
#include "dsp_bridge.h"
#include "log.h"

static unsigned long input_buffer_size = 0x1000;
static unsigned long output_buffer_size = 0x1000;
static bool done;
static int ntimes;

static int dsp_handle;
static void *proc;

static void
signal_handler(int signal)
{
	done = true;
}

static inline struct dsp_node *
create_node(void)
{
	struct dsp_node *node;
	const struct dsp_uuid dummy_uuid = { 0x3dac26d0, 0x6d4b, 0x11dd, 0xad, 0x8b,
		{ 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66 } };

	if (!dsp_register(dsp_handle, &dummy_uuid, DSP_DCD_LIBRARYTYPE, "/lib/dsp/dummy.dll64P"))
		return false;

	if (!dsp_register(dsp_handle, &dummy_uuid, DSP_DCD_NODETYPE, "/lib/dsp/dummy.dll64P"))
		return false;

	if (!dsp_node_allocate(dsp_handle, proc, &dummy_uuid, NULL, NULL, &node)) {
		pr_err("dsp node allocate failed");
		return NULL;
	}

	if (!dsp_node_create(dsp_handle, node)) {
		pr_err("dsp node create failed");
		return NULL;
	}

	pr_info("dsp node created");

	return node;
}

static inline bool
destroy_node(struct dsp_node *node)
{
	if (node) {
		if (!dsp_node_free(dsp_handle, node)) {
			pr_err("dsp node free failed");
			return false;
		}

		pr_info("dsp node deleted");
	}

	return true;
}

static inline void
configure_dsp_node(void *node,
		dmm_buffer_t *input_buffer,
		dmm_buffer_t *output_buffer)
{
	struct dsp_msg msg;

	msg.cmd = 0;
	msg.arg_1 = (uint32_t) input_buffer->map;
	msg.arg_2 = (uint32_t) output_buffer->map;
	dsp_node_put_message(dsp_handle, node, &msg, -1);
}

static bool
run_task(struct dsp_node *node,
		unsigned long times)
{
	unsigned long exit_status;

	dmm_buffer_t *input_buffer;
	dmm_buffer_t *output_buffer;

	if (!dsp_node_run(dsp_handle, node)) {
		pr_err("dsp node run failed");
		return false;
	}

	pr_info("dsp node running");

	input_buffer = dmm_buffer_new(dsp_handle, proc, DMA_TO_DEVICE);
	output_buffer = dmm_buffer_new(dsp_handle, proc, DMA_FROM_DEVICE);

	dmm_buffer_allocate(input_buffer, input_buffer_size);
	dmm_buffer_allocate(output_buffer, output_buffer_size);

	dmm_buffer_map(output_buffer);
	dmm_buffer_map(input_buffer);

	configure_dsp_node(node, input_buffer, output_buffer);

	pr_info("running %lu times", times);

	while (!done) {
		struct dsp_msg msg;

#ifdef FILL_DATA
		{
			static unsigned char foo = 1;
			unsigned int i;
			for (i = 0; i < input_buffer->size; i++)
				((char *) input_buffer->data)[i] = foo;
			foo++;
		}
#endif
		dmm_buffer_begin(input_buffer, input_buffer->size);
		dmm_buffer_begin(output_buffer, output_buffer->size);
		msg.cmd = 1;
		msg.arg_1 = input_buffer->size;
		dsp_node_put_message(dsp_handle, node, &msg, -1);
		dsp_node_get_message(dsp_handle, node, &msg, -1);
		dmm_buffer_end(input_buffer, input_buffer->size);
		dmm_buffer_end(output_buffer, output_buffer->size);

		if (--times == 0)
			break;
	}

	dmm_buffer_unmap(output_buffer);
	dmm_buffer_unmap(input_buffer);

	dmm_buffer_free(output_buffer);
	dmm_buffer_free(input_buffer);

	if (!dsp_node_terminate(dsp_handle, node, &exit_status)) {
		pr_err("dsp node terminate failed: %lx", exit_status);
		return false;
	}

	pr_info("dsp node terminated");

	return true;
}

static void handle_options(int *argc, const char ***argv)
{
	while (*argc > 0) {
		const char *cmd = (*argv)[0];
		if (cmd[0] != '-')
			break;

#ifdef DEBUG
		if (!strcmp(cmd, "-d") || !strcmp(cmd, "--debug"))
			debug_level = 4;
#endif

		if (!strcmp(cmd, "-n") || !strcmp(cmd, "--ntimes")) {
			if (*argc < 2) {
				pr_err("bad option");
				exit(-1);
			}
			ntimes = atoi((*argv)[1]);
			(*argv)++;
			(*argc)--;
		}

		(*argv)++;
		(*argc)--;
	}
}

int main(int argc, const char *argv[])
{
	struct dsp_node *node;
	int ret = 0;

	signal(SIGINT, signal_handler);

#ifdef DEBUG
	debug_level = 3;
#endif
	ntimes = 1000;

	argc--; argv++;
	handle_options(&argc, &argv);

	dsp_handle = dsp_open();

	if (dsp_handle < 0) {
		pr_err("dsp open failed");
		return -1;
	}

	if (!dsp_attach(dsp_handle, 0, NULL, &proc)) {
		pr_err("dsp attach failed");
		ret = -1;
		goto leave;
	}

	node = create_node();
	if (!node) {
		pr_err("dsp node creation failed");
		ret = -1;
		goto leave;
	}

	run_task(node, ntimes);
	destroy_node(node);

leave:
	if (proc) {
		if (!dsp_detach(dsp_handle, proc)) {
			pr_err("dsp detach failed");
			ret = -1;
		}
		proc = NULL;
	}

	if (dsp_handle > 0) {
		if (dsp_close(dsp_handle) < 0) {
			pr_err("dsp close failed");
			return -1;
		}
	}

	return ret;
}
