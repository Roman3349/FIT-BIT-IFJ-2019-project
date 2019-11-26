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

#pragma once

#include "scanner.h"

enum treeElementType {
    E_CODE,
    E_CODE_BLOCK,
    E_STATEMENTS,
    E_STATEMENT,
    E_S_IF,
    E_S_WHILE,
    E_S_PASS,
    E_S_RETURN,
    E_S_ELSE,
    E_S_FUNCTION_DEF,
    E_S_FUNCTION_CALL,
    E_S_EXPRESSION,
    E_TOKEN,
};

typedef enum treeElementType treeElementType_t;

// Derivation tree element
typedef struct{
    treeElementType_t type;
    void* data;
    unsigned int nodeSize;
} treeElement_t;


/**
 * Initializes new tree/subtree of set type
 * @param tree pointer to tree for initialization
 * @param type element type
 * @return initialized tree
 */
void treeInit(treeElement_t* tree, treeElementType_t type);

/**
 * Adds new sub-node to provided node
 * @param treeNode node to which insert
 * @param treeElementType new element type
 * @return insertion successful
 */
treeElement_t* treeAddElement(treeElement_t* treeNode, treeElementType_t treeElementType);


/**
 * Inserts element into tree node
 * @param treeNode tree node
 * @param element  inserted element
 * @return pointer to inserted element in tree
 */
treeElement_t* treeInsertElement(treeElement_t* treeNode, treeElement_t element);

/**
 * Recursively frees the tree
 * @param tree tree to free
 */
void treeFree(treeElement_t tree);

/**
 * Insert token to tree element
 * @param tree tree element
 * @param token token
 * @return insertion successful
 */
bool treeAddToken(treeElement_t* tree, token_t token);


/**
 * Prints tree in readable form
 * @param tree tree to print
 * @param indentation initial indentation
 */
void printTree(treeElement_t tree, int indentation);
