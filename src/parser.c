/*
 * Copyright (C) 2019 František Jeřábek <fanajerabek@seznam.cz>
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

#include "parser.h"

//#include "scanner.h"
token_t* current_token = NULL;

token_t next_token(FILE* file, intStack_t* stack) {
    *current_token = scan(file, stack);
    return *current_token;
}

token_t peek_token(FILE* file, intStack_t* stack){
    if (current_token == NULL) {
        return next_token(file, stack);
    }
    return *current_token;
}


tree_element_t syntax_parse (FILE* file) {
//    intStack_t* stack = stackInit();
    tree_el_data_t data = { .d = 1234};
    tree_element_t tree = { .data = &data};


    printf("current_token %d \n", current_token->type);
    return tree;
}

bool function_param_g(FILE* file, intStack_t* stack) {
    return expression_g(file, stack);
}

bool function_nparam_g(FILE* file, intStack_t* stack) {

    token_t token = peek_token(file, stack);
    if(token.type != T_STRING || token.data.intval != ',') return false;//FIXME Check for COMMA token
    token = next_token(file, stack);
    if(function_param_g(file, stack)) return true;
    return true;
}

bool function_params_g(FILE* file, intStack_t* stack) {
    if(!function_param_g(file, stack)) return false;
    function_nparam_g(file, stack);
    return true;

}

bool function_call_g(FILE* file, intStack_t* stack) {
    if(peek_token(file, stack).type != T_ID) return false;
    if(next_token(file, stack).type != T_LPAR) return false;
    //TODO: function_params_g
    return true;
}

bool eols_g (FILE* file, intStack_t* stack) {
    while(peek_token(file, stack).type == T_EOL) {
        next_token(file, stack);
    }
    return true;
}

bool expression_g (FILE* file, intStack_t* stack) {
    token_t token = peek_token(file, stack);
    if(token.type == T_NONE || token.type == T_ID || token.type == T_NUMBER || token.type == T_STRING) { //TODO: Grammar for math expressions
        next_token(file, stack);
        return true;
    } else {
        return false;
    }
}
