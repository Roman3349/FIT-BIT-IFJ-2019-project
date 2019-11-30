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

#include "token_stack.h"

tokenStack_t* tokenStackInit(FILE* file, intStack_t* lexStack) {
    tokenStack_t* stack = malloc(sizeof(tokenStack_t));
    stack->head = NULL;
    stack->file = file;
    stack->lexStack = lexStack;
    return stack;
}

void tokenStackFree(tokenStack_t* stack) {
    tokenStackItem_t* item = stack->head;
    tokenStackItem_t* next;
    while (item != NULL) {
        next = item->next;
        free(item);
        item = next;
    }
    free(stack);
}

bool tokenStackIsEmpty(tokenStack_t* stack) {
    return stack->head == NULL;
}

void tokenStackPush(tokenStack_t* stack, token_t value) {
    tokenStackItem_t* item = malloc(sizeof(tokenStackItem_t));
    item->value = value;
    item->next = stack->head;
    stack->head = item;
}

token_t tokenStackPop(tokenStack_t* stack, int* errCode) {
    if (tokenStackIsEmpty(stack)) {
		token_t token = scan(stack->file, stack->lexStack);
		if(token.type == T_UNKNOWN) {
			*errCode = LEXICAL_ERR_CODE;
		} else if(token.type == T_ERROR) {
			*errCode = INTERNAL_ERR_CODE;
		}
		tokenStackPush(stack, token);
    }
    tokenStackItem_t* item = stack->head;
    stack->head = item->next;
    token_t token = item->value;
    free(item);
    return token;
}

token_t tokenStackTop(tokenStack_t* stack, int* errCode) {
    if (tokenStackIsEmpty(stack)) {
    	token_t token = scan(stack->file, stack->lexStack);
    	if(token.type == T_UNKNOWN) {
    		*errCode = LEXICAL_ERR_CODE;
    	} else if(token.type == T_ERROR) {
    		*errCode = INTERNAL_ERR_CODE;
    	}
		tokenStackPush(stack, token);
    }
    return stack->head->value;
}
