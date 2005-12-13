/* debug.h
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdarg.h>
#include <stdio.h>

typedef enum {
    RC_DEBUG_LEVEL_ALWAYS   = -1,
    RC_DEBUG_LEVEL_NONE     = 0,
    RC_DEBUG_LEVEL_ERROR    = 1,
    RC_DEBUG_LEVEL_CRITICAL = 2,
    RC_DEBUG_LEVEL_WARNING  = 3,
    RC_DEBUG_LEVEL_MESSAGE  = 4,
    RC_DEBUG_LEVEL_INFO     = 5,
    RC_DEBUG_LEVEL_DEBUG    = 6,
} DebugLevel;

#define DEBUG_LEVEL_ALWAYS   RC_DEBUG_LEVEL_ALWAYS
#define DEBUG_LEVEL_NONE     RC_DEBUG_LEVEL_NONE
#define DEBUG_LEVEL_ERROR    RC_DEBUG_LEVEL_ERROR
#define DEBUG_LEVEL_CRITICAL RC_DEBUG_LEVEL_CRITICAL
#define DEBUG_LEVEL_WARNING  RC_DEBUG_LEVEL_WARNING
#define DEBUG_LEVEL_MESSAGE  RC_DEBUG_LEVEL_MESSAGE
#define DEBUG_LEVEL_INFO     RC_DEBUG_LEVEL_INFO
#define DEBUG_LEVEL_DEBUG    RC_DEBUG_LEVEL_DEBUG

void debug_full (DebugLevel  level, const char   *format, ...);

#ifdef RC_DEBUG_VERBOSE

const char *debug_helper (const char *format, ...);

#define debug(level, format...) \
	debug_full (level, "%s (%s, %s:%d)", debug_helper (format), __FUNCTION__, __FILE__, __LINE__)

#else

#define debug debug_full

#endif

#define rc_debug debug

#endif /* _DEBUG_H */
