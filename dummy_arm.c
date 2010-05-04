/*
 * Copyright (C) 2008-2009 Nokia Corporation.
 *
 * Author: Felipe Contreras <felipe.contreras@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
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

static int dsp_handle;
static void *proc;

static void
signal_handler(int signal)
{
	done = true;
}

static inline dsp_node_t *
create_node(void)
{
	dsp_node_t *node;
	const dsp_uuid_t dummy_uuid = { 0x3dac26d0, 0x6d4b, 0x11dd, 0xad, 0x8b,
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
destroy_node(dsp_node_t *node)
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
	dsp_msg_t msg;

	msg.cmd = 0;
	msg.arg_1 = (uint32_t) input_buffer->map;
	msg.arg_2 = (uint32_t) output_buffer->map;
	dsp_node_put_message(dsp_handle, node, &msg, -1);
}

static bool
run_task(dsp_node_t *node,
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

	input_buffer = dmm_buffer_new(dsp_handle, proc);
	output_buffer = dmm_buffer_new(dsp_handle, proc);

	dmm_buffer_allocate(input_buffer, input_buffer_size);
	dmm_buffer_allocate(output_buffer, output_buffer_size);

	configure_dsp_node(node, input_buffer, output_buffer);

	pr_info("running %lu times", times);

	while (!done) {
		dsp_msg_t msg;

#ifdef FILL_DATA
		{
			static unsigned char foo;
			unsigned int i;
			for (i = 0; i < input_buffer->size; i++)
				((char *) input_buffer->data)[i] = foo;
			foo++;
		}
#endif
		dmm_buffer_flush(input_buffer, input_buffer->size);
		msg.cmd = 1;
		msg.arg_1 = input_buffer->size;
		dsp_node_put_message(dsp_handle, node, &msg, -1);
		dsp_node_get_message(dsp_handle, node, &msg, -1);
		dmm_buffer_invalidate(output_buffer, output_buffer->size);

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

static void
handle_options(int *argc,
	       const char ***argv)
{
	while (*argc > 0) {
		const char *cmd = (*argv)[0];
		if (cmd[0] != '-')
			break;

		if (!strcmp(cmd, "-d") || !strcmp(cmd, "--debug"))
			debug_level = 3;

		(*argv)++;
		(*argc)--;
	}
}

int
main(int argc,
     const char **argv)
{
	dsp_node_t *node;
	int ret = 0;

	signal(SIGINT, signal_handler);

	debug_level = 2;

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

	run_task(node, 24 * 60 * 10);
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
