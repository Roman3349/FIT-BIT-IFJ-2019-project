/*
 * Copyright (C) 2019 František Jeřábek <xjerab25@stud.fit.vutbr.cz>
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

#include "parse_tree.h"

void treeInit(treeElement_t* tree, treeElementType_t elementType) {
    tree->type = elementType;
    tree->data = NULL;
    tree->nodeSize = 0;
}

treeElement_t* treeAddElement(treeElement_t* treeNode, treeElementType_t type) {
    if(treeNode == NULL)
        return NULL;

    if(treeNode->type == E_TOKEN) {
        return NULL;
    }

    if(treeNode->data == NULL){
        treeNode->data = malloc(sizeof(treeElement_t));
        if(treeNode->data == NULL)
            return NULL;
    } else {
        treeNode->data = realloc(treeNode->data, sizeof(treeElement_t[(treeNode->nodeSize + 1)]));
    }

    treeInit(&((treeElement_t*)treeNode->data)[treeNode->nodeSize], type);

    return &((treeElement_t*)treeNode->data)[treeNode->nodeSize++];
}

bool treeAddToken(treeElement_t* tree, token_t token) {
    if(tree == NULL)
        return false;

    treeElement_t * element = treeAddElement(tree, E_TOKEN);
    if(element == NULL)
        return false;

    element->data = malloc(sizeof(token));
    memcpy(element->data, &token, sizeof(token));

    return tree;
}

void treeFree(treeElement_t tree) {
    if (tree.type == E_TOKEN) {
        switch(((token_t*)tree.data)->type){
            case T_STRING:
            case T_STRING_ML:
            case T_ID:
                dynStrFree(((token_t*)tree.data)->data.strval);
                break;
            default:
                break;
        }
        free(tree.data);
        return;
    }

    if (tree.data != NULL) {
        for(size_t i = 0; i < tree.nodeSize; i++) {
            treeFree(((treeElement_t* )(tree.data))[i]);
        }
    }
    free(tree.data);
}

void printTree(treeElement_t tree, int indent) {
    for(int i = indent; i > 0; i--){
        printf("\t");
    }
    switch(tree.type) {
        case E_TOKEN:
            printf("TOKEN: %s\n", tokenToString(((token_t*)tree.data)->type));
            break;
        case E_CODE:
            printf("CODE ");
            break;
        case E_CODE_BLOCK:
            printf("CODE_BLOCK ");
            break;
        case E_STATEMENTS:
            printf("STATEMENTS ");
            break;
        case E_STATEMENT:
            printf("STATEMENT ");
            break;
        case E_S_IF:
            printf("IF ");
            break;
        case E_S_WHILE:
            printf("WHILE ");
            break;
        case E_S_PASS:
            printf("PASS ");
            break;
        case E_S_RETURN:
            printf("RETURN ");
            break;
        case E_S_ELSE:
            printf("ELSE ");
            break;
        case E_S_FUNCTION_DEF:
            printf("FUNCTION DEF");
            break;
        case E_S_FUNCTION_CALL:
            printf("FUNCTION CALL");
            break;
        case E_S_EXPRESSION:
            printf("EXPRESSION");
            break;
    }
    if(tree.type != E_TOKEN) {
        printf("{\n");
        for (size_t i = 0; i < tree.nodeSize; i++) {
            printTree(((treeElement_t *) tree.data)[i], indent + 1);
        }
        for(int j = indent; j > 0; j--){
            printf("\t");
        }
        printf("}\n");
    }
}

