/*
 * Copyright (C) 2009-2010 Felipe Contreras
 *
 * Author: Felipe Contreras <felipe.contreras@gmail.com>
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1, a copy of which is found in LICENSE included in the
 * packaging of this file.
 */

#ifndef DSP_BRIDGE_H
#define DSP_BRIDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define ALLOCATE_HEAP

#define DSP_MMUFAULT 0x00000010
#define DSP_SYSERROR 0x00000020
#define DSP_NODEMESSAGEREADY 0x00000200

#define MAX_PROFILES 16
#define DSP_MAXNAMELEN 32

struct dsp_uuid {
	uint32_t field_1;
	uint16_t field_2;
	uint16_t field_3;
	uint8_t field_4;
	uint8_t field_5;
	uint8_t field_6[6];
};

struct dsp_node {
	void *handle;
	void *heap;
	void *msgbuf_addr;
	size_t msgbuf_size;
};

/* note: cmd = 0x20000000 has special handling */
struct dsp_msg {
	uint32_t cmd;
	uint32_t arg_1;
	uint32_t arg_2;
};

struct dsp_notification {
	char *name;
	void *handle;
};

struct dsp_node_attr_in {
	unsigned long cb;
	int priority;
	unsigned int timeout;
	unsigned int profile_id;
	unsigned int heap_size;
	void *gpp_va;
};

enum dsp_dcd_object_type {
	DSP_DCD_NODETYPE,
	DSP_DCD_PROCESSORTYPE,
	DSP_DCD_LIBRARYTYPE,
	DSP_DCD_CREATELIBTYPE,
	DSP_DCD_EXECUTELIBTYPE,
	DSP_DCD_DELETELIBTYPE,
};

enum dsp_node_type {
	DSP_NODE_DEVICE,
	DSP_NODE_TASK,
	DSP_NODE_DAISSOCKET,
	DSP_NODE_MESSAGE,
};

#ifdef ALLOCATE_HEAP
/* The dsp_resourcereqmts structure for node's resource requirements */
struct dsp_resourcereqmts {
	uint32_t cb_struct;
	uint32_t static_data_size;
	uint32_t global_data_size;
	uint32_t program_mem_size;
	uint32_t uwc_execution_time;
	uint32_t uwc_period;
	uint32_t uwc_deadline;
	uint32_t avg_exection_time;
	uint32_t minimum_period;
};

struct dsp_nodeprofs {
	uint32_t heap_size;
};

/* The dsp_ndb_props structure reports the attributes of a node */
struct dsp_ndb_props {
	uint32_t cb_struct;
	struct dsp_uuid node_id;
	char ac_name[DSP_MAXNAMELEN];
	enum dsp_node_type ntype;
	uint32_t cache_on_gpp;
	struct dsp_resourcereqmts dsp_resource_reqmts;
	int32_t prio;
	uint32_t stack_size;
	uint32_t sys_stack_size;
	uint32_t stack_seg;
	uint32_t message_depth;
	uint32_t num_input_streams;
	uint32_t num_output_streams;
	uint32_t timeout;
	uint32_t count_profiles; /* Number of supported profiles */
	struct dsp_nodeprofs node_profiles[MAX_PROFILES]; /* Array of profiles */
	uint32_t stack_seg_name; /* Stack Segment Name */
};
#endif

enum dsp_resource {
	DSP_RESOURCE_DYNDARAM = 0,
	DSP_RESOURCE_DYNSARAM,
	DSP_RESOURCE_DYNEXTERNAL,
	DSP_RESOURCE_DYNSRAM,
	DSP_RESOURCE_PROCLOAD,
};

struct dsp_info {
	unsigned long cb;
	enum dsp_resource type;
	union {
		unsigned long resource;
		struct {
			unsigned long size;
			unsigned long total_free_size;
			unsigned long len_max_free_block;
			unsigned long free_blocks;
			unsigned long alloc_blocks;
		} mem;
		struct {
			unsigned long load;
			unsigned long pred_load;
			unsigned long freq;
			unsigned long pred_freq;
		} proc;
	} result;
};

enum dsp_connect_type {
	CONNECTTYPE_NODEOUTPUT,
	CONNECTTYPE_GPPOUTPUT,
	CONNECTTYPE_NODEINPUT,
	CONNECTTYPE_GPPINPUT
};

struct dsp_stream_connect {
	unsigned long cb;
	enum dsp_connect_type type;
	unsigned int index;
	void *node_handle;
	struct dsp_uuid node_id;
	unsigned int stream_index;
};

enum dsp_stream_mode {
	STRMMODE_PROCCOPY,
	STRMMODE_ZEROCOPY,
	STRMMODE_LDMA,
	STRMMODE_RDMA
};

struct dsp_stream_attr {
	unsigned int seg_id;
	unsigned int buf_size;
	unsigned int num_bufs;
	unsigned int alignment;
	unsigned int timeout;
	enum dsp_stream_mode mode;
	unsigned int dma_chnl_id;
	unsigned int dma_priority;
};

struct dsp_stream_attr_in {
	unsigned long cb;
	unsigned int timeout;
	unsigned int segment;
	unsigned int alignment;
	unsigned int num_bufs;
	enum dsp_stream_mode mode;
	unsigned int dma_chnl_id;
	unsigned int dma_priority;
};

enum dsp_stream_state {
	STREAM_IDLE,
	STREAM_READY,
	STREAM_PENDING,
	STREAM_DONE
};

struct dsp_stream_info {
	unsigned long cb;
	unsigned int num_bufs_allowed;
	unsigned int num_bufs_in_stream;
	unsigned long num_bytes;
	void *sync_handle;
	enum dsp_stream_state state;
};

enum dsp_node_state {
	NODE_ALLOCATED,
	NODE_CREATED,
	NODE_RUNNING,
	NODE_PAUSED,
	NODE_DONE
};

struct dsp_node_info {
	unsigned long cb;
	struct dsp_ndb_props props;
	unsigned int priority;
	enum dsp_node_state state;
	void *owner;
	unsigned int num_streams;
	struct dsp_stream_connect streams[16];
	unsigned int node_env;
};

struct dsp_node_attr {
	unsigned long cb;
	struct dsp_node_attr_in attr_in;
	unsigned long inputs;
	unsigned long outputs;
	struct dsp_node_info info;
};

int dsp_open(void);

int dsp_close(int handle);

bool dsp_attach(int handle,
		unsigned int num,
		const void *info,
		void **ret_handle);

bool dsp_detach(int handle,
		void *proc_handle);

bool dsp_start(int handle,
		void *proc_handle);

bool dsp_stop(int handle,
		void *proc_handle);

bool dsp_load(int handle,
		void *proc_handle,
		int argc, char **argv,
		char **env);

bool dsp_node_allocate(int handle,
		void *proc_handle,
		const struct dsp_uuid *node_uuid,
		const void *cb_data,
		struct dsp_node_attr_in *attrs,
		struct dsp_node **ret_node);

bool dsp_node_free(int handle,
		struct dsp_node *node);

bool dsp_node_connect(int handle,
		struct dsp_node *node,
		unsigned int stream,
		struct dsp_node *other_node,
		unsigned int other_stream,
		struct dsp_stream_attr *attrs,
		void *params);

bool dsp_node_create(int handle,
		struct dsp_node *node);

bool dsp_node_run(int handle,
		struct dsp_node *node);

bool dsp_node_terminate(int handle,
		struct dsp_node *node,
		unsigned long *status);

bool dsp_node_put_message(int handle,
		struct dsp_node *node,
		const struct dsp_msg *message,
		unsigned int timeout);

bool dsp_node_get_message(int handle,
		struct dsp_node *node,
		struct dsp_msg *message,
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
		struct dsp_node *node,
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
		size_t info_size,
		unsigned int *ret_num);

bool dsp_register(int handle,
		const struct dsp_uuid *uuid,
		enum dsp_dcd_object_type type,
		const char *path);

bool dsp_unregister(int handle,
		const struct dsp_uuid *uuid,
		enum dsp_dcd_object_type type);

bool dsp_proc_get_info(int handle,
		void *proc_handle,
		enum dsp_resource type,
		struct dsp_info *info,
		unsigned size);

static inline bool
dsp_send_message(int handle,
		struct dsp_node *node,
		uint32_t cmd,
		uint32_t arg_1,
		uint32_t arg_2)
{
	struct dsp_msg msg;

	msg.cmd = cmd;
	msg.arg_1 = arg_1;
	msg.arg_2 = arg_2;

	return dsp_node_put_message(handle, node, &msg, -1);
}

bool dsp_node_get_attr(int handle,
		struct dsp_node *node,
		struct dsp_node_attr *attr,
		size_t attr_size);

bool dsp_enum_nodes(int handle,
		void *proc_handle,
		void **node_table,
		unsigned node_table_size,
		unsigned *num_nodes,
		unsigned *allocated);

bool dsp_stream_open(int handle,
		struct dsp_node *node,
		unsigned int direction,
		unsigned int index,
		struct dsp_stream_attr_in *attrin,
		void *stream);

bool dsp_stream_close(int handle,
		void *stream);

bool dsp_stream_idle(int handle,
		void *stream,
		bool flush);

bool dsp_stream_reclaim(int handle,
		void *stream,
		unsigned char **buff,
		unsigned long *data_size,
		unsigned long *buff_size,
		unsigned long *args);

bool dsp_stream_issue(int handle,
		void *stream,
		unsigned char *buff,
		unsigned long data_size,
		unsigned long buff_size,
		unsigned long arg);

bool dsp_stream_get_info(int handle,
		void *stream,
		struct dsp_stream_info *info,
		unsigned int size);

bool dsp_stream_allocate_buffers(int handle,
		void *stream,
		unsigned int size,
		unsigned char **buff,
		unsigned int num_buf);

bool dsp_stream_free_buffers(int handle,
		void *stream,
		unsigned char **buff,
		unsigned int num_buf);

#endif /* DSP_BRIDGE_H */
