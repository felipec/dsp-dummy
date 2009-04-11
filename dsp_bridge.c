#define NEW_BRIDGE
#include "dsp_bridge.h"

/* for open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h> /* for close */
#include <sys/ioctl.h> /* for ioctl */
#include <stdlib.h> /* for free */

#ifdef ALLOCATE_SM
#include <malloc.h> /* for memalign */
#include <sys/mman.h> /* for mmap */
#endif

#if 0
struct enum_proc {
	unsigned int num;
	struct DSP_PROCESSORINFO *info;
	unsigned int info_size;
	unsigned int *num_procs;
};
#endif

int dsp_open(void)
{
	return open("/dev/DspBridge", O_RDWR);
}

int dsp_close(int handle)
{
	return close(handle);
}

/* not used */
#if 0
struct dsp_proc_attach_in
{
	unsigned long struc;
	unsigned int timeout;
};
#endif

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

	return DSP_SUCCEEDED(ioctl(handle, 7, &arg));
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

	return DSP_SUCCEEDED(ioctl(handle, 9, &arg));
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

	return DSP_SUCCEEDED(ioctl(handle, 15, &arg));
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

	return DSP_SUCCEEDED(ioctl(handle, 5, &arg));
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

	return DSP_SUCCEEDED(ioctl(handle, 1, &arg));
}

struct register_object {
	dsp_uuid_t *uuid;
	enum dsp_dcd_object_type type;
	const char *path;
};

bool dsp_register(int handle,
		  dsp_uuid_t *uuid,
		  enum dsp_dcd_object_type type,
		  const char *path)
{
	struct register_object arg = {
		.uuid = uuid,
		.type = type,
		.path = path,
	};

	return DSP_SUCCEEDED(ioctl(handle, 3, &arg));
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

	return DSP_SUCCEEDED(ioctl(handle, 4, &arg));
}

struct node_create {
	void *node_handle;
};

bool dsp_node_create(int handle,
		     void *node_handle)
{
	struct node_create arg = {
		.node_handle = node_handle,
	};

	return DSP_SUCCEEDED(ioctl(handle, 28, &arg));
}

struct node_run {
	void *node_handle;
};

bool dsp_node_run(int handle,
		  void *node_handle)
{
	struct node_run arg = {
		.node_handle = node_handle,
	};

	return DSP_SUCCEEDED(ioctl(handle, 36, &arg));
}

struct node_terminate {
	void *node_handle;
	unsigned long *status;
};

bool dsp_node_terminate(int handle,
			void *node_handle,
			unsigned long *status)
{
	struct node_terminate arg = {
		.node_handle = node_handle,
		.status = status,
	};

	return DSP_SUCCEEDED(ioctl(handle, 37, &arg));
}

struct node_put_message {
	void *node_handle;
	const dsp_msg_t *message;
	unsigned int timeout;
};

bool dsp_node_put_message(int handle,
			  void *node_handle,
			  const dsp_msg_t *message,
			  unsigned int timeout)
{
	struct node_put_message arg = {
		.node_handle = node_handle,
		.message = message,
		.timeout = timeout,
	};

	return DSP_SUCCEEDED(ioctl(handle, 34, &arg));
}

struct node_get_message {
	void *node_handle;
	dsp_msg_t *message;
	unsigned int timeout;
};

bool dsp_node_get_message(int handle,
			  void *node_handle,
			  dsp_msg_t *message,
			  unsigned int timeout)
{
	struct node_get_message arg = {
		.node_handle = node_handle,
		.message = message,
		.timeout = timeout,
	};

	return DSP_SUCCEEDED(ioctl(handle, 32, &arg));
}

struct node_delete {
	void *node_handle;
};

static inline bool dsp_node_delete(int handle,
				   void *node_handle)
{
	struct node_delete arg = {
		.node_handle = node_handle,
	};

	return DSP_SUCCEEDED(ioctl(handle, 29, &arg));
}

#ifdef ALLOCATE_SM
enum dsp_node_state
{
	NODE_ALLOCATED,
	NODE_CREATED,
	NODE_RUNNING,
	NODE_PAUSED,
	NODE_DONE
};

enum dsp_connect_type
{
	CONNECTTYPE_NODEOUTPUT,
	CONNECTTYPE_GPPOUTPUT,
	CONNECTTYPE_NODEINPUT,
	CONNECTTYPE_GPPINPUT
};

struct dsp_stream_connect
{
	unsigned long cb;
	enum dsp_connect_type type;
	unsigned int index;
	void *node;
	dsp_uuid_t node_id;
	unsigned int stream_index;
};

struct dsp_node_info
{
	unsigned long cb;
	struct dsp_ndb_props props;
	unsigned int priority;
	enum dsp_node_state state;
	void *owner;
	unsigned int num_streams;
	struct dsp_stream_connect streams[16];
	unsigned int node_env;
};

struct dsp_node_attr
{
	unsigned long cb;
	struct dsp_node_attr_in attr_in;
	unsigned long inputs;
	unsigned long outputs;
	struct dsp_node_info info;
};

struct node_get_attr {
	void *node_handle;
	struct dsp_node_attr *attr;
	unsigned int attr_size;
};

static inline bool dsp_node_get_attr(int handle,
				     void *node_handle,
				     struct dsp_node_attr *attr,
				     size_t attr_size)
{
	struct node_get_attr arg = {
		.node_handle = node_handle,
		.attr = attr,
		.attr_size = attr_size,
	};

	return DSP_SUCCEEDED(ioctl(handle, 31, &arg));
}

struct dsp_buffer_attr
{
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
				      void *node_handle,
				      size_t size,
				      struct dsp_buffer_attr *attr,
				      void **buffer)
{
	struct node_alloc_buf arg = {
		.node_handle = node_handle,
		.size = size,
		.attr = attr,
		.buffer = buffer,
	};

	if (!DSP_SUCCEEDED(ioctl(handle, 25, &arg))) {
		*buffer = NULL;
		return false;
	}

	return true;
}
#endif

#ifdef ALLOCATE_SM
struct dsp_cmm_seg_info
{
	unsigned long base_pa;
	unsigned long size;
	unsigned long gpp_base_pa;
	unsigned long gpp_size;
	unsigned long dsp_base_va;
	unsigned long dsp_size;
	unsigned long use_count;
	unsigned long base_va;
};

struct dsp_cmm_info
{
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

	if (!DSP_SUCCEEDED(ioctl(handle, 52, &cmm_arg))) {
		return false;
	}

	cmm_info_arg.cmm = cmm;
	if (!DSP_SUCCEEDED(ioctl(handle, 53, &cmm_info_arg))) {
		return false;
	}

	return true;
}

static inline bool allocate_segments(int handle,
				     void *proc_handle,
				     void *node_handle)
{
	struct dsp_cmm_info cmm_info;
	struct dsp_node_attr attr;
	DSP_NODETYPE node_type;

	if (!get_cmm_info(handle, proc_handle, &cmm_info)) {
		return false;
	}

	if (!dsp_node_get_attr(handle, node_handle, &attr, sizeof(attr))) {
		return false;
	}

	node_type = attr.info.props.uNodeType;

	if ((node_type != NODE_DEVICE) && (cmm_info.segments > 0))
	{
		struct dsp_cmm_seg_info *seg;

		seg = &cmm_info.info[0];

		if (seg->base_pa != 0 && seg->size > 0)
		{
			void *base;
			struct dsp_buffer_attr buffer_attr;

			base = mmap(NULL, seg->size,
				    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED,
				    handle, seg->base_pa);

			if (!base)
				return false;

			buffer_attr.alignment = 0;
			buffer_attr.segment = 1 | 0x10000000;
			buffer_attr.cb = 0;
			if (!dsp_node_alloc_buf(handle, node_handle, seg->size,
						&buffer_attr, &base))
			{
				munmap(base, seg->size);
				return false;
			}
		}
	}

	return true;
}
#endif

#if 0
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

	return DSP_SUCCEEDED(ioctl(handle, 38, &arg));
}
#endif

#if 0
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
		       const struct dsp_cb_data *cb_data,
		       struct dsp_node_attr_in *attrs,
		       void **ret_node)
{
	void *node_handle = NULL;
	struct node_allocate arg = {
		.proc_handle = proc_handle,
		.node_id = node_uuid,
		.cb_data = cb_data,
		.attrs = attrs,
		.ret_node = &node_handle,
	};

	void *virtual = NULL;

#if 0
	if (attrs) {
		struct dsp_ndb_props props;
		unsigned int heap_size = 0;

		if (!get_uuid_props(handle, proc_handle, node_uuid, &props)) {
			attrs->gpp_va = NULL;
			return false;
		}

		if (attrs->profile_id < props.uCountProfiles) {
			heap_size = props.aProfiles[attrs->profile_id].ulHeapSize;
			if (heap_size) {
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

	if (!DSP_SUCCEEDED(ioctl(handle, 24, &arg))) {
		free(virtual);
		return false;
	}

#ifdef ALLOCATE_SM
	if (!allocate_segments(handle, proc_handle, node_handle)) {
		dsp_node_delete(handle, node_handle);
		return false;
	}
#endif

	*ret_node = node_handle;

	return true;
}

bool dsp_node_free(int handle,
		   void *node_handle)
{
#ifdef ALLOCATE_SM
	struct dsp_cmm_info cmm_info;
	struct dsp_node_attr attr;
	DSP_NODETYPE node_type;

	if (!get_cmm_info(handle, NULL, &cmm_info)) {
		return false;
	}

	if (!dsp_node_get_attr(handle, node_handle, &attr, sizeof(attr))) {
		return false;
	}

	node_type = attr.info.props.uNodeType;

	if (node_type != NODE_DEVICE) {
		struct dsp_cmm_seg_info *seg;

		seg = &cmm_info.info[0];

		if (seg->base_pa != 0 && seg->size > 0) {
			void *base = NULL;
			struct dsp_buffer_attr buffer_attr;

			buffer_attr.alignment = 0;
			buffer_attr.segment = 1 | 0x10000000;
			if (!dsp_node_alloc_buf(handle, node_handle, 1,
						&buffer_attr, &base))
			{
				return false;
			}

			if (base && munmap(base, seg->size) < 0) {
				return false;
			}
		}
	}
#endif

	dsp_node_delete(handle, node_handle);

#if 0
	free(attr.attr_in.gpp_va);
#endif

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

	return DSP_SUCCEEDED(ioctl(handle, 17, &arg));
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

	return DSP_SUCCEEDED(ioctl(handle, 18, &arg));
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

#if 0
	unsigned long i;
	unsigned long last_byte;
	/* Physical memory pages are reserved by the
	 * driver (get_user_pages), so no need to
	 * reserve here. Ensure that physical memory
	 * pages are reserved */
	size_t page_size = getpagesize();
	for (i = 0; i < size; i += page_size) {
		*(volatile unsigned char *)(mpu_addr + i) = *(unsigned char *)(mpu_addr + i);
	}
	/* Non page-aligned size: Write final byte */
	last_byte = mpu_addr + size - 1;
	*(volatile unsigned char *)(last_byte) = *(unsigned char *)(last_byte);
#endif
	return DSP_SUCCEEDED(ioctl(handle, 19, &arg));
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

	return DSP_SUCCEEDED(ioctl(handle, 20, &arg));
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

	return DSP_SUCCEEDED(ioctl(handle, 21, &arg));
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

	return DSP_SUCCEEDED(ioctl(handle, 23, &arg));
}
