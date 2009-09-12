/*
 * Copyright (C) 2009 Felipe Contreras
 *
 * Author: Felipe Contreras <felipe.contreras@gmail.com>
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

#ifndef DSP_BRIDGE_H
#define DSP_BRIDGE_H

#include <stdbool.h>
#include <stdint.h>

#define ALLOCATE_HEAP

#define DSP_SUCCEEDED(x) ((int)(x) >= 0)
#define DSP_FAILED(x) ((int)(x) < 0)

#define DSP_MMUFAULT 0x00000010
#define DSP_SYSERROR 0x00000020
#define DSP_NODEMESSAGEREADY 0x00000200

typedef struct
{
	uint32_t field_1;
	uint16_t field_2;
	uint16_t field_3;
	uint8_t field_4;
	uint8_t field_5;
	uint8_t field_6[6];
} dsp_uuid_t;

/* note: cmd = 0x20000000 has special handling */
typedef struct
{
	uint32_t cmd;
	uint32_t arg_1;
	uint32_t arg_2;
} dsp_msg_t;

struct dsp_notification
{
	char *name;
	void *handle;
};

struct dsp_node_attr_in
{
	unsigned long cb;
	int priority;
	unsigned int timeout;
	unsigned int profile_id;
	unsigned int heap_size;
	void *gpp_va;
};

enum dsp_dcd_object_type
{
	DSP_DCD_NODETYPE,
	DSP_DCD_PROCESSORTYPE,
	DSP_DCD_LIBRARYTYPE,
	DSP_DCD_CREATELIBTYPE,
	DSP_DCD_EXECUTELIBTYPE,
	DSP_DCD_DELETELIBTYPE,
};

enum dsp_node_type
{
	DSP_NODE_DEVICE,
	DSP_NODE_TASK,
	DSP_NODE_DAISSOCKET,
	DSP_NODE_MESSAGE,
};

#ifdef ALLOCATE_HEAP
struct DSP_RESOURCEREQMTS {
	unsigned long cbStruct;
	unsigned int uStaticDataSize;
	unsigned int uGlobalDataSize;
	unsigned int uProgramMemSize;
	unsigned int uWCExecutionTime;
	unsigned int uWCPeriod;
	unsigned int uWCDeadline;
	unsigned int uAvgExectionTime;
	unsigned int uMinimumPeriod;
};

struct DSP_NODEPROFS {
	unsigned int ulHeapSize;
};

struct dsp_ndb_props
{
	unsigned long cbStruct;
	dsp_uuid_t uiNodeID;
	char acName[32];
	enum dsp_node_type uNodeType;
	unsigned int bCacheOnGPP;
	struct DSP_RESOURCEREQMTS dspResourceReqmts;
	int iPriority;
	unsigned int uStackSize;
	unsigned int uSysStackSize;
	unsigned int uStackSeg;
	unsigned int uMessageDepth;
	unsigned int uNumInputStreams;
	unsigned int uNumOutputStreams;
	unsigned int uTimeout;
	unsigned int uCountProfiles; /* Number of supported profiles */
	struct DSP_NODEPROFS aProfiles[16];	/* Array of profiles */
	unsigned int uStackSegName; /* Stack Segment Name */
};
#endif

int dsp_open(void);

int dsp_close(int handle);

bool dsp_attach(int handle,
		unsigned int num,
		const void *info,
		void **ret_handle);

bool dsp_detach(int handle,
		void *proc_handle);

bool dsp_node_allocate(int handle,
		       void *proc_handle,
		       const dsp_uuid_t *node_uuid,
		       const void *cb_data,
		       struct dsp_node_attr_in *attrs,
		       void **ret_node);

bool dsp_node_free(int handle,
		   void *node_handle);

bool dsp_node_create(int handle,
		     void *node_handle);

bool dsp_node_run(int handle,
		  void *node_handle);

bool dsp_node_terminate(int handle,
			void *node_handle,
			unsigned long *status);

bool dsp_node_put_message(int handle,
			  void *node_handle,
			  const dsp_msg_t *message,
			  unsigned int timeout);

bool dsp_node_get_message(int handle,
			  void *node_handle,
			  dsp_msg_t *message,
			  unsigned int timeout);

bool dsp_reserve(int handle,
		 void *proc_handle,
		 unsigned long size,
		 void **addr);

bool dsp_unreserve(int handle,
		   void *proc_handle,
		   void *addr);

bool dsp_map(int handle,
	     void *proc_handle,
	     void *mpu_addr,
	     unsigned long size,
	     void *req_addr,
	     void *ret_map_addr,
	     unsigned long attr);

bool dsp_unmap(int handle,
	       void *proc_handle,
	       void *map_addr);

bool dsp_flush(int handle,
	       void *proc_handle,
	       void *mpu_addr,
	       unsigned long size,
	       unsigned long flags);

bool dsp_invalidate(int handle,
		    void *proc_handle,
		    void *mpu_addr,
		    unsigned long size);

bool dsp_register_notify(int handle,
			 void *proc_handle,
			 unsigned int event_mask,
			 unsigned int notify_type,
			 struct dsp_notification *info);

bool dsp_node_register_notify(int handle,
			      void *node_handle,
			      unsigned int event_mask,
			      unsigned int notify_type,
			      struct dsp_notification *info);

bool dsp_wait_for_events(int handle,
			 struct dsp_notification **notifications,
			 unsigned int count,
			 unsigned int *ret_index,
			 unsigned int timeout);

bool dsp_enum(int handle,
	      unsigned int num,
	      struct dsp_ndb_props *info,
	      unsigned int info_size,
	      unsigned int *ret_num);

bool dsp_register(int handle,
		  const dsp_uuid_t *uuid,
		  enum dsp_dcd_object_type type,
		  const char *path);

bool dsp_unregister(int handle,
		    dsp_uuid_t *uuid,
		    enum dsp_dcd_object_type type);

static inline bool
dsp_send_message(int handle,
		 void *node_handle,
		 uint32_t cmd,
		 uint32_t arg_1,
		 uint32_t arg_2)
{
	dsp_msg_t msg;

	msg.cmd = cmd;
	msg.arg_1 = arg_1;
	msg.arg_2 = arg_2;

	return dsp_node_put_message(handle, node_handle, &msg, -1);
}

#endif /* DSP_BRIDGE_H */
