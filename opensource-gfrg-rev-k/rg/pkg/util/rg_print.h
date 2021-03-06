/****************************************************************************
 *
 * rg/pkg/util/rg_print.h
 * 
 * Copyright (C) Jungo LTD 2004
 * 
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General 
 * Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02111-1307, USA.
 *
 * Developed by Jungo LTD.
 * Residential Gateway Software Division
 * www.jungo.com
 * info@jungo.com
 */
#ifndef _RG_PRINT_H_
#define _RG_PRINT_H_

#include <stdarg.h>
#include <stdlib.h>

int snprintf_l(char *str, size_t size, const char *format, ...)
    __attribute__((__format__(__printf__, 3, 4)));
int console_printf(const char *format, ...)
    __attribute__((__format__(__printf__, 1, 2)));

#endif

