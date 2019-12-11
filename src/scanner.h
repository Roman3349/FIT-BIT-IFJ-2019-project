/*
 * Copyright (C) 2019 Pavel Raur <xraurp00@stud.fit.vutbr.cz>
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "dynamic_string.h"
#include "stack.h"
#include "error.h"

enum execution_status {
	SUCCESS = 0,
	EXECUTION_ERROR = -1,
	ANALYSIS_FAILED = -2
};

enum token_type {
    T_EOL,
    T_EOF,
    T_OP_ADD,       // +
    T_OP_SUB,       // -
    T_OP_MUL,       // *
    T_OP_DIV,       // /
    T_OP_IDIV,      // // (integer division)
    T_OP_EQ,        // ==
    T_OP_GREATER,   // >
    T_OP_LESS,      // <
    T_OP_GREATER_EQ,// >=
    T_OP_LESS_EQ,   // <=
    T_OP_NOT_EQ,    // !=
    T_NUMBER,
    T_FLOAT,
    T_STRING,
    T_STRING_ML,    // multiline string, can be also multiline comment
    T_ID,           // id of var/function (var name)
    T_KW_DEF,
    T_KW_IF,
    T_KW_ELSE,
    T_KW_WHILE,
    T_KW_PASS,
    T_KW_RETURN,
    T_KW_NONE,
    T_BOOL_AND,     // and
    T_BOOL_OR,      // or
    T_BOOL_NEG,     // not
    T_BOOL_TRUE,
    T_BOOL_FALSE,
    T_ASSIGN,       // =
    T_COLON,        // :
    T_COMMA,        // ,
    T_LPAR,         // (
    T_RPAR,         // )
    T_INDENT,
    T_DEDENT,
    T_UNKNOWN,      // lexical analysis failed - bad syntax
    T_ERROR         // scanner can't continue in execution
};

// token data
typedef union tokenValue {
    dynStr_t* strval;
    long intval;
    double floatval;
} tokenValue_t;

// output token
typedef struct token {
    enum token_type type;
    tokenValue_t data;
} token_t;

/**
 * Scans source code in file and creates token representation
 * @param file    source file
 * @param stack   stack for offset checking
 * @returns token that represents current keyword
 * @pre file is opened in read mode
 */
token_t scan(FILE* file, intStack_t* stack);

/**
 * Checks if digit is octal
 * @param num  digit to check
 * @returns true if number is octal, false if not
 */
bool is_oct(int num);

/**
 * Checks if digit is binary
 * @param num  digit to check
 * @returns true if number is binary, false otherwise
 */
bool is_bin(int num);

/**
 * Scans number to a token
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @param first_number  first digit of the number
 * @returns execution status
 * @pre token must be empty - token.data.strval must point to NULL
 */
int process_number(FILE* file, token_t* token ,int first_number);

/**
 * Scans keyword to a token
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @param first_number  first char of the keyword
 * @returns status: SUCCESS on success, EXECUTION_ERROR if there was internal error
 * @pre token must be empty - token.data.strval must point to NULL
 */
int process_keyword(FILE* file, token_t* token, int first_char);

/**
 * @param keyword scanned to string
 * @returns token type
 */
enum token_type getKeywordType(char* string);

/**
 * Scans string or multiline comment to a token
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @param qmark         first quotation mark, to determine string end
 * @returns status: SUCCESS on success, EXECUTION_ERROR on internal error
 *      and ANALYSIS_FAILED if string is not complete
 * @pre token must be empty - token.data.strval must point to NULL
 */
int process_string(FILE* file, token_t* token, int qmark);

/**
 * Scans line comment to a token (everything to the end of the line)
 * @param file          source file
 * @returns status: SUCCESS or EXECUTION_ERROR
 */
int remove_line_comment(FILE* file);

/**
 * Process the escape sequences
 * @param file Source file
 * @param token Token
 * @param c Character to process
 * @return Execution status
 */
int process_escape_seq(FILE* file, token_t *token, int c);

/**
 * Returns string representation of token
 * @param token token
 * @return string representation
 */
char* tokenToString(enum token_type type);


/**
 * Converts token from boolean to integer
 * @param token token to convert
 * @param errCode error code
 */
void tokenBoolToInt(token_t * token, int* errCode);


/**
 * Converts token from Integer to Float
 * @param token token to convert
 * @param errCode error code
 */
void tokenIntToFloat(token_t * token, int* errCode);
