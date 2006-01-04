/* debug.cc
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


#include <zypp/solver/detail/debug.h>

#include <string>
#include <stdarg.h>
#include <stdio.h>

using namespace std;

const char *
debug_helper (const char *format, ...)
{
    va_list args;
    static char *str = NULL;

    if (str != NULL) free ((void *)str);

    va_start (args, format);
    vasprintf (&str, format, args);
    va_end (args);

    return str;
}

void
debug_full (DebugLevel level, const char *format, ...)
{
    va_list args;
    char *str = NULL;

    va_start (args, format);
    vasprintf (&str, format, args);
    va_end (args);

    printf ("%s\n", str); fflush (stdout);

    free ((void *)str);
}
