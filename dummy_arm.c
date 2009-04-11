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

typedef struct
{
	DSP_HPROCESSOR processor;
	DSP_HNODE node;
} Core;

static void
signal_handler (int signal)
{
	done = true;
}

static bool
create_node (Core *core)
{
	DSP_STATUS status = DSP_SOK;

	status = DSPNode_Allocate (core->processor, &dummy_uuid, NULL, NULL, &core->node);

	if (DSP_FAILED (status))
		fprintf (stdout, "DSPNode_Allocate failed: 0x%lx\n", status);

	/* create node on the DSP */
	if (DSP_SUCCEEDED (status))
	{
		status = DSPNode_Create (core->node);
		if (DSP_FAILED (status))
			fprintf (stdout, "DSPNode_Create failed: 0x%lx\n", status);
		else
			fprintf (stdout, "DSPNodeCreate succeeded\n");
	}

	return DSP_SUCCEEDED (status) ? true : false;
}

static inline void
configure_dsp_node (Core *core,
		    DmmBuffer *input_buffer,
		    DmmBuffer *output_buffer)
{
	struct DSP_MSG msg;

	msg.dwCmd = 0;
	msg.dwArg1 = (unsigned long) input_buffer->map;
	msg.dwArg2 = (unsigned long) output_buffer->map;
	DSPNode_PutMessage (core->node, &msg, DSP_FOREVER);
}

static bool
run_task (Core *core,
	  unsigned long times)
{
	DSP_STATUS status;
	DSP_STATUS exit_status;

	DmmBuffer *input_buffer;
	DmmBuffer *output_buffer;

	/* start the node */
	status = DSPNode_Run (core->node);
	if (DSP_FAILED (status))
		fprintf (stdout, "DSPNode_Run failed: 0x%lx\n", status);
	else
		fprintf (stdout, "DSPNode_run succeeded\n");

	input_buffer = dmm_buffer_new (core->processor);
	output_buffer = dmm_buffer_new (core->processor);

	dmm_buffer_allocate (input_buffer, input_buffer_size);
	dmm_buffer_allocate (output_buffer, output_buffer_size);

	configure_dsp_node (core, input_buffer, output_buffer);

	fprintf (stdout, "running %lu times\n", times);

	while (!done)
	{
		struct DSP_MSG msg;
#ifdef FILL_DATA
		{
			static unsigned char foo;
			unsigned int i;
			for (i = 0; i < input_buffer->size; i++)
			{
				((char *) input_buffer->data)[i] = foo;
			}
			foo++;
		}
#endif
		dmm_buffer_flush (input_buffer);
		msg.dwCmd = 1;
		msg.dwArg1 = input_buffer->size;
		DSPNode_PutMessage (core->node, &msg, DSP_FOREVER);
		DSPNode_GetMessage (core->node, &msg, DSP_FOREVER);
		dmm_buffer_invalidate (output_buffer);

		if (--times == 0)
			break;
	}

	dmm_buffer_unmap (output_buffer);
	dmm_buffer_unmap (input_buffer);

	dmm_buffer_free (output_buffer);
	dmm_buffer_free (input_buffer);

	status = DSPNode_Terminate (core->node, &exit_status);
	if (DSP_FAILED (status))
		fprintf (stdout, "DSPNode_Terminate failed: 0x%lx\n", status);

	return DSP_SUCCEEDED (status) ? true : false;
}

static bool
destroy_node (Core *core)
{
	DSP_STATUS status = DSP_SOK;

	if (core->node)
	{
		/* delete node */
		status = DSPNode_Delete (core->node);
		if (DSP_FAILED (status))
			fprintf (stdout, "DSPNode_Delete failed: 0x%lx\n", status);
		core->node = NULL;
	}

	return DSP_SUCCEEDED (status) ? true : false;
}

int
main (int argc,
      char **argv)
{
	DSP_STATUS status = DSP_SOK;
	Core core;

	(void) signal (SIGINT, signal_handler);

	/* context initialization */
	core.processor = NULL;

	status = DspManager_Open (0, NULL);
	if (DSP_SUCCEEDED (status))
	{
		/* processor level initialization. */
		status = DSPProcessor_Attach (0, NULL, &core.processor);
		if (DSP_SUCCEEDED (status))
		{
			/* node level initialization. */
			if (create_node (&core))
			{
				run_task (&core, 24 * 60 * 10);
			}
		}
		else
			fprintf (stdout, "DSPProcessor_Attach failed: 0x%lx\n", status);

		destroy_node (&core);

		if (core.processor)
		{
			/* detach from processor */
			status = DSPProcessor_Detach (core.processor);
			if (DSP_FAILED (status))
				fprintf (stdout, "DSPProcessor_Detach failed: 0x%lx\n", status);
			core.processor = NULL;
		}

		status = DspManager_Close (0, NULL);
	}
	else
		fprintf (stdout, "DspManager_Open failed: 0x%lx\n", status);

	return (DSP_SUCCEEDED (status) ? 0 : -1);
}
