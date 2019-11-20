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

tree_element_t syntax_parse(FILE* file) {

    intStack_t* stack = stackInit();

//    token_t token;
    parse_while(file, stack);

//    printf("Last token: %s\n", token_to_string(token));

    tree_el_data_t data = { .d=1000 };
    tree_element_t tree = { .data = &data };
    return tree;
}


bool parse_expression(FILE* file, intStack_t* stack){
    printf("%s\n", token_to_string(scan(file, stack)));
    printf("%s\n", token_to_string(scan(file, stack)));
    printf("%s\n", token_to_string(scan(file, stack)));
    return true; //FIXME: Expression grammar parser (temporary scrap for 3 tokens)
}

bool parse_while(FILE* file, intStack_t* stack) {
    if(!parse_expression(file, stack)) {
        return false;
    }

    token_t token = scan(file, stack);
    if(token.type != T_COLON) {
        fprintf(stderr, "Syntax error. Expected colon, got %s \n", token_to_string(token));
        return false;
    }

    token = scan(file, stack);
    if(token.type != T_EOL) {
        fprintf(stderr, "Syntax error. Expected EOL, got %s \n", token_to_string(token));
        return false;
    }

    token = scan(file, stack);
    if(token.type != T_INDENT) {
        fprintf(stderr, "Syntax error. Expected indentation, got %s \n", token_to_string(token));
        return false;
    }

    if(!parse_block(file, stack, &token)) { //TODO: block parser
        return false;
    }

    if(token.type != T_DEDENT) {
        fprintf(stderr, "Syntax error. Invalid indentation near %s token.\n", token_to_string(token));
        return false;
    }

    return true;
}

bool parse_assignment(FILE* file, intStack_t* stack) {
    if(!parse_expression(file, stack)){
        return false;
    }

    token_t token = scan(file, stack);
    if(token.type == T_EOL) {
        fprintf(stderr, "Syntax error. Expected EOL, got  %s token.\n", token_to_string(token));
        return false;
    }

    return true;
}

char* token_to_string (token_t token) {
    switch(token.type) {

        case T_EOL:
            return "EOL";
        case T_EOF:
            return "EOF";
        case T_OP_NEG:
            return "'!='";
        case T_OP_ADD:
            return "'+'";
        case T_OP_SUB:
            return "'-'";
        case T_OP_MUL:
            return "'*'";
        case T_OP_DIV:
            return "'/'";
        case T_OP_EQ:
            return "'=='";
        case T_OP_GREATER:
            return "'>'";
        case T_OP_LESS:
            return "'<'";
        case T_OP_GREATER_EQ:
            return "'>='";
        case T_OP_LESS_EQ:
            return "'<='";
        case T_OP_NOT_EQ:
            return "'!='";
        case T_BOOL_AND:
            return "'and'";
        case T_BOOL_OR:
            return "'or'";
        case T_BOOL_NEG:
            return "'not'";
        case T_NUMBER:
            return "NUMBER";
        case T_FLOAT:
            return "FLOAT";
        case T_STRING:
            return "string";
        case T_STRING_ML:
            return "multiline string";
        case T_ID:
            return "identifier";
        case T_KW_DEF:
            return "'def'";
        case T_KW_IF:
            return "'if'";
        case T_KW_ELSE:
            return "'else'";
        case T_KW_WHILE:
            return "'while'";
        case T_KW_PASS:
            return "'pass'";
        case T_KW_RETURN:
            return "'return'";
        case T_ASSIGN:
            return "'='";
        case T_COLON:
            return "':'";
        case T_COMMA:
            return "','";
        case T_LPAR:
            return "'('";
        case T_RPAR:
            return "')'";
        case T_LBRACKET:
            return "[";
        case T_RBRACKET:
            return "]";
        case T_LBRACE:
            return "{";
        case T_RBRACE:
            return "}";
        case T_INDENT:
            return "indentation";
        case T_DEDENT:
            return "dedentation";
        case T_UNKNOWN:
            return "unknown";
        case T_NONE:
            return "'none'";
        case T_ERROR:
            return "'error'";
    }
    return "";
}
