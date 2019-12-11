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

#include "stack.h"

intStack_t* stackInit() {
	intStack_t* stack = malloc(sizeof(intStack_t));
	stack->head = NULL;
	return stack;
}

void stackFree(intStack_t* stack) {
	intStack_item_t* item = stack->head;
	intStack_item_t* next;
	while (item != NULL) {
		next = item->next;
		free(item);
		item = next;
	}
	free(stack);
}

bool stackIsEmpty(intStack_t* stack) {
	return stack->head == NULL;
}

void stackPush(intStack_t* stack, int value) {
	intStack_item_t* item = malloc(sizeof(intStack_item_t));
	item->value = value;
	item->next = stack->head;
	stack->head = item;
	stack->count++;
}

bool stackPop(intStack_t* stack, int* value) {
	if (stackIsEmpty(stack)) {
		return false;
	}
	intStack_item_t* item = stack->head;
	stack->head = item->next;
	*value = item->value;
	free(item);
	stack->count--;
	return true;
}

bool stackTop(intStack_t* stack, int* value) {
	if (stackIsEmpty(stack)) {
		return false;
	}
	*value = stack->head->value;
	return true;
}

bool stackCount(intStack_t* stack, int* value){
    if (!stack)
        return false;
    *value = stack->count;
    return true;
}

bool stackGetIndent(intStack_t* stack, int* value, int number) {
    if (!stack)
        return false;
    if(stack->count - number < 0 || stack->count - number >= stack->count)
        return false;
    intStack_item_t* item = stack->head;
    for(int i = stack->count - number; i > 0; i--) {
        item = item->next;
    }
    *value = item->value;
    return true;
}
