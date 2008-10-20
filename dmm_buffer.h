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

#ifndef DMM_BUFFER_H
#define DMM_BUFFER_H

#include <dbapi.h>

#define DMM_PAGE_SIZE 4096
#define ROUND_TO_PAGESIZE(n) ((((n) + 4095) / DMM_PAGE_SIZE) * DMM_PAGE_SIZE)

typedef struct
{
    DSP_HNODE handle;
    void *data;
    void *allocated_data;
    unsigned long size;
    void *reserve;
    void *map;
} DmmBuffer;

DmmBuffer *dmm_buffer_new (DSP_HNODE handle);

void dmm_buffer_free (DmmBuffer *buffer);

void dmm_buffer_allocate (DmmBuffer *buffer, unsigned int size);

void dmm_buffer_use (DmmBuffer *buffer, void *data, unsigned int size);

static inline void
dmm_buffer_map (DmmBuffer *buffer)
{
#ifdef DEBUG
    printf ("%s: %p\n", __func__, buffer);
#endif
    unsigned int to_reserve;
    to_reserve = ROUND_TO_PAGESIZE (buffer->size) + (2 * DMM_PAGE_SIZE);
    DSPProcessor_ReserveMemory (buffer->handle, to_reserve, &buffer->reserve);
    DSPProcessor_Map (buffer->handle, buffer->data, buffer->size,
                      buffer->reserve, &buffer->map, 0);
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

#endif /* DMM_BUFFER_H */
