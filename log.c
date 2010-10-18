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

#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define SYSLOG

#ifdef SYSLOG
#include <syslog.h>
#endif

#ifdef DEBUG
unsigned debug_level = 2;
#endif

#ifdef SYSLOG
static inline int
log_level_to_syslog(unsigned int level)
{
	switch (level) {
	case 0: return LOG_ERR;
	case 1: return LOG_WARNING;
	case 2:
	case 3: return LOG_INFO;
	default: return LOG_DEBUG;
	}
}
#endif

static inline const char *
log_level_to_string(unsigned int level)
{
	switch (level) {
	case 0: return "error"; break;
	case 1: return "warning"; break;
	case 2: return "test"; break;
	case 3: return "info"; break;
	case 4: return "debug"; break;
	default: return NULL; break;
	}
}

void pr_helper(unsigned int level,
		const char *file,
		const char *function,
		unsigned int line,
		const char *fmt,
		...)
{
	char *tmp;
	va_list args;

#ifdef DEBUG
	if (level > debug_level)
		return;
#endif

	va_start(args, fmt);

	if (vasprintf(&tmp, fmt, args) < 0)
		goto leave;

	if (level <= 1) {
#ifdef SYSLOG
		syslog(log_level_to_syslog(level), "%s", tmp);
#endif
		fprintf(stderr, "%s: %s: %s\n",
				log_level_to_string(level), function, tmp);
	}
	else if (level == 2)
		fprintf(stderr, "%s: %s:%s(%u): %s\n",
				log_level_to_string(level), file, function, line, tmp);
#if defined(DEVEL) || defined(DEBUG)
	else if (level == 3)
		fprintf(stderr, "%s: %s: %s\n",
				log_level_to_string(level), function, tmp);
#endif
#ifdef DEBUG
	else if (level == 4)
		fprintf(stderr, "%s: %s:%s(%u): %s\n",
				log_level_to_string(level), file, function, line, tmp);
#endif

	free(tmp);

leave:
	va_end(args);
}
