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

#include "dmm_buffer.h"

#include <stdlib.h>

DmmBuffer *
dmm_buffer_new (DSP_HNODE handle)
{
    DmmBuffer *buffer;
    buffer = calloc (1, sizeof (DmmBuffer));

    buffer->handle = handle;

    return buffer;
}

void
dmm_buffer_free (DmmBuffer *buffer)
{
#ifdef DEBUG
    printf ("%s: %p\n", __func__, buffer);
#endif
    DSPProcessor_UnMap (buffer->handle, buffer->map);
    DSPProcessor_UnReserveMemory (buffer->handle, buffer->reserve);
    free (buffer->allocated_data);
    free (buffer);
}

void
dmm_buffer_allocate (DmmBuffer *buffer,
                     unsigned int size)
{
    buffer->data = buffer->allocated_data = malloc (size);
    buffer->size = size;
    dmm_buffer_map (buffer);
}

void
dmm_buffer_use (DmmBuffer *buffer,
                void *data,
                unsigned int size)
{
    buffer->data = data;
    buffer->size = size;
    dmm_buffer_map (buffer);
}
