//
// Created by fjerabek on 12.11.19.
//

#ifndef FIT_BIT_IFJ_2019_PROJECT_PARSER_H
#define FIT_BIT_IFJ_2019_PROJECT_PARSER_H

#include "scanner.h"
#include "stdbool.h"


// Derivation tree element data

typedef union tree_el_data tree_el_data_t;

enum State { FUNCTION_DEF, BLOCK, ROOT };
// Derivation tree element
typedef struct{
    tree_el_data_t* data;
} tree_element_t;

union tree_el_data {
    char* s;
    double d;
    long l;
    tree_element_t element;
};

/**
 * Parse line of source code in file and creates derivation tree representation
 * @param file    source file
 * @returns derivation  tree representation of one line
 * @pre file is opened in read mode
 */
tree_element_t syntax_parse(FILE * file);


/**
 * Parses while structure after while keyword
 * @param file source file
 * @param stack lexical analysis stack
 * @return parsing successful
 */
bool parse_while(FILE* file, intStack_t* stack);

/**
 * Returns string representation of token
 * @param token token
 * @return string representation
 */
char* token_to_string(token_t token);

/**
 * Parses code block
 * @param file source file
 * @param stack lexical analysis stack
 * @param last_token pointer for writing first non-block token
 * @return parsing successful
 */
bool parse_block(FILE* file, intStack_t* stack, token_t* last_token);

/**
 * Parses assignment expression
 * @param file source file
 * @param stack lexical analysis stack
 * @return parsing successful
 */
bool parse_assignment(FILE* file, intStack_t* stack);


#endif //FIT_BIT_IFJ_2019_PROJECT_PARSER_H
