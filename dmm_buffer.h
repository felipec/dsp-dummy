/*
 * Copyright (C) 2008 Nokia Corporation.
 *
 * Authors:
 * Felipe Contreras <felipe.contreras@nokia.com>
 * Marco Ballesio <marco.ballesio@nokia.com>
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

#ifndef DMM_BUFFER_H
#define DMM_BUFFER_H

#include <stdlib.h> /* for calloc, free */

#include <dbapi.h>

#define DMM_PAGE_SIZE 4096
#define ARM_BUFFER_ALIGNMENT 128
#define ROUND_UP(num, scale) (((num) + ((scale) - 1)) & ~((scale) - 1))

typedef struct
{
	DSP_HNODE handle;
	void *data;
	void *allocated_data;
	unsigned long size;
	void *reserve;
	void *map;
} DmmBuffer;

static inline DmmBuffer *
dmm_buffer_new (DSP_HNODE handle)
{
	DmmBuffer *buffer;
	buffer = calloc (1, sizeof (DmmBuffer));

	buffer->handle = handle;

	return buffer;
}

static inline void
dmm_buffer_free (DmmBuffer *buffer)
{
#ifdef DEBUG
	printf ("%s: %p\n", __func__, buffer);
#endif
	free (buffer->allocated_data);
	free (buffer);
}

static inline void
dmm_buffer_map (DmmBuffer *buffer)
{
#ifdef DEBUG
	printf ("%s: %p\n", __func__, buffer);
#endif
	unsigned int to_reserve;
	to_reserve = ROUND_UP (buffer->size, DMM_PAGE_SIZE) + (2 * DMM_PAGE_SIZE);
	DSPProcessor_ReserveMemory (buffer->handle, to_reserve, &buffer->reserve);
	DSPProcessor_Map (buffer->handle, buffer->data, buffer->size,
			  buffer->reserve, &buffer->map, 0);
}

static inline void
dmm_buffer_unmap (DmmBuffer *buffer)
{
#ifdef DEBUG
	printf ("%s: %p\n", __func__, buffer);
#endif
	DSPProcessor_UnMap (buffer->handle, buffer->map);
	DSPProcessor_UnReserveMemory (buffer->handle, buffer->reserve);
}

static inline void
dmm_buffer_flush (DmmBuffer *buffer)
{
#ifdef DEBUG
	printf ("%s: %p\n", __func__, buffer);
#endif
	DSPProcessor_FlushMemory (buffer->handle, buffer->data, buffer->size, 0);
}

static inline void
dmm_buffer_invalidate (DmmBuffer *buffer)
{
#ifdef DEBUG
	printf ("%s: %p\n", __func__, buffer);
#endif
	DSPProcessor_InvalidateMemory (buffer->handle, buffer->data, buffer->size);
}

static inline void
dmm_buffer_allocate (DmmBuffer *buffer,
		     unsigned int size)
{
#ifdef ARM_BUFFER_ALIGNMENT
	buffer->allocated_data = malloc (size + 2 * ARM_BUFFER_ALIGNMENT);
	buffer->data = (void *) ROUND_UP ((unsigned long) buffer->allocated_data, ARM_BUFFER_ALIGNMENT);
#else
	buffer->data = buffer->allocated_data = malloc (size);
#endif
	buffer->size = size;
	dmm_buffer_map (buffer);
}

static inline void
dmm_buffer_use (DmmBuffer *buffer,
		void *data,
		unsigned int size)
{
	buffer->data = data;
	buffer->size = size;
	dmm_buffer_map (buffer);
}

#endif /* DMM_BUFFER_H */
