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

#include "dynamic_string_list.h"

dynStrList_t *dynStrListInit() {
	dynStrList_t *list = malloc(sizeof(dynStrList_t));
	if (list == NULL) {
		return NULL;
	}
	list->head = NULL;
	list->tail = NULL;
	return list;
}

void dynStrListClear(dynStrList_t *list) {
	dynStrListEl_t *current = list->head;
	while (current != NULL) {
		dynStrListElFree(current);
		current = list->head;
	}
}

void dynStrListFree(dynStrList_t *list) {
	dynStrListClear(list);
	free(list);
}

bool dynStrListIsEmpty(dynStrList_t *list) {
	if (list == NULL) {
		return true;
	}
	return list->head == NULL;
}

unsigned long dynStrListSize(dynStrList_t *list) {
	if (dynStrListIsEmpty(list)) {
		return 0;
	}
	unsigned long size = 0;
	dynStrListEl_t *current = list->head;
	while (current != NULL) {
		size++;
		current = current->next;
	}
	return size;
}

void dynStrListPopFront(dynStrList_t *list) {
	dynStrListRemove(dynStrListFront(list));
}

void dynStrListPopBack(dynStrList_t *list) {
	dynStrListRemove(dynStrListBack(list));
}

bool dynStrListPushFront(dynStrList_t *list, dynStr_t *string) {
	if (list == NULL || string == NULL) {
		return false;
	}
	dynStrListEl_t *element = dynStrListElInit();
	if (element == NULL) {
		return false;
	}
	element->string = string;
	element->next = list->head;
	list->head->prev = element;
	list->head = element;
	return true;
}

bool dynStrListPushBack(dynStrList_t *list, dynStr_t *string) {
	if (list == NULL || string == NULL) {
		return false;
	}
	dynStrListEl_t *element = dynStrListElInit();
	if (element == NULL) {
		return false;
	}
	element->string = string;
	element->prev = list->tail;
	list->tail->next = element;
	list->tail = element;
	return true;
}

dynStrListEl_t *dynStrListFront(dynStrList_t *list) {
	if (list == NULL) {
		return NULL;
	}
	return list->head;
}

dynStrListEl_t *dynStrListElPrev(dynStrListEl_t *element) {
	if (element == NULL) {
		return NULL;
	}
	return element->prev;
}

dynStrListEl_t *dynStrListElNext(dynStrListEl_t *element) {
	if (element == NULL) {
		return NULL;
	}
	return element->next;
}

dynStrListEl_t *dynStrListBack(dynStrList_t *list) {
	return list->tail;
}

void dynStrListRemove(dynStrListEl_t *element) {
	if (element == NULL) {
		return;
	}
	dynStrListEl_t *next = element->next;
	next->prev = element->prev;
	dynStrListElFree(element);
}

dynStrListEl_t *dynStrListElInit() {
	dynStrListEl_t *element = malloc(sizeof(dynStrListEl_t));
	if (element == NULL) {
		return NULL;
	}
	element->next = NULL;
	element->prev = NULL;
	element->string = NULL;
	return element;
}

void dynStrListElFree(dynStrListEl_t *element) {
	if (element == NULL) {
		return;
	}
	dynStrFree(element->string);
	free(element);
}

dynStr_t *dynStrListElGet(dynStrListEl_t *element) {
	return element->string;
}
