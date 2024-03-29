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

#include <stdbool.h>
#include <stdio.h>
#include "error.h"
#include "scanner.h"
#include "token_stack.h"
#include "parse_tree.h"
#include "tree_element_stack.h"
#include "symtable.h"

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


/**
 * Parse line of source code in file and creates derivation tree representation
 * @param file source file
 * @param symTable symbol table
 * @param errCode error code
 * @returns derivation  tree representation of one line
 * @pre file is opened in read mode
 */
treeElement_t syntaxParse(FILE * file, symTable_t* symTable, int* errCode);


/**
 * Parses while structure after while keyword
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @return parsing error code
 */
int parseWhile(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* context);

/**
 * Parses code block
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @return parsing error code
 */
int parseBlock(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* context);

/**
 * Parses function call
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @return parsing error code
 */
int parseFunctionCall(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* context);

/**
 * Parses pass keyword
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @return parsing error code
 */
int parsePass(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* context);

/**
 * Parses return keyword and value
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @return parsing error code
 */
int parseReturn(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* context);

/**
 * Parses if keyword
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @return parsing error code
 */
int parseIf(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* context);

/**
 * Parses else keyword
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @return parsing error code
 */
int parseElse(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* context);


/**
 * Parses function definition
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @return parsing error code
 */
int parseFunctionDef(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable);

/**
 * Parses function definition parameters
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param functionName defining function name
 * @return parsing error code
 */
int parseFunctionDefParams(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* functionName);

/**
 * Parses function call parameters
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @param functionName name of calling function
 * @return parsing error code
 */
int parseFunctionCallParams(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* context, dynStr_t* functionName);

/**
 * Check if next token matches expected token eventually prints error message
 * @param stack token stack
 * @param expectedToken expected token
 * @param tree parse tree
 * @return processing error code
 */
int processToken(tokenStack_t* stack, enum token_type expectedToken, treeElement_t* tree);

/**
 * Converts statementPart to token if possible
 * @param statementPart statement part
 * @param type pointer to token type function
 * @return conversion error code
 */
int statementPartToTokenType(statementPart_t statementPart, enum token_type* type);

/**
 * Parses expression
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @return parsing error code
 */
int parseExpression(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* context);

/**
 * Processes statement part
 * @param stack token stack
 * @param part statement part
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @return processing error code
 */
int processStatementPart(tokenStack_t* stack, statementPart_t part, treeElement_t* tree, symTable_t* symTable, dynStr_t* context);

/**
 * Parses assignment
 * @param stack token stack
 * @param tree parse tree
 * @param symTable symbol table
 * @param context parser context
 * @return processing error code
 */
int parseAssignment(tokenStack_t* stack, treeElement_t* tree, symTable_t* symTable, dynStr_t* context);
