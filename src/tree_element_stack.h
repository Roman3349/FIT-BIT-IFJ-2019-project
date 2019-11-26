/*
 * Copyright (C) 2019 František Jeřábek <xjerab25@stud.fit.vutbr.cz>, Roman Ondráček <xondra58@stud.fit.vutbr.cz>
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
#include "scanner.h"
#include "parse_tree.h"

typedef struct treeStackItem treeStackItem_t;

struct treeStackItem {
    treeStackItem_t* next;
    treeElement_t* value;
};

typedef struct token_stack {
    treeStackItem_t* head;
} treeStack_t;

/**
 * Initializes a stack
 * @return Stack
 */
treeStack_t* tokenStackInit();

/**
 * Frees a stack
 * @param stack Stack to free
 */
void tokenStackFree(treeStack_t* stack);

/**
 * Check if a stack is empty
 * @param stack Stack to check
 * @return Is stack empty?
 */
bool tokenStackIsEmpty(treeStack_t* stack);

/**
 * Push pointer into stack
 * @param stack Stack
 * @param value Value which will be pushed
 */
void tokenStackPush(treeStack_t* stack, treeElement_t* value);

/**
 * Pops item from stack. If stack is empty gets next token from lexical analysis
 * @param stack Stack
 * @return popped value
 */
treeElement_t* tokenStackPop(treeStack_t* stack);

/**
 * Returns top item from stack. If stack is empty load next token from lexical analysis
 * @param stack Stack
 * @return top value
 */
treeElement_t* tokenStackTop(treeStack_t* stack);
