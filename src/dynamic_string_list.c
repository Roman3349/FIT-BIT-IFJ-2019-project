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
	if (list == NULL) {
		return;
	}
	dynStrListEl_t *current = list->head;
	while (current != NULL) {
		dynStrListEl_t *next = current->next;
		dynStrListElFree(current);
		current = next;
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
	dynStrListRemove(list, dynStrListFront(list));
}

void dynStrListPopBack(dynStrList_t *list) {
	dynStrListRemove(list, dynStrListBack(list));
}

bool dynStrListPushFront(dynStrList_t *list, dynStr_t *string) {
	if (list == NULL || string == NULL) {
		return false;
	}
	dynStrListEl_t *element = dynStrListElInit(string);
	if (element == NULL) {
		return false;
	}
	if (list->head == NULL) {
		list->head = list->tail = element;
	} else {
		element->next = list->head;
		list->head->prev = element;
		list->head = element;
	}
	return true;
}

bool dynStrListPushBack(dynStrList_t *list, dynStr_t *string) {
	if (list == NULL || string == NULL) {
		return false;
	}
	dynStrListEl_t *element = dynStrListElInit(string);
	if (element == NULL) {
		return false;
	}
	if (list->head == NULL) {
		list->head = list->tail = element;
	} else {
		element->prev = list->tail;
		list->tail->next = element;
		list->tail = element;
	}
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
	if (list == NULL) {
		return NULL;
	}
	return list->tail;
}

void dynStrListRemove(dynStrList_t *list, dynStrListEl_t *element) {
	if (list == NULL || element == NULL) {
		return;
	}
	dynStrListEl_t *prev = element->prev;
	dynStrListEl_t *next = element->next;
	if (prev != NULL) {
		prev->next = next;
	} else {
		list->head = next;
	}
	if (next != NULL) {
		next->prev = prev;
	} else {
		list->tail = prev;
	}
	dynStrListElFree(element);
}

dynStrListEl_t *dynStrListElInit(dynStr_t *string) {
	if (string == NULL) {
		return NULL;
	}
	dynStrListEl_t *element = malloc(sizeof(dynStrListEl_t));
	if (element == NULL) {
		return NULL;
	}
	element->next = NULL;
	element->prev = NULL;
	element->string = string;
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
