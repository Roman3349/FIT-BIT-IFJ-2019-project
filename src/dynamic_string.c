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

#include "dynamic_string.h"

dynStr_t *dynStrInit() {
	dynStr_t *string = malloc(sizeof(dynStr_t));
	if (string == NULL) {
		// TODO: Add error message
		return NULL;
	}
	string->string = malloc(DYN_STR_LENGTH);
	if (string->string == NULL) {
		// TODO: Add error message
		return NULL;
	}
	string->alloc_size = DYN_STR_LENGTH;
	dynStrClear(string);
	return string;
}

void dynStrClear(dynStr_t *string) {
	assert(string != NULL);
	string->string[0] = 0;
	string->size = 0;
}

void dynStrFree(dynStr_t *string) {
	if (string != NULL) {
		free(string->string);
		free(string);
	}
}

void dynStrAppendChar(dynStr_t *string, char c) {
	assert(string != NULL);
	if (string->size + 1 >= string->alloc_size) {
		// TODO: Add NULL check
		string->string = realloc(string->string, string->alloc_size + DYN_STR_LENGTH);
	}
	string->string[string->size++] = c;
	string->string[string->size] = 0;
}

void dynStrAppendString(dynStr_t *string, const char* str) {
	assert(string != NULL);
	size_t strLen = strlen(str);
	if (string->size + strLen + 1 >= string->alloc_size) {
		string->string = realloc(string->string, string->alloc_size + strLen + DYN_STR_LENGTH);
	}
	strcat(string->string, str);
	string->size += strLen;
	string->string[string->size] = 0;
}

bool dynStrEqualString(dynStr_t* string, const char* str) {
	assert(string != NULL);
	return strcmp(string->string, str) == 0;
}
