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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <dbapi.h>

#include "dmm_buffer.h"

static unsigned long input_buffer_size = 0x1000;
static unsigned long output_buffer_size = 0x1000;
static bool done;

static const struct DSP_UUID dummy_uuid = { 0x3dac26d0, 0x6d4b, 0x11dd, 0xad, 0x8b, { 0x08, 0x00, 0x20, 0x0c, 0x9a,0x66 } };

static void *proc;

static void
signal_handler(int signal)
{
	done = true;
}

static void *
create_node(void)
{
	void *node;
	DSP_STATUS status = DSP_SOK;

	status = DSPNode_Allocate(proc, &dummy_uuid, NULL, NULL, &node);

	if (DSP_FAILED(status))
		fprintf(stdout, "DSPNode_Allocate failed: 0x%lx\n", status);

	/* create node on the DSP */
	if (DSP_SUCCEEDED(status)) {
		status = DSPNode_Create(node);
		if (DSP_FAILED(status))
			fprintf(stdout, "DSPNode_Create failed: 0x%lx\n", status);
		else
			fprintf(stdout, "DSPNodeCreate succeeded\n");
	}

	return DSP_SUCCEEDED(status) ? node : NULL;
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
	DSPNode_PutMessage(node, &msg, DSP_FOREVER);
}

static bool
run_task(void *node,
	 unsigned long times)
{
	DSP_STATUS status;
	DSP_STATUS exit_status;

	dmm_buffer_t *input_buffer;
	dmm_buffer_t *output_buffer;

	/* start the node */
	status = DSPNode_Run(node);
	if (DSP_FAILED(status))
		fprintf(stdout, "DSPNode_Run failed: 0x%lx\n", status);
	else
		fprintf(stdout, "DSPNode_run succeeded\n");

	input_buffer = dmm_buffer_new(proc);
	output_buffer = dmm_buffer_new(proc);

	dmm_buffer_allocate(input_buffer, input_buffer_size);
	dmm_buffer_allocate(output_buffer, output_buffer_size);

	configure_dsp_node(node, input_buffer, output_buffer);

	fprintf(stdout, "running %lu times\n", times);

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
		DSPNode_PutMessage(node, &msg, DSP_FOREVER);
		DSPNode_GetMessage(node, &msg, DSP_FOREVER);
		dmm_buffer_invalidate(output_buffer);

		if (--times == 0)
			break;
	}

	dmm_buffer_unmap(output_buffer);
	dmm_buffer_unmap(input_buffer);

	dmm_buffer_free(output_buffer);
	dmm_buffer_free(input_buffer);

	status = DSPNode_Terminate(node, &exit_status);
	if (DSP_FAILED(status))
		fprintf(stdout, "DSPNode_Terminate failed: 0x%lx\n", status);

	return DSP_SUCCEEDED(status) ? true : false;
}

static bool
destroy_node(void *node)
{
	DSP_STATUS status = DSP_SOK;

	if (node) {
		status = DSPNode_Delete(node);
		if (DSP_FAILED(status))
			fprintf(stdout, "DSPNode_Delete failed: 0x%lx\n", status);
	}

	return DSP_SUCCEEDED(status) ? true : false;
}

int
main(int argc,
     char **argv)
{
	DSP_STATUS status = DSP_SOK;

	(void) signal(SIGINT, signal_handler);

	status = DspManager_Open(0, NULL);
	if (DSP_SUCCEEDED(status)) {
		void *node;

		status = DSPProcessor_Attach(0, NULL, &proc);
		if (DSP_SUCCEEDED(status)) {
			if ((node = create_node())) {
				run_task(node, 24 * 60 * 10);
			}
		}
		else
			fprintf(stdout, "DSPProcessor_Attach failed: 0x%lx\n", status);

		destroy_node(node);

		if (proc) {
			status = DSPProcessor_Detach(proc);
			if (DSP_FAILED(status))
				fprintf(stdout, "DSPProcessor_Detach failed: 0x%lx\n", status);
			proc = NULL;
		}

		status = DspManager_Close(0, NULL);
	}
	else
		fprintf(stdout, "DspManager_Open failed: 0x%lx\n", status);

	return (DSP_SUCCEEDED(status) ? 0 : -1);
}
