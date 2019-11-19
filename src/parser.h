//
// Created by fjerabek on 12.11.19.
//

#ifndef FIT_BIT_IFJ_2019_PROJECT_PARSER_H
#define FIT_BIT_IFJ_2019_PROJECT_PARSER_H

#include "scanner.h"
#include "stdbool.h"


// Derivation tree element data

typedef union tree_el_data tree_el_data_t;

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

bool function_param_g(FILE* file, intStack_t* stack);

bool function_nparam_g(FILE* file, intStack_t* stack);

bool function_params_g(FILE* file, intStack_t* stack);

bool function_call_g(FILE* file, intStack_t* stack);

bool eols_g(FILE* file, intStack_t* stack);

bool expression_g(FILE* file, intStack_t* stack);

#endif //FIT_BIT_IFJ_2019_PROJECT_PARSER_H
