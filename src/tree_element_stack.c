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
#include "tree_element_stack.h"

treeStack_t* treeStackInit() {
    treeStack_t* stack = malloc(sizeof(treeStack_t));
    stack->head = NULL;
    return stack;
}

void treeStackFree(treeStack_t* stack) {
    treeStackItem_t* item = stack->head;
    treeStackItem_t* next;
    while (item != NULL) {
        next = item->next;
        free(item);
        item = next;
    }
    free(stack);
}

bool treeStackIsEmpty(treeStack_t* stack) {
    return stack->head == NULL;
}

void treeStackPush(treeStack_t* stack, treeElement_t* value) {
    treeStackItem_t* item = malloc(sizeof(treeStackItem_t));
    item->value = value;
    item->next = stack->head;
    stack->head = item;
}

treeElement_t* treeStackPop(treeStack_t* stack) {
    if (treeStackIsEmpty(stack)) {
        return NULL;
    }
    treeStackItem_t* item = stack->head;
    stack->head = item->next;
    treeElement_t* token = item->value;
    free(item);
    return token;
}

treeElement_t* treeStackTop(treeStack_t* stack) {
    if (treeStackIsEmpty(stack)) {
        return NULL;
    }
    return stack->head->value;
}
