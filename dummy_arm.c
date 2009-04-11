/*
 * Copyright (C) 2008 Nokia Corporation.
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

#include <dbapi.h>

#include "dmm_buffer.h"
#include "log.h"

static unsigned long input_buffer_size = 0x1000;
static unsigned long output_buffer_size = 0x1000;
static bool done;

static void *proc;

static void
signal_handler(int signal)
{
	done = true;
}

static inline void *
create_node(void)
{
	void *node;
	const struct DSP_UUID dummy_uuid = { 0x3dac26d0, 0x6d4b, 0x11dd, 0xad, 0x8b,
		{ 0x08, 0x00, 0x20, 0x0c, 0x9a,0x66 } };

	if (DSP_FAILED(DSPNode_Allocate(proc, &dummy_uuid, NULL, NULL, &node))) {
		pr_err("dsp node allocate failed");
		return NULL;
	}

	if (DSP_FAILED(DSPNode_Create(node))) {
		pr_err("dsp node create failed");
		return NULL;
	}

	pr_info("dsp node created");

	return node;
}

static inline bool
destroy_node(void *node)
{
	if (node) {
		if (DSP_FAILED(DSPNode_Delete(node))) {
			pr_err("dsp node delete failed");
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
	struct DSP_MSG msg;

	msg.dwCmd = 0;
	msg.dwArg1 = (unsigned long) input_buffer->map;
	msg.dwArg2 = (unsigned long) output_buffer->map;
	DSPNode_PutMessage(node, &msg, -1);
}

static bool
run_task(void *node,
	 unsigned long times)
{
	DSP_STATUS exit_status;

	dmm_buffer_t *input_buffer;
	dmm_buffer_t *output_buffer;

	if (DSP_FAILED(DSPNode_Run(node))) {
		pr_err("dsp node run failed");
		return false;
	}

	pr_info("dsp node running");

	input_buffer = dmm_buffer_new(proc);
	output_buffer = dmm_buffer_new(proc);

	dmm_buffer_allocate(input_buffer, input_buffer_size);
	dmm_buffer_allocate(output_buffer, output_buffer_size);

	configure_dsp_node(node, input_buffer, output_buffer);

	pr_info("running %lu times", times);

	while (!done) {
		struct DSP_MSG msg;

#ifdef FILL_DATA
		{
			static unsigned char foo;
			unsigned int i;
			for (i = 0; i < input_buffer->size; i++) {
				((char *) input_buffer->data)[i] = foo;
			}
			foo++;
		}
#endif
		dmm_buffer_flush(input_buffer);
		msg.dwCmd = 1;
		msg.dwArg1 = input_buffer->size;
		DSPNode_PutMessage(node, &msg, -1);
		DSPNode_GetMessage(node, &msg, -1);
		dmm_buffer_invalidate(output_buffer);

		if (--times == 0)
			break;
	}

	dmm_buffer_unmap(output_buffer);
	dmm_buffer_unmap(input_buffer);

	dmm_buffer_free(output_buffer);
	dmm_buffer_free(input_buffer);

	if (DSP_FAILED(DSPNode_Terminate(node, &exit_status))) {
		pr_err("dsp node terminate failed: %lx", exit_status);
		return false;
	}

	pr_info("dsp node terminated");

	return true;
}

int
main(int argc,
     char **argv)
{
	void *node;
	int ret = 0;

	(void) signal(SIGINT, signal_handler);

#ifdef DEBUG
	debug_level = 3;
#else
	debug_level = 2;
#endif

	if (DSP_FAILED(DspManager_Open(0, NULL))) {
		pr_err("dsp open failed");
		return -1;
	}

	if (DSP_FAILED(DSPProcessor_Attach(0, NULL, &proc))) {
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
		if (DSP_FAILED(DSPProcessor_Detach(proc))) {
			pr_err("dsp detach failed");
			ret = 1;
			goto leave;
		}
		proc = NULL;
	}

	if (DSP_FAILED(DspManager_Close(0, NULL))) {
		pr_err("dsp close failed");
		return -1;
	}

	return ret;
}
