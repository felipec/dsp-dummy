/*
 * Copyright (C) 2009 Nokia Corporation.
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

#ifndef LOG_H
#define LOG_H

extern int debug_level;

void
pr_helper(unsigned int level,
	  const char *file,
	  const char *function,
	  unsigned int line,
	  const char *fmt,
	  ...);

#define pr_base(level, ...) pr_helper(level, __FILE__, __func__, __LINE__, __VA_ARGS__)

#define pr_err(...) pr_base(0, __VA_ARGS__)
#define pr_warning(...) pr_base(1, __VA_ARGS__)
#define pr_info(...) pr_base(2, __VA_ARGS__)
#define pr_debug(...) pr_base(3, __VA_ARGS__)

#endif /* LOG_H */
