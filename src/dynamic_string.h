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

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * @return Execution status
 */
bool dynStrAppendChar(dynStr_t *string, char c);

/**
 * Appends a string to a dynamic string
 * @param string Dynamic string
 * @param str String to append
 * @return Execution status
 */
bool dynStrAppendString(dynStr_t *string, const char* str);

/**
 * Determines whether two dynamic strings have the same value
 * @param string1 First dynamic string to compare
 * @param string2 Second dynamic string to compare
 * @return String equality
 */
bool dynStrEqual(dynStr_t* string1, dynStr_t *string2);

/**
 * Determines whether dynamic string and string have the same value
 * @param string Dynamic string
 * @param str String to compare
 * @return String equality
 */
bool dynStrEqualString(dynStr_t* string, const char* str);

/**
 * Checks if the dynamic string is empty
 * @param string Dynamic string
 * @return Is dynamic string empty?
 */
bool dynStrIsEmpty(dynStr_t *string);

/**
 * Copies the dynamic string
 * @param dst Destination
 * @param src Source
 * @return Execution status
 */
bool dynStrCopy(dynStr_t *dst, dynStr_t *src);

/**
 * Clones the dynamic string
 * @param src Dynamic string
 * @return Cloned dynamic string
 */
dynStr_t *dynStrClone(dynStr_t *src);

/**
 * Escape the dynamic string
 * @param string Dynamic string
 * @return Execution status
 */
bool dynStrEscape(dynStr_t *string);

/**
 * Returns a character from the string
 * @param string Dynamic string
 * @param index Character position in the string
 * @return Character
 */
char dynStrGetChar(dynStr_t *string, unsigned long index);

/**
 * Returns a string value of the dynamic string
 * @param string Dynamic string
 * @return String value of the dynamic string
 */
const char *dynStrGetString(dynStr_t* string);
