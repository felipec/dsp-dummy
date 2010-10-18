/*
 * Copyright (C) 2009-2010 Felipe Contreras
 * Copyright (C) 2009-2010 Nokia Corporation
 *
 * Author: Felipe Contreras <felipe.contreras@gmail.com>
 *
 * This file may be used under the terms of the GNU Lesser General Public
 * License version 2.1, a copy of which is found in LICENSE included in the
 * packaging of this file.
 */

#ifndef LOG_H
#define LOG_H

#ifdef DEBUG
extern unsigned debug_level;
#endif

void pr_helper(unsigned int level,
		const char *file,
		const char *function,
		unsigned int line,
		const char *fmt,
		...) __attribute__((format(printf, 5, 6)));

#define pr_base(level, ...) pr_helper(level, __FILE__, __func__, __LINE__, __VA_ARGS__)

#define pr_err(...) pr_base(0, __VA_ARGS__)
#define pr_warning(...) pr_base(1, __VA_ARGS__)
#define pr_test(...) pr_base(2, __VA_ARGS__)

#ifdef DEBUG
#define pr_info(...) pr_base(3, __VA_ARGS__)
#define pr_debug(...) pr_base(4, __VA_ARGS__)
#else
#define pr_info(...) ({ if (0) pr_base(3, __VA_ARGS__); })
#define pr_debug(...) ({ if (0) pr_base(4, __VA_ARGS__); })
#endif

#endif /* LOG_H */
