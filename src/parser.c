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

#include "parser.h"
#include "token_stack.h"

//#include "scanner.h"

treeElement_t syntaxParse(FILE* file) {

    intStack_t* intStack = stackInit();
    tokenStack_t* tokenStack = tokenStackInit(file, intStack);
    while(tokenStackTop(tokenStack).type != T_EOF) {
        if (!parseBlock(tokenStack)) {
            fprintf(stderr, "!!!!!!!!!!!!! SYNTAX ERROR !!!!!!!!!!!!!\n");
        }

        if (tokenStackTop(tokenStack).type == T_KW_DEF) {
            parseFunctionDef(tokenStack);
        } else if(tokenStackTop(tokenStack).type == T_EOF) {
            break;
        } else {
            fprintf(stderr, "Syntax error. Expected 'def', got %s \n", tokenToString(tokenStackTop(tokenStack).type));
            break;
        }

    }
    printf("Last token: %s\n", tokenToString(tokenStackTop(tokenStack).type)); //TODO: REMOVE debug print

    treeElData_t data = { .d=1000 };
    treeElement_t tree = { .data = &data };
    tokenStackFree(tokenStack);
    stackFree(intStack);
    return tree;
}

bool processToken(tokenStack_t* stack, enum token_type type) {
    token_t token = tokenStackPop(stack);
    if(token.type != type){
        switch (type){
            case T_INDENT:
            case T_DEDENT:
                fprintf(stderr, "Syntax error. Invalid indentation near: %s \n", tokenToString(token.type));
                break;

            default:
                fprintf(stderr, "Syntax error. Expected %s, got: %s \n", tokenToString(type), tokenToString(token.type));
                break;
        }

        return false;
    }
    return true;
}

bool parseFunctionDef(tokenStack_t* stack) {
    token_t token = tokenStackPop(stack);
    if(token.type != T_KW_DEF) {
        fprintf(stderr, "Parsing 'def' with %s token.\n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_ID) {
        fprintf(stderr, "Syntax error. Expected identifier, got %s \n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_LPAR) {
        fprintf(stderr, "Syntax error. Expected '(', got %s \n", tokenToString(token.type));
        return false;
    }

    bool parameters = true;

    while(parameters) {
        token = tokenStackPop(stack);
        switch (token.type) {
            case T_ID: // new local variable as function parameter
                printf("function parameter: %s\n", tokenToString(token.type));
                if(tokenStackTop(stack).type == T_COMMA)
                    tokenStackPop(stack); // multiple parameters pop comma
                break;

            case T_RPAR:
                parameters = false;
                break;

            default:
                fprintf(stderr, "Syntax error. Expected identifier, got %s \n", tokenToString(token.type));
                return false;
        }
    }

    token = tokenStackPop(stack);
    if(token.type != T_COLON) {
        fprintf(stderr, "Syntax error. Expected ':', got %s \n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_EOL) {
        fprintf(stderr, "Syntax error. Expected EOL, got %s \n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_INDENT) {
        fprintf(stderr, "Syntax error. Invalid indentation near: %s \n", tokenToString(tokenStackTop(stack).type));
        return false;
    }

    if(!parseBlock(stack))
        return false;

    token = tokenStackPop(stack);
    if(token.type != T_DEDENT) {
        fprintf(stderr, "Syntax error. Invalid indentation near: %s \n", tokenToString(tokenStackTop(stack).type));
        return false;
    }
    return true;
}

bool parseExpression(tokenStack_t* stack){
    printf("expression: %s\n", tokenToString(tokenStackPop(stack).type));
    printf("expression: %s\n", tokenToString(tokenStackPop(stack).type));
    printf("expression: %s\n", tokenToString(tokenStackPop(stack).type));
    return true; //FIXME: Expression grammar parser (temporary scrap 3 tokens)
}

bool parseWhile(tokenStack_t* stack) {
    token_t token = tokenStackPop(stack);
    if(token.type != T_KW_WHILE) {
        fprintf(stderr, "Parsing 'while' with %s token.\n", tokenToString(token.type));
        return false;
    }

    if(!parseExpression(stack)) {
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_COLON) {
        fprintf(stderr, "Syntax error. Expected ':', got %s \n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_EOL) {
        fprintf(stderr, "Syntax error. Expected EOL, got %s \n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack) ;
    if(token.type != T_INDENT) {
        fprintf(stderr, "Syntax error. Expected indentation, got %s \n", tokenToString(token.type));
        return false;
    }

    if(!parseBlock(stack)) {
        return false;
    }


    token = tokenStackPop(stack);
    if(token.type != T_DEDENT) {
        fprintf(stderr, "Syntax error. Invalid indentation near %s\n", tokenToString(tokenStackTop(stack).type));
        return false;
    }

    return true;
}

bool parseAssignment(tokenStack_t* stack) {
    token_t token = tokenStackPop(stack);
    if(token.type != T_ASSIGN) {
        fprintf(stderr, "Parsing 'assignment' with  %s token.\n", tokenToString(token.type));
        return false;
    }

    if(!parseExpression(stack)){
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_EOL) {
        fprintf(stderr, "Syntax error. Expected EOL, got  %s token.\n", tokenToString(token.type));
        return false;
    }

    return true;
}

bool parseBlock(tokenStack_t* stack) {

    /*
     * If parsing of any block-statement fails block parsing fails.
     * unexpected block-tokens results in correct block parsing and returns unexpected token in last_token
    **/
    bool tokenRecognized = true;
    token_t id;
    token_t token;

    while(tokenRecognized) {
        token = tokenStackTop(stack);
        printf("block parsing: %s\n", tokenToString(token.type)); //TODO: REMOVE debug print
        switch (token.type) {
            case T_KW_IF:
                parseIf(stack);
                break;

            case T_KW_WHILE:
                if (!parseWhile(stack))
                    return false;
                break;

            case T_KW_PASS:
                if (!parsePass(stack))
                    return false;
                break;

            case T_KW_RETURN:
                if(!parseReturn(stack))
                    return false;
                break;

            case T_ID:
                id = tokenStackPop(stack); // variable/function ID
                printf("ID: %s\n", id.data.strval->string);// TODO: REMOVE debug print

                if(tokenStackTop(stack).type == T_ASSIGN) { // variable asignment
                    if (!parseAssignment(stack))
                        return false;
                } else if(tokenStackTop(stack).type == T_LPAR){
                    if(!parseFunctionCall(stack))
                        return false;
                } else {
                    tokenStackPush(stack, id); // variable id is part of the expression;
                    if(!parseExpression(stack))
                        return false;
                    if(tokenStackPop(stack).type != T_EOL) {// newline after expression
                        fprintf(stderr, "Syntax error. Expected expression got: %s\n", tokenToString(token.type));
                        return false;
                    }
                }
                break;

                case T_NUMBER:
                case T_LPAR:
                case T_STRING:
                case T_STRING_ML:
                case T_FLOAT: // ignore expressions if their result is not used
                    while(tokenStackPop(stack).type != T_EOL){}
                    break;

            default:
                tokenRecognized = false;
                printf("Block parser end. Last token: %s\n", tokenToString(token.type));// TODO: REMOVE debug print
        }
    }
    return true;
}


bool parseIf(tokenStack_t* stack) {
    token_t token = tokenStackPop(stack);
    if(token.type != T_KW_IF) {
        fprintf(stderr, "Parsing 'if' with %s token.\n", tokenToString(token.type));
        return false;
    }

    if(!parseExpression(stack)) {
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_COLON) {
        fprintf(stderr, "Syntax error. Expected ':' got: %s\n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_EOL) {
        fprintf(stderr, "Syntax error. Expected EOL got: %s\n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_INDENT) {
        fprintf(stderr, "Syntax error. Invalid indentation near: %s\n", tokenToString(tokenStackTop(stack).type));
        return false;
    }

    if(!parseBlock(stack))
        return false;

    token = tokenStackPop(stack);
    if(token.type != T_DEDENT) {
        fprintf(stderr, "Syntax error. Invalid indentation near: %s\n", tokenToString(tokenStackTop(stack).type));
        return false;
    }

    if(tokenStackTop(stack).type == T_KW_ELSE) {
        if(!parseElse(stack)) {
            return false;
        }
    }
    return true;
}

bool parseElse(tokenStack_t* stack) {

    token_t token = tokenStackPop(stack);
    if(token.type != T_KW_ELSE) {
        fprintf(stderr, "Parsing 'else' with %s token.\n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_COLON) {
        fprintf(stderr, "Syntax error. Expected ':' got: %s\n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_EOL) {
        fprintf(stderr, "Syntax error. Expected EOL got:  %s\n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);
    if(token.type != T_INDENT) {
        fprintf(stderr, "Syntax error. Invalid indentation near:  %s\n", tokenToString(tokenStackTop(stack).type));
        return false;
    }

    if(!parseBlock(stack))
        return false;

    token = tokenStackPop(stack);
    if(token.type != T_DEDENT) {
        fprintf(stderr, "Syntax error. Invalid indentation near:  %s\n", tokenToString(tokenStackTop(stack).type));
        return false;
    }

    return false;
}

bool parseReturn(tokenStack_t* stack) {
    token_t token = tokenStackPop(stack);
    if(token.type != T_KW_RETURN) {
        fprintf(stderr, "Parsing 'return' with %s token.\n", tokenToString(token.type));
        return false;
    }

    token = tokenStackTop(stack);
    if(token.type != T_EOL) {
        if(!parseExpression(stack)) {
            return false;
        }
    }

    tokenStackPop(stack); // pop EOL from stack

    return true;
}

bool parseFunctionCall(tokenStack_t* stack) {
    //TODO check function ID in symbol table
    bool parameters = true;
    token_t token = tokenStackPop(stack);

    if(token.type != T_LPAR) {
        fprintf(stderr, "Parsing 'function call' with %s token.\n", tokenToString(token.type));
        return false;
    }

    while(parameters) {
        token = tokenStackPop(stack);
        switch (token.type) {
            case T_NUMBER:
            case T_ID:
            case T_FLOAT:
            case T_STRING:
            case T_STRING_ML:
                if (tokenStackTop(stack).type == T_COMMA || tokenStackTop(stack).type == T_RPAR) {
                    if(token.type == T_ID){ //TODO: DEBUG print of function parameter REMOVE
                        printf("parameter %s\n", token.data.strval->string);
                    } else if (token.type == T_NUMBER) {
                        printf("parameter %ld\n", token.data.intval);
                    }
                } else if (tokenStackTop(stack).type == T_LPAR) { // Nested function calls
                    if (!parseFunctionCall(stack))
                        return false;
                } else {
                    tokenStackPush(stack,token); // Parameter is expression push back id for expresion parser
                    parseExpression(stack);
                }

                if(tokenStackTop(stack).type == T_COMMA)
                    tokenStackPop(stack);// pop comma token if multiple parameters
                break;

            case T_RPAR: {
                parameters = false;
                break;
            }

            default:
                fprintf(stderr, "Syntax error. Expected expression got: %s\n", tokenToString(token.type));
                return false;
        }
    }

    if(tokenStackTop(stack).type == T_EOL) { //TODO: function call inside expression
        tokenStackPop(stack);

    }

    return true;
}


bool parsePass(tokenStack_t* stack) {
    token_t token = tokenStackPop(stack);
    if(token.type != T_KW_PASS) {
        fprintf(stderr, "Parsing 'pass' with %s token.\n", tokenToString(token.type));
        return false;
    }

    token = tokenStackPop(stack);

    if(token.type != T_EOL) {
        fprintf(stderr, "Syntax error. Expected EOL after pass got %s\n", tokenToString(token.type));
        return false;
    }
    return true;
}

char* tokenToString (enum token_type type) {
    switch(type) {
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
            return "STRING";
        case T_STRING_ML:
            return "STRING_MULTILINE";
        case T_ID:
            return "IDENTIFIER";
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
            return "INDENTATION";
        case T_DEDENT:
            return "DEDENTATION";
        case T_UNKNOWN:
            return "UNKNOWN";
        case T_NONE:
            return "NONE";
        case T_ERROR:
            return "ERROR";
        default:
            return "";
    }
}
