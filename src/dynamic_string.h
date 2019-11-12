/*
 * Copyright (C) 2019 Roman Ondráček <xondra58@stud.fit.vutbr.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#pragma once

#include <stdlib.h>

#define DYN_STR_LENGTH 8

typedef struct dynamic_string {
	char* string;
	unsigned long size;
	unsigned long alloc_size;
} dynStr_t;

/**
 * Initializes a dynamic string
 * @return Initialized dynamic string
 */
dynStr_t *dynStrInit();

/**
 * Clears a dynamic string
 * @param string Dynamic string
 */
void dynStrClear(dynStr_t *string);

/**
 * Frees a dynamic string
 * @param string Dynamic string
 */
void dynStrFree(dynStr_t *string);

/**
 * Appends a char to a dynamic string
 * @param string Dynamic string
 * @param c Char to append
 */
void dynStrAppendChar(dynStr_t *string, char c);

/**
 * Appends a string to a dynamic string
 * @param string Dynamic string
 * @param str String to append
 */
void dynStrAppendString(dynStr_t *string, const char* str);
