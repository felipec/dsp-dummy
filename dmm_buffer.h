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

#include "log.h"

#include <dbapi.h>

#define DMM_PAGE_SIZE 4096
#define ARM_BUFFER_ALIGNMENT 128
#define ROUND_UP(num, scale) (((num) + ((scale) - 1)) & ~((scale) - 1))

typedef struct
{
	void *node;
	void *data;
	void *allocated_data;
	unsigned long size;
	void *reserve;
	void *map;
} dmm_buffer_t;

static inline dmm_buffer_t *
dmm_buffer_new(void *node)
{
	dmm_buffer_t *b;
	b = calloc(1, sizeof(*b));

	b->node = node;

	return b;
}

static inline void
dmm_buffer_free(dmm_buffer_t *b)
{
	pr_debug("%p", b);
	free(b->allocated_data);
	free(b);
}

static inline void
dmm_buffer_map(dmm_buffer_t *b)
{
	pr_debug("%p", b);
	unsigned int to_reserve;
	to_reserve = ROUND_UP(b->size, DMM_PAGE_SIZE) + (2 * DMM_PAGE_SIZE);
	DSPProcessor_ReserveMemory(b->node, to_reserve, &b->reserve);
	DSPProcessor_Map(b->node, b->data, b->size, b->reserve, &b->map, 0);
}

static inline void
dmm_buffer_unmap(dmm_buffer_t *b)
{
	pr_debug("%p", b);
	DSPProcessor_UnMap(b->node, b->map);
	DSPProcessor_UnReserveMemory(b->node, b->reserve);
}

static inline void
dmm_buffer_flush(dmm_buffer_t *b)
{
	pr_debug("%p", b);
	DSPProcessor_FlushMemory(b->node, b->data, b->size, 0);
}

static inline void
dmm_buffer_invalidate(dmm_buffer_t *b)
{
	pr_debug("%p", b);
	DSPProcessor_InvalidateMemory(b->node, b->data, b->size);
}

static inline void
dmm_buffer_allocate(dmm_buffer_t *b,
		    unsigned int size)
{
#ifdef ARM_BUFFER_ALIGNMENT
	b->allocated_data = malloc(size + 2 * ARM_BUFFER_ALIGNMENT);
	b->data = (void *) ROUND_UP((unsigned long) b->allocated_data, ARM_BUFFER_ALIGNMENT);
#else
	b->data = b->allocated_data = malloc(size);
#endif
	b->size = size;
	dmm_buffer_map(b);
}

static inline void
dmm_buffer_use(dmm_buffer_t *b,
	       void *data,
	       unsigned int size)
{
	b->data = data;
	b->size = size;
	dmm_buffer_map(b);
}

#endif /* DMM_BUFFER_H */
