/*
 * Copyright (C) 2008 Texas Instruments, Incorporated
 * Copyright (C) 2008-2009 Nokia Corporation.
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

#ifndef NODE_H
#define NODE_H

#include <stdint.h>

typedef struct {
	uint32_t cmd;
	uint32_t arg_1;
	uint32_t arg_2;
} dsp_msg_t;

extern unsigned short NODE_getMsg(void *node, dsp_msg_t *msg, unsigned int timeout);
extern unsigned short NODE_putMsg(void *node, void *dest, dsp_msg_t *msg, unsigned int timeout);

extern void BCACHE_inv(void *ptr, size_t size, unsigned short wait);
extern void BCACHE_wbInv(void *ptr, size_t size, unsigned short wait);

#endif /* NODE_H */
