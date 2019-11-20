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
#include "stdbool.h"
#include "token_stack.h"


// Derivation tree element data
typedef union treeElData treeElData_t;

// Derivation tree element
typedef struct{
    treeElData_t* data;
} treeElement_t;

union treeElData {
    char* s;
    double d;
    long l;
    treeElement_t element;
};

/**
 * Parse line of source code in file and creates derivation tree representation
 * @param file    source file
 * @returns derivation  tree representation of one line
 * @pre file is opened in read mode
 */
treeElement_t syntaxParse(FILE * file);


/**
 * Parses while structure after while keyword
 * @param file source file
 * @param stack token stack
 * @return parsing successful
 */
bool parseWhile(tokenStack_t* stack);

/**
 * Returns string representation of token
 * @param token token
 * @return string representation
 */
char* tokenToString(enum token_type type);

/**
 * Parses code block
 * @param file source file
 * @param stack token stack
 * @return parsing successful
 */
bool parseBlock(tokenStack_t* stack);

/**
 * Parses assignment expression
 * @param file source file
 * @param stack token stack
 * @return parsing successful
 */
bool parseAssignment(tokenStack_t* stack);


/**
 * Parses function call
 * @param file source file
 * @param stack token stack
 * @return parsing successful
 */
bool parseFunctionCall(tokenStack_t* stack);

/**
 * Parses pass keyword
 * @param file source file
 * @param stack token stack
 * @return parsing successful
 */
bool parsePass(tokenStack_t* stack);

/**
 * Parses return keyword and value
 * @param stack token stack
 * @return parsing successful
 */
bool parseReturn(tokenStack_t* stack);

/**
 * Parses if keyword
 * @param stack token stack
 * @return parsing successful
 */
bool parseIf(tokenStack_t* stack);

/**
 * Parses else keyword
 * @param stack token stack
 * @return parsing successful
 */
bool parseElse(tokenStack_t* stack);


/**
 * Parses function definition
 * @param stack token stack
 * @return parsing successful
 */
bool parseFunctionDef(tokenStack_t* stack);

/**
 * Check if next token matches expected token eventually prints error message
 * @param stack token stack
 * @param expectedToken expected token
 * @return got expected token
 */
bool processToken(tokenStack_t* stack, enum token_type expectedToken);