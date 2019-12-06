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

#include <stdbool.h>

#include "dynamic_string.h"

typedef struct dynamic_string_list_element dynStrListEl_t;

struct dynamic_string_list_element {
	dynStrListEl_t *next;
	dynStrListEl_t *prev;
	dynStr_t *string;
};


typedef struct dynamic_string_list {
	dynStrListEl_t *head;
	dynStrListEl_t *tail;
} dynStrList_t;

/**
 * Initializes a new dynamic string list
 * @return Initialized dynamic string list
 */
dynStrList_t *dynStrListInit();

/**
 * Clears the dynamic string list
 * @param list Dynamic string list to clear
 */
void dynStrListClear(dynStrList_t *list);

/**
 * Frees the dynamic string list
 * @param list Dynamic string list to free
 */
void dynStrListFree(dynStrList_t *list);

/**
 * Checks is the dynamic string lis is empty
 * @param list Dynamic string list
 * @return Dynamic string list emptyness
 */
bool dynStrListIsEmpty(dynStrList_t *list);

/**
 * Returns the size of the dynamic string list
 * @param list Dynamic string list
 * @return Size of dynamic string list
 */
unsigned long dynStrListSize(dynStrList_t *list);

/**
 * Deletes the first element
 * @param list Dynamic string list
 */
void dynStrListPopFront(dynStrList_t *list);

/**
 *Deletes the last element
 * @param list Dynamic string list
 */
void dynStrListPopBack(dynStrList_t *list);

/**
 * Adds a new element at the beginning
 * @param list Dynamic string list
 * @param string Dynamic string to add
 * @return Execution status
 */
bool dynStrListPushFront(dynStrList_t *list, dynStr_t *string);

/**
 * Adds a new element at the end
 * @param list Dynamic string list
 * @param string Dynamic string to add
 * @return Execution status
 */
bool dynStrListPushBack(dynStrList_t *list, dynStr_t *string);

/**
 * Returns the first element
 * @param list Dynamic string list
 * @return First element
 */
dynStrListEl_t *dynStrListFront(dynStrList_t *list);

/**
 * Returns the previous element
 * @param element Element
 * @return Previous element
 */
dynStrListEl_t *dynStrListElPrev(dynStrListEl_t *element);

/**
 * Returns the next element
 * @param element Element
 * @return Next element
 */
dynStrListEl_t *dynStrListElNext(dynStrListEl_t *element);

/**
 * Returns the last element
 * @param list Dynamic string list
 * @return Last element
 */
dynStrListEl_t *dynStrListBack(dynStrList_t *list);

/**
 * Removes the element from the dynamic string list
 * @param list Dynamic string list
 * @param element Dynamic string list to remove
 */
void dynStrListRemove(dynStrList_t *list, dynStrListEl_t *element);

/**
 * Initializes a new dynamic string list element
 * @param string Dynamic string
 * @return Initialized dynamic string element
 */
dynStrListEl_t *dynStrListElInit(dynStr_t *string);

/**
 * Frees a dynamic string list element
 * @param element Dynamic string element to free
 */
void dynStrListElFree(dynStrListEl_t *element);

/**
 * Returns the dynamic string from dynamic string list element
 * @param element Dynamic string list element
 * @return Dynamic string
 */
dynStr_t *dynStrListElGet(dynStrListEl_t *element);
