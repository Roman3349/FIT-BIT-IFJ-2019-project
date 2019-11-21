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

enum statementPart{
    S_EOL = T_EOL,
    S_INDENT = T_INDENT,
    S_DEDENT = T_DEDENT,
    S_COLON = T_COLON,
    S_LPAR = T_LPAR,
    S_RPAR = T_RPAR,
    S_ID = T_ID,
    S_KW_IF = T_KW_IF,
    S_KW_PASS = T_KW_PASS,
    S_KW_RETURN = T_KW_RETURN,
    S_KW_ELSE = T_KW_ELSE,
    S_KW_DEF = T_KW_DEF,
    S_KW_WHILE = T_KW_WHILE,
    S_BLOCK = 0xFFFF,
    S_EXPRESSION,
    S_DEF_PARAMS,
    S_CALL_PARAMS
    };

typedef enum statementPart statementPart_t;

//Statement definitions
statementPart_t while_s[] = {S_KW_WHILE, S_EXPRESSION, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT };
statementPart_t if_s[] = {S_KW_IF, S_EXPRESSION, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT};
statementPart_t pass_s[] = {S_KW_PASS, S_EOL};
statementPart_t return_s[] = {S_KW_RETURN, S_EXPRESSION, S_EOL};
statementPart_t else_s[] = {S_KW_ELSE, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT};
statementPart_t functionDef_s[] = {S_KW_DEF, S_ID, S_LPAR, S_DEF_PARAMS, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT};
statementPart_t functionCall_s[] = {S_ID, S_LPAR, S_CALL_PARAMS};


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
 * Parses function definition parameters
 * @param stack token stack
 * @return parsing successful
 */
bool parseFunctionDefParams(tokenStack_t* stack);

/**
 * Parses function call parameters
 * @param stack token stack
 * @return parsing successful
 */
bool parseFunctionCallParams(tokenStack_t* stack);

/**
 * Check if next token matches expected token eventually prints error message
 * @param stack token stack
 * @param expectedToken expected token
 * @return got expected token
 */
bool processToken(tokenStack_t* stack, enum token_type expectedToken);

/**
 * Converts statementPart to token if possible
 * @param statementPart statement part
 * @param type pointer to token type function
 * @return conversion successful
 */
bool statementPartToTokenType(statementPart_t statementPart, enum token_type* type);

/**
 * Parses expression
 * @param stack token stack
 * @return parsing successful
 */
bool parseExpression(tokenStack_t* stack);

/**
 * Processes statement part
 * @param stack token stack
 * @param part statement part
 * @return processing successful
 */
bool processStatementPart(tokenStack_t* stack, statementPart_t part);
