/*
 * Copyright (C) 2019 Pavel Raur <xraurp00@stud.fit.vutbr.cz>
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
#ifndef FIT_BIT_IFJ_2019_PROJECT_SCANNER_H
#define FIT_BIT_IFJ_2019_PROJECT_SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dynamic_string.h"
#include "stack.h"

enum token_type {
    T_WHITESPACE,
    T_EOL,
    T_EOF,
    T_OPERATOR,
    T_NUMBER,
    T_STRING,
    T_STRING_ML,  // multiline string, can be also multiline comment
    T_ID,         // id of var/function (var name)
    T_ID_DEF,
    T_ID_IF,
    T_ID_ELSE,
    T_ID_WHILE,
    T_ID_PASS,
    T_ID_RETURN,
    T_COLON,      // :
    T_LPAR,       // (
    T_RPAR,       // )
    T_BRACKET,    // [
    T_RBRACKET,   // ]
    T_LBRACE,     // {
    T_RBRACE,     // }
    T_COMMENT,    // #
    T_INDENT,
    T_DEDENT,
    T_UNKNOWN,
    T_NONE
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

/*
 * Scans source code in file and creates token representation
 * @param file    source file
 * @param stack   stack for offset checking
 * @returns token that represents current keyword
 * @pre file is opened in read mode
 */
token_t scan(FILE* file, intStack_t* stack);

/*
 * Counts tabs or spaces in offset at the beginning of the line
 * @param file    source file
 * @returns number of whitespaces in padding or -1 if eof
 */
int getCodeOffset(FILE* file);

/*
 * Checks if digit is decimal
 * @param num  digit to check
 * @returns true if number is decimal, false if not
 */
bool is_dec(int num);

/*
 * Checks if digit is octal
 * @param num  digit to check
 * @returns true if number is octal, false if not
 */
bool is_oct(int num);

/*
 * Checks if digit is hexadecimal
 * @param num  digit to check
 * @returns true if number is hexadecimal, false if not
 */
bool is_hex(int num);

/*
 * Checks if digit is binary
 * @param num  digit to check
 * @returns TRUE if number is binary, FALSE otherwise
 */
bool is_bin(int num);

/*
 * Scans number to a token
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @param first_number  first digit of the number
 * @returns status: 0 = success
 *                 -1 = file error
 *                 -2 = token error / memory allocation
 * @pre token must be empty - initialized to type T_NONE and value NULL
 */
int process_number(FILE* file, token_t* token ,char first_number);

/*
 * Scans keyword to a token
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @param first_number  first char of the keyword
 * @returns status: 0 = success
 *                 -1 = file error
 *                 -2 = token error / memory allocation
 * @pre token must be empty - initialized to type T_NONE and value NULL
 */
int process_keyword(FILE* file, token_t* token, char first_char);

/*
 * @param keyword scanned to string
 * @returns token type
 */
enum token_type getKeywordType(char* string);

/*
 * Checks if given character is lowercase of uppercase letter
 * @param c  string to check
 * @returns TRUE if c is letter in given range, FALSE otherwise
 */
int is_letter(int c);

/*
 * Scans string or multiline comment to a token
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @param qmark         first quotation mark, to determine string end
 * @returns status: 0 = success
 *                 -1 = file error
 *                 -2 = token error / memory allocation
 * @pre token must be empty - initialized to type T_NONE and value NULL
 */
int process_string(FILE* file, token_t* token, char qmark);

/*
 * Scans line comment to a token (everything to the end of the line)
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @returns status: 0 = success
 *                 -1 = file error
 *                 -2 = token error / memory allocation
 * @pre token must be empty - initialized to type T_NONE and value NULL
 */
int process_comment(FILE* file, token_t* token);

#endif //FIT_BIT_IFJ_2019_PROJECT_SCANNER_H
