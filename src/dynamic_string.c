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
		return NULL;
	}
	string->string = malloc(DYN_STR_LENGTH);
	if (string->string == NULL) {
		free(string);
		return NULL;
	}
	string->alloc_size = DYN_STR_LENGTH;
	dynStrClear(string);
	return string;
}

void dynStrClear(dynStr_t *string) {
	if (string == NULL) {
		return;
	}
	string->string[0] = 0;
	string->size = 0;
	char *tmp = realloc(string->string, DYN_STR_LENGTH);
	if (tmp == NULL) {
		return;
	}
	string->string = tmp;
}

void dynStrFree(dynStr_t *string) {
	if (string != NULL) {
		free(string->string);
		free(string);
	}
}

bool dynStrAppendChar(dynStr_t *string, char c) {
	if (string == NULL) {
		return false;
	}
	if (string->size + 1 >= string->alloc_size) {
		unsigned long newSize = string->alloc_size + DYN_STR_LENGTH;
		char *tmp = realloc(string->string, newSize);
		if (tmp == NULL) {
			return false;
		}
		string->string = tmp;
		string->alloc_size = newSize;
	}
	string->string[string->size++] = c;
	string->string[string->size] = 0;
	return true;
}

bool dynStrAppendString(dynStr_t *string, const char* str) {
	if (string == NULL || str == NULL) {
		return false;
	}
	size_t strLen = strlen(str);
	if (string->size + strLen + 1 >= string->alloc_size) {
		unsigned long newSize = string->alloc_size + strLen + DYN_STR_LENGTH;
		char* tmp = realloc(string->string, newSize);
		if (tmp == NULL) {
			return false;
		}
		string->string = tmp;
		string->alloc_size = newSize;
	}
	strcat(string->string, str);
	string->size += strLen;
	string->string[string->size] = 0;
	return true;
}

bool dynStrEqual(dynStr_t* string1, dynStr_t *string2) {
	if (string1 == NULL && string2 == NULL) {
		return true;
	}
	if (string1 == NULL || string2 == NULL) {
		return false;
	}
	return strcmp(string1->string, string2->string) == 0;
}

bool dynStrEqualString(dynStr_t* string, const char* str) {
	if (string == NULL || str == NULL) {
		return false;
	}
	return strcmp(string->string, str) == 0;
}

bool dynStrIsEmpty(dynStr_t *string) {
	assert(string != NULL);
	return string->size == 0;
}

bool dynStrCopy(dynStr_t *dst, dynStr_t *src) {
	if (dst == NULL || src == NULL) {
		return false;
	}
	char* tmp = realloc(dst->string, src->alloc_size);
	if (tmp == NULL) {
		return false;
	}
	dst->string = tmp;
	dst->alloc_size = src->alloc_size;
	dst->size = src->size;
	strcpy(dst->string, src->string);
	return true;
}

dynStr_t *dynStrClone(dynStr_t *src) {
	dynStr_t *string = dynStrInit();
	if (string == NULL) {
		return NULL;
	}
	if (!dynStrCopy(string, src)) {
		dynStrFree(string);
		return NULL;
	}
	return string;
}

bool dynStrEscape(dynStr_t *string) {
	if (string == NULL) {
		return false;
	}
	dynStr_t *tmp = dynStrInit();
	if (tmp == NULL) {
		return false;
	}
	for (size_t i = 0; i < strlen(string->string); i++) {
		char c = string->string[i];
		if (!isprint(c) || (c == ' ') || (c == '#') || (c == '\\')) {
			char esc[5];
			sprintf(esc, "\\%03d", c);
			if (!dynStrAppendString(tmp, esc)) {
				return false;
			}
		} else {
			if (!dynStrAppendChar(tmp, c)) {
				return false;
			}
		}
	}
	if (!dynStrCopy(string, tmp)) {
		return false;
	}
	dynStrFree(tmp);
	return true;
}

char dynStrGetChar(dynStr_t *string, unsigned long index) {
	if (string == NULL || index > string->size) {
		return 0;
	}
	return string->string[index];
}

const char *dynStrGetString(dynStr_t* string) {
	return string->string;
}
