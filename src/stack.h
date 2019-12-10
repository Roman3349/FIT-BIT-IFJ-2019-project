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
#include <stdlib.h>

typedef struct stack_item intStack_item_t;

struct stack_item {
	intStack_item_t* next;
	int value;
};

typedef struct intStack {
	intStack_item_t* head;
	int count;
} intStack_t;

/**
 * Initializes a stack
 * @return Stack
 */
intStack_t* stackInit();

/**
 * Frees a stack
 * @param stack Stack to free
 */
void stackFree(intStack_t* stack);

/**
 * Check if a stack is empty
 * @param stack Stack to check
 * @return Is stack empty?
 */
bool stackIsEmpty(intStack_t* stack);

/**
 * Push pointer into stack
 * @param stack Stack
 * @param value Value which will be pushed
 */
void stackPush(intStack_t* stack, int value);

/**
 * Pops item from stack
 * @param stack Stack
 * @param value Popped value
 * @return No error?
 */
bool stackPop(intStack_t* stack, int* value);

/**
 * Returns top item from stack
 * @param stack Stack
 * @param value Top item
 * @return No error?
 */
bool stackTop(intStack_t* stack, int* value);

/**
 * Return number of indents on stack
 * @param stack Stack
 * @param value Returned value
 * @return No error?
 */
bool stackCount(intStack_t* stack, int* value);

/**
 * Return given indent value
 * @param stack Stack
 * @param value Returned value
 * @param number Number of indent which value is returned
 * @return No error?
 */
bool stackGetIndent(intStack_t* stack, int* value, int number);
