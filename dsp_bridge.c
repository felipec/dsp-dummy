/*
 * Copyright (C) 2009 Felipe Contreras
 * Copyright (C) 2007 Texas Instruments, Incorporated
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

#include "dsp_bridge.h"

/* for open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h> /* for close */
#include <sys/ioctl.h> /* for ioctl */
#include <stdlib.h> /* for free */

#include <malloc.h> /* for memalign */

#define ALLOCATE_SM

#ifdef ALLOCATE_SM
#include <malloc.h> /* for memalign */
#include <sys/mman.h> /* for mmap */
#endif

/*
 * Dspbridge ioctl numbering scheme
 *
 *    7                           0
 *  ---------------------------------
 *  |  Module   |   ioctl Number    |
 *  ---------------------------------
 *  | x | x | x | 0 | 0 | 0 | 0 | 0 |
 *  ---------------------------------
 */

/* ioctl driver identifier */
#define DB 0xDB

/*
 * Following are used to distinguish between module ioctls, this is needed
 * in case new ioctls are introduced.
 */
#define DB_MODULE_MASK 0xE0
#define DB_IOC_MASK    0x1F

#ifdef NEW_API

#include <linux/ioctl.h>

/* ioctl module masks */
#define DB_MGR  0x0
#define DB_PROC 0x20
#define DB_NODE 0x40
#define DB_STRM 0x60
#define DB_CMM  0x80

/* Used to calculate the ioctl per dspbridge module */
#define DB_IOC(module, num) \
	(((module) & DB_MODULE_MASK) | ((num) & DB_IOC_MASK))

#else /* NEW_API */

#define DB_MGR  1
#define DB_PROC 7
#define DB_NODE 24
#define DB_STRM 39
#define DB_CMM  50

#define DB_IOC(module, num)	((module) + (num))

#undef _IOR
#undef _IOW
#undef _IOWR

#define _IOR(type, nr, size)	(nr)
#define _IOW(type, nr, size)	(nr)
#define _IOWR(type, nr, size)	(nr)

#endif /* NEW_API */

/* MGR Module */
#define MGR_WAIT		_IOWR(DB, DB_IOC(DB_MGR, 4), unsigned long)
#define MGR_ENUMNODE_INFO	_IOWR(DB, DB_IOC(DB_MGR, 0), unsigned long)
#define MGR_REGISTEROBJECT	_IOWR(DB, DB_IOC(DB_MGR, 2), unsigned long)
#define MGR_UNREGISTEROBJECT	_IOWR(DB, DB_IOC(DB_MGR, 3), unsigned long)

/* PROC Module */
#define PROC_ATTACH		_IOWR(DB, DB_IOC(DB_PROC, 0), unsigned long)
/* PROC_DETACH Deprecated */
#define PROC_DETACH		_IOR(DB, DB_IOC(DB_PROC, 2), unsigned long)
#define PROC_REGISTERNOTIFY	_IOWR(DB, DB_IOC(DB_PROC, 8), unsigned long)
#define PROC_RSVMEM		_IOWR(DB, DB_IOC(DB_PROC, 10), unsigned long)
#define PROC_UNRSVMEM		_IOW(DB, DB_IOC(DB_PROC, 11), unsigned long)
#define PROC_MAPMEM		_IOWR(DB, DB_IOC(DB_PROC, 12), unsigned long)
#define PROC_UNMAPMEM		_IOR(DB, DB_IOC(DB_PROC, 13), unsigned long)
#define PROC_FLUSHMEMORY	_IOW(DB, DB_IOC(DB_PROC, 14), unsigned long)
#define PROC_INVALIDATEMEMORY	_IOW(DB, DB_IOC(DB_PROC, 16), unsigned long)
#define PROC_GET_STATE		_IOWR(DB, DB_IOC(DB_PROC, 5), unsigned long)
#define PROC_ENUMRESOURCES	_IOWR(DB, DB_IOC(DB_PROC, 4), unsigned long)

/* NODE Module */
#define NODE_REGISTERNOTIFY	_IOWR(DB, DB_IOC(DB_NODE, 11), unsigned long)
#define NODE_CREATE		_IOW(DB, DB_IOC(DB_NODE, 4), unsigned long)
#define NODE_RUN		_IOW(DB, DB_IOC(DB_NODE, 12), unsigned long)
#define NODE_TERMINATE		_IOWR(DB, DB_IOC(DB_NODE, 13), unsigned long)
#define NODE_PUTMESSAGE		_IOW(DB, DB_IOC(DB_NODE, 10), unsigned long)
#define NODE_GETMESSAGE		_IOWR(DB, DB_IOC(DB_NODE, 8), unsigned long)
#define NODE_DELETE		_IOW(DB, DB_IOC(DB_NODE, 5), unsigned long)
#define NODE_GETATTR		_IOWR(DB, DB_IOC(DB_NODE, 7), unsigned long)
#define NODE_ALLOCMSGBUF	_IOWR(DB, DB_IOC(DB_NODE, 1), unsigned long)
#define NODE_GETUUIDPROPS	_IOWR(DB, DB_IOC(DB_NODE, 14), unsigned long)
#define NODE_ALLOCATE		_IOWR(DB, DB_IOC(DB_NODE, 0), unsigned long)

/* CMM Module */
#define CMM_GETHANDLE		_IOR(DB, DB_IOC(DB_CMM, 2), unsigned long)
#define CMM_GETINFO		_IOR(DB, DB_IOC(DB_CMM, 3), unsigned long)

int dsp_open(void)
{
	return open("/dev/DspBridge", O_RDWR);
}

int dsp_close(int handle)
{
	return close(handle);
}

struct proc_attach {
	unsigned int num;
	const void *info; /* not used */
	void **ret_handle;
};

bool dsp_attach(int handle,
		unsigned int num,
		const void *info,
		void **ret_handle)
{
	struct proc_attach arg = {
		.num = num,
		.info = info,
		.ret_handle = ret_handle,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_ATTACH, &arg));
}

struct proc_detach {
	void *proc_handle;
};

bool dsp_detach(int handle,
		void *proc_handle)
{
	struct proc_detach arg = {
		.proc_handle = proc_handle,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_DETACH, &arg));
}

struct register_notify {
	void *proc_handle;
	unsigned int event_mask;
	unsigned int notify_type;
	struct dsp_notification *info;
};

bool dsp_register_notify(int handle,
			 void *proc_handle,
			 unsigned int event_mask,
			 unsigned int notify_type,
			 struct dsp_notification *info)
{
	struct register_notify arg = {
		.proc_handle = proc_handle,
		.event_mask = event_mask,
		.notify_type = notify_type,
		.info = info,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_REGISTERNOTIFY, &arg));
}

struct node_register_notify {
	void *node_handle;
	unsigned int event_mask;
	unsigned int notify_type;
	struct dsp_notification *info;
};

bool dsp_node_register_notify(int handle,
			      dsp_node_t *node,
			      unsigned int event_mask,
			      unsigned int notify_type,
			      struct dsp_notification *info)
{
	struct node_register_notify arg = {
		.node_handle = node->handle,
		.event_mask = event_mask,
		.notify_type = notify_type,
		.info = info,
	};

	return DSP_SUCCEEDED(ioctl(handle, NODE_REGISTERNOTIFY, &arg));
}

struct wait_for_events {
	struct dsp_notification **notifications;
	unsigned int count;
	unsigned int *ret_index;
	unsigned int timeout;
};

bool dsp_wait_for_events(int handle,
			 struct dsp_notification **notifications,
			 unsigned int count,
			 unsigned int *ret_index,
			 unsigned int timeout)
{
	struct wait_for_events arg = {
		.notifications = notifications,
		.count = count,
		.ret_index = ret_index,
		.timeout = timeout,
	};

	return DSP_SUCCEEDED(ioctl(handle, MGR_WAIT, &arg));
}

struct enum_node {
	unsigned int num;
	struct dsp_ndb_props *info;
	unsigned int info_size;
	unsigned int *ret_num;
};

bool dsp_enum(int handle,
	      unsigned int num,
	      struct dsp_ndb_props *info,
	      size_t info_size,
	      unsigned int *ret_num)
{
	struct enum_node arg = {
		.num = num,
		.info = info,
		.info_size = info_size,
		.ret_num = ret_num,
	};

	return DSP_SUCCEEDED(ioctl(handle, MGR_ENUMNODE_INFO, &arg));
}

struct register_object {
	const dsp_uuid_t *uuid;
	enum dsp_dcd_object_type type;
	const char *path;
};

bool dsp_register(int handle,
		  const dsp_uuid_t *uuid,
		  enum dsp_dcd_object_type type,
		  const char *path)
{
	struct register_object arg = {
		.uuid = uuid,
		.type = type,
		.path = path,
	};

	return DSP_SUCCEEDED(ioctl(handle, MGR_REGISTEROBJECT, &arg));
}

struct unregister_object {
	dsp_uuid_t *uuid;
	enum dsp_dcd_object_type type;
};

bool dsp_unregister(int handle,
		    dsp_uuid_t *uuid,
		    enum dsp_dcd_object_type type)
{
	struct unregister_object arg = {
		.uuid = uuid,
		.type = type,
	};

	return DSP_SUCCEEDED(ioctl(handle, MGR_UNREGISTEROBJECT, &arg));
}

struct node_create {
	void *node_handle;
};

bool dsp_node_create(int handle,
		     dsp_node_t *node)
{
	struct node_create arg = {
		.node_handle = node->handle,
	};

	return DSP_SUCCEEDED(ioctl(handle, NODE_CREATE, &arg));
}

struct node_run {
	void *node_handle;
};

bool dsp_node_run(int handle,
		  dsp_node_t *node)
{
	struct node_run arg = {
		.node_handle = node->handle,
	};

	return DSP_SUCCEEDED(ioctl(handle, NODE_RUN, &arg));
}

struct node_terminate {
	void *node_handle;
	unsigned long *status;
};

bool dsp_node_terminate(int handle,
			dsp_node_t *node,
			unsigned long *status)
{
	struct node_terminate arg = {
		.node_handle = node->handle,
		.status = status,
	};

	return DSP_SUCCEEDED(ioctl(handle, NODE_TERMINATE, &arg));
}

struct node_put_message {
	void *node_handle;
	const dsp_msg_t *message;
	unsigned int timeout;
};

bool dsp_node_put_message(int handle,
			  dsp_node_t *node,
			  const dsp_msg_t *message,
			  unsigned int timeout)
{
	struct node_put_message arg = {
		.node_handle = node->handle,
		.message = message,
		.timeout = timeout,
	};

	return DSP_SUCCEEDED(ioctl(handle, NODE_PUTMESSAGE, &arg));
}

struct node_get_message {
	void *node_handle;
	dsp_msg_t *message;
	unsigned int timeout;
};

bool dsp_node_get_message(int handle,
			  dsp_node_t *node,
			  dsp_msg_t *message,
			  unsigned int timeout)
{
	struct node_get_message arg = {
		.node_handle = node->handle,
		.message = message,
		.timeout = timeout,
	};

	return DSP_SUCCEEDED(ioctl(handle, NODE_GETMESSAGE, &arg));
}

struct node_delete {
	void *node_handle;
};

static inline bool dsp_node_delete(int handle,
				   dsp_node_t *node)
{
	struct node_delete arg = {
		.node_handle = node->handle,
	};

	return DSP_SUCCEEDED(ioctl(handle, NODE_DELETE, &arg));
}

#ifdef ALLOCATE_SM
struct node_get_attr {
	void *node_handle;
	struct dsp_node_attr *attr;
	unsigned int attr_size;
};

bool dsp_node_get_attr(int handle,
		       dsp_node_t *node,
		       struct dsp_node_attr *attr,
		       size_t attr_size)
{
	struct node_get_attr arg = {
		.node_handle = node->handle,
		.attr = attr,
		.attr_size = attr_size,
	};

	return DSP_SUCCEEDED(ioctl(handle, NODE_GETATTR, &arg));
}

struct dsp_buffer_attr {
	unsigned long cb;
	unsigned int segment;
	unsigned int alignment;
};

struct node_alloc_buf {
	void *node_handle;
	unsigned int size;
	struct dsp_buffer_attr *attr;
	void **buffer;
};

static inline bool dsp_node_alloc_buf(int handle,
				      dsp_node_t *node,
				      size_t size,
				      struct dsp_buffer_attr *attr,
				      void **buffer)
{
	struct node_alloc_buf arg = {
		.node_handle = node->handle,
		.size = size,
		.attr = attr,
		.buffer = buffer,
	};

	if (!DSP_SUCCEEDED(ioctl(handle, NODE_ALLOCMSGBUF, &arg))) {
		*buffer = NULL;
		return false;
	}

	return true;
}

struct dsp_cmm_seg_info {
	unsigned long base_pa;
	unsigned long size;
	unsigned long gpp_base_pa;
	unsigned long gpp_size;
	unsigned long dsp_base_va;
	unsigned long dsp_size;
	unsigned long use_count;
	unsigned long base_va;
};

struct dsp_cmm_info {
	unsigned long segments;
	unsigned long use_count;
	unsigned long min_block_size;
	struct dsp_cmm_seg_info info[1];
};

struct cmm_get_handle {
	void *proc_handle;
	struct cmm_object **cmm;
};

struct cmm_get_info {
	struct cmm_object *cmm;
	struct dsp_cmm_info *info;
};

static inline bool get_cmm_info(int handle,
				void *proc_handle,
				struct dsp_cmm_info *cmm_info)
{
	struct cmm_object *cmm;
	struct cmm_get_handle cmm_arg = {
		.proc_handle = proc_handle,
		.cmm = &cmm,
	};
	struct cmm_get_info cmm_info_arg = {
		.info = cmm_info,
	};

	if (!DSP_SUCCEEDED(ioctl(handle, CMM_GETHANDLE, &cmm_arg)))
		return false;

	cmm_info_arg.cmm = cmm;
	if (!DSP_SUCCEEDED(ioctl(handle, CMM_GETINFO, &cmm_info_arg)))
		return false;

	return true;
}

static inline bool allocate_segments(int handle,
				     void *proc_handle,
				     dsp_node_t *node)
{
	struct dsp_cmm_info cmm_info;
	struct dsp_node_attr attr;
	enum dsp_node_type node_type;

	if (!get_cmm_info(handle, proc_handle, &cmm_info))
		return false;

	if (!dsp_node_get_attr(handle, node, &attr, sizeof(attr)))
		return false;

	node_type = attr.info.props.uNodeType;

	if ((node_type != DSP_NODE_DEVICE) && (cmm_info.segments > 0)) {
		struct dsp_cmm_seg_info *seg;

		seg = &cmm_info.info[0];

		if (seg->base_pa != 0 && seg->size > 0) {
			void *base;
			struct dsp_buffer_attr buffer_attr;

			base = mmap(NULL, seg->size,
				    PROT_READ | PROT_WRITE, MAP_SHARED | 0x2000 /* MAP_LOCKED */,
				    handle, seg->base_pa);

			if (!base)
				return false;

			buffer_attr.alignment = 0;
			buffer_attr.segment = 1 | 0x10000000;
			buffer_attr.cb = 0;
			if (!dsp_node_alloc_buf(handle, node, seg->size,
						&buffer_attr, &base))
			{
				munmap(base, seg->size);
				return false;
			}

			node->msgbuf_addr = base;
			node->msgbuf_size = seg->size;
		}
	}

	return true;
}
#endif

#ifdef ALLOCATE_HEAP
struct get_uuid_props {
	void *proc_handle;
	const dsp_uuid_t *node_uuid;
	struct dsp_ndb_props *props;
};

static inline bool get_uuid_props(int handle,
				  void *proc_handle,
				  const dsp_uuid_t *node_uuid,
				  struct dsp_ndb_props *props)
{
	struct get_uuid_props arg = {
		.proc_handle = proc_handle,
		.node_uuid = node_uuid,
		.props = props,
	};

	return DSP_SUCCEEDED(ioctl(handle, NODE_GETUUIDPROPS, &arg));
}

#define PG_SIZE_4K 4096
#define PG_MASK(pg_size) (~((pg_size)-1))
#define PG_ALIGN_LOW(addr, pg_size) ((addr) & PG_MASK(pg_size))
#define PG_ALIGN_HIGH(addr, pg_size) (((addr)+(pg_size)-1) & PG_MASK(pg_size))
#endif

struct node_allocate {
	void *proc_handle;
	const dsp_uuid_t *node_id;
	const void *cb_data;
	struct dsp_node_attr_in *attrs;
	void **ret_node;
};

bool dsp_node_allocate(int handle,
		       void *proc_handle,
		       const dsp_uuid_t *node_uuid,
		       const void *cb_data,
		       struct dsp_node_attr_in *attrs,
		       dsp_node_t **ret_node)
{
	dsp_node_t *node;
	void *node_handle = NULL;
	struct node_allocate arg = {
		.proc_handle = proc_handle,
		.node_id = node_uuid,
		.cb_data = cb_data,
		.attrs = attrs,
		.ret_node = &node_handle,
	};

#ifdef ALLOCATE_HEAP
	if (attrs) {
		struct dsp_ndb_props props;

		if (!get_uuid_props(handle, proc_handle, node_uuid, &props)) {
			attrs->gpp_va = NULL;
			return false;
		}

		if (attrs->profile_id < props.uCountProfiles) {
			unsigned int heap_size = 0;

			heap_size = props.aProfiles[attrs->profile_id].ulHeapSize;
			if (heap_size) {
				void *virtual = NULL;

				heap_size = PG_ALIGN_HIGH(heap_size, PG_SIZE_4K);
				virtual = memalign(128, heap_size);
				if (!virtual)
					return false;
				attrs->heap_size = heap_size;
				attrs->gpp_va = virtual;
			}
		}
	}
#endif

	if (!DSP_SUCCEEDED(ioctl(handle, NODE_ALLOCATE, &arg))) {
		if (attrs) {
			free(attrs->gpp_va);
			attrs->gpp_va = NULL;
		}
		return false;
	}

	node = calloc(1, sizeof(*node));
	node->handle = node_handle;
	node->heap = attrs->gpp_va;

#ifdef ALLOCATE_SM
	if (!allocate_segments(handle, proc_handle, node)) {
		dsp_node_delete(handle, node);
		return false;
	}
#endif

	*ret_node = node;

	return true;
}

bool dsp_node_free(int handle,
		   dsp_node_t *node)
{
	munmap(node->msgbuf_addr, node->msgbuf_size);
	dsp_node_delete(handle, node);
	free(node->heap);
	free(node);

	return true;
}

struct reserve_mem {
	void *proc_handle;
	unsigned long size;
	void **addr;
};

bool dsp_reserve(int handle,
		 void *proc_handle,
		 unsigned long size,
		 void **addr)
{
	struct reserve_mem arg = {
		.proc_handle = proc_handle,
		.size = size,
		.addr = addr,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_RSVMEM, &arg));
}

struct unreserve_mem {
	void *proc_handle;
	unsigned long size;
	void *addr;
};

bool dsp_unreserve(int handle,
		   void *proc_handle,
		   void *addr)
{
	struct unreserve_mem arg = {
		.proc_handle = proc_handle,
		.addr = addr,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_UNRSVMEM, &arg));
}

struct map_mem {
	void *proc_handle;
	void *mpu_addr;
	unsigned long size;
	void *req_addr;
	void **ret_map_addr;
	unsigned long attr;
};

bool dsp_map(int handle,
	     void *proc_handle,
	     void *mpu_addr,
	     unsigned long size,
	     void *req_addr,
	     void *ret_map_addr,
	     unsigned long attr)
{
	struct map_mem arg = {
		.proc_handle = proc_handle,
		.mpu_addr = mpu_addr,
		.size = size,
		.req_addr = req_addr,
		.ret_map_addr = ret_map_addr,
		.attr = attr,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_MAPMEM, &arg));
}

struct unmap_mem {
	void *proc_handle;
	unsigned long size;
	void *map_addr;
};

bool dsp_unmap(int handle,
	       void *proc_handle,
	       void *map_addr)
{
	struct unmap_mem arg = {
		.proc_handle = proc_handle,
		.map_addr = map_addr,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_UNMAPMEM, &arg));
}

struct flush_mem {
	void *proc_handle;
	void *mpu_addr;
	unsigned long size;
	unsigned long flags;
};

bool dsp_flush(int handle,
	       void *proc_handle,
	       void *mpu_addr,
	       unsigned long size,
	       unsigned long flags)
{
	struct flush_mem arg = {
		.proc_handle = proc_handle,
		.mpu_addr = mpu_addr,
		.size = size,
		.flags = flags,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_FLUSHMEMORY, &arg));
}

struct invalidate_mem {
	void *proc_handle;
	void *mpu_addr;
	unsigned long size;
};

bool dsp_invalidate(int handle,
		    void *proc_handle,
		    void *mpu_addr,
		    unsigned long size)
{
	struct invalidate_mem arg = {
		.proc_handle = proc_handle,
		.mpu_addr = mpu_addr,
		.size = size,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_INVALIDATEMEMORY, &arg));
}

struct proc_get_info {
	void *proc_handle;
	unsigned type;
	struct dsp_info *info;
	unsigned size;
};

bool dsp_proc_get_info(int handle,
		       void *proc_handle,
		       unsigned type,
		       struct dsp_info *info,
		       unsigned size)
{
	struct proc_get_info arg = {
		.proc_handle = proc_handle,
		.type = type,
		.info = info,
		.size = size,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_GET_STATE, &arg));
}

struct enum_nodes {
	void *proc_handle;
	void **node_table;
	unsigned node_table_size;
	unsigned *num_nodes;
	unsigned *allocated;
};

bool dsp_enum_nodes(int handle,
		    void *proc_handle,
		    void **node_table,
		    unsigned node_table_size,
		    unsigned *num_nodes,
		    unsigned *allocated)
{
	struct enum_nodes arg = {
		.proc_handle = proc_handle,
		.node_table = node_table,
		.node_table_size = node_table_size,
		.num_nodes = num_nodes,
		.allocated = allocated,
	};

	return DSP_SUCCEEDED(ioctl(handle, PROC_ENUMRESOURCES, &arg));
}
