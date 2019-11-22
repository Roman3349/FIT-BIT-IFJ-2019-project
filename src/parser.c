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

//Statement definitions
statementPart_t while_s[] = {S_KW_WHILE, S_EXPRESSION, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT };
statementPart_t if_s[] = {S_KW_IF, S_EXPRESSION, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT};
statementPart_t pass_s[] = {S_KW_PASS, S_EOL};
statementPart_t return_s[] = {S_KW_RETURN, S_EXPRESSION, S_EOL};
statementPart_t else_s[] = {S_KW_ELSE, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT};
statementPart_t functionDef_s[] = {S_KW_DEF, S_ID, S_LPAR, S_DEF_PARAMS, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT};
statementPart_t functionCall_s[] = {S_ID, S_LPAR, S_CALL_PARAMS};

treeElement_t syntaxParse(FILE* file) {

    intStack_t* intStack = stackInit();
    tokenStack_t* tokenStack = tokenStackInit(file, intStack);
    while(tokenStackTop(tokenStack).type != T_EOF) {
        if (!parseBlock(tokenStack)) {
            fprintf(stderr, "!!!!!!!!!!!!! SYNTAX ERROR !!!!!!!!!!!!!\n");
            break;
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

bool processStatementPart(tokenStack_t* stack, statementPart_t part) {
    enum token_type tokenType;
    switch(part) {
        case S_BLOCK:
            if(!parseBlock(stack))
                return false;
            break;
        case S_EXPRESSION:
            if(!parseExpression(stack))
                return false;
            break;
        case S_DEF_PARAMS:
            if(!parseFunctionDefParams(stack))
                return false;
            break;
        case S_CALL_PARAMS:
            if(!parseFunctionCallParams(stack))
                return false;
            break;

        default:
            if(statementPartToTokenType(part, &tokenType)) {
                if(!processToken(stack, tokenType))
                    return false;
            } else {
                return false;
            }
    }

    return true;
}

bool parseFunctionDef(tokenStack_t* stack) {
    size_t partSize = sizeof(functionDef_s) / sizeof(functionDef_s[0]); //Get number of statement parts

    for(size_t i = 0; i < partSize; i++){
        if(!processStatementPart(stack, functionDef_s[i]))
            return false;
    }

    return true;
}

bool parseFunctionDefParams(tokenStack_t* stack) {
    bool parameters = true;
    while(parameters) {
        token_t token = tokenStackPop(stack);
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
    return true;
}

bool parseExpression(tokenStack_t* stack){
    bool expression = true;
    while(expression) {
        token_t token = tokenStackTop(stack);
        switch (token.type) {
            case T_LPAR:
                tokenStackPop(stack); //Pop ( from the stack
                if(!parseExpression(stack))
                    return false;
                processToken(stack, T_RPAR);
                break;


            case T_NUMBER:
            case T_FLOAT:
            case T_STRING:
            case T_ID:
            case T_STRING_ML:
                token = tokenStackPop(stack);
                if(token.type == T_ID){
                    if(tokenStackTop(stack).type == T_LPAR) { // operator is function call
                        tokenStackPush(stack, token); //Push back function ID
                        parseFunctionCall(stack);
                    }
                }

                printf("EXPRESSION OPERAND: %s\n", tokenToString(token.type));
                token_t test;

                switch(tokenStackTop(stack).type){
                    case T_OP_ADD:
                    case T_OP_SUB:
                    case T_OP_MUL:
                    case T_OP_DIV:
                    case T_OP_EQ:
                    case T_ASSIGN:
                    case T_OP_GREATER:
                    case T_OP_GREATER_EQ:
                    case T_OP_LESS:
                    case T_OP_LESS_EQ:
                    case T_OP_NEG:
                    case T_OP_NOT_EQ:
                        test = tokenStackPop(stack);

                        printf("EXPRESSION OPERATOR: %s\n", tokenToString(test.type));
                        break;

                    default:
                        expression = false;
                }
                break;

            default:
                expression = false;
        }
    }

    return true;
}

bool parseWhile(tokenStack_t* stack) {
    size_t partSize = sizeof(while_s) / sizeof(while_s[0]); //Get number of statement parts

    for(size_t i = 0; i < partSize; i++){
        if(!processStatementPart(stack, while_s[i]))
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
    token_t token;

    while(tokenRecognized) {
        token = tokenStackTop(stack);
        printf("block parsing: %s\n", tokenToString(token.type)); //TODO: REMOVE debug print
        switch (token.type) {
            case T_KW_IF:
                if(!parseIf(stack))
                    return false;
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
                if(!parseExpression(stack))
                        return false;
                if(!processToken(stack, T_EOL)) // Newline after expression
                        return false;
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
    size_t partSize = sizeof(if_s) / sizeof(if_s[0]); //Get number of statement parts

    for(size_t i = 0; i < partSize; i++){
        if(!processStatementPart(stack, if_s[i]))
            return false;
    }

    if(tokenStackTop(stack).type == T_KW_ELSE) {
        parseElse(stack);
    }

    return true;
}

bool parseElse(tokenStack_t* stack) {
    size_t partSize = sizeof(else_s) / sizeof(else_s[0]); //Get number of statement parts

    for(size_t i = 0; i < partSize; i++){
        if(!processStatementPart(stack, else_s[i]))
            return false;
    }

    return true;
}

bool parseReturn(tokenStack_t* stack) {
    size_t partSize = sizeof(return_s) / sizeof(return_s[0]); //Get number of statement parts

    for(size_t i = 0; i < partSize; i++){
        if(!processStatementPart(stack, return_s[i]))
            return false;
    }

    return true;
}

bool parseFunctionCall(tokenStack_t* stack) {
    size_t partSize = sizeof(functionCall_s) / sizeof(functionCall_s[0]); //Get number of statement parts

    for(size_t i = 0; i < partSize; i++){
        if(!processStatementPart(stack, functionCall_s[i]))
            return false;
    }

    return true;
}

bool parseFunctionCallParams(tokenStack_t* stack) {
    bool parameters = true;
    while(parameters) {
        if(!parseExpression(stack))
            return false;

        if(tokenStackTop(stack).type == T_COMMA)
            tokenStackPop(stack);// pop comma token if multiple parameters
        else if(tokenStackTop(stack).type == T_RPAR)
            parameters = false;
    }
    return true;
}


bool parsePass(tokenStack_t* stack) {
    size_t partSize = sizeof(pass_s) / sizeof(pass_s[0]); //Get number of statement parts

    for(size_t i = 0; i < partSize; i++){
        if(!processStatementPart(stack, pass_s[i]))
            return false;
    }

    return true;
}

bool statementPartToTokenType(statementPart_t part, enum token_type* type){
    switch(part){
        case S_EOL:
        case S_INDENT:
        case S_DEDENT:
        case S_COLON:
        case S_LPAR:
        case S_RPAR:
        case S_ID:
        case S_KW_IF:
        case S_KW_PASS:
        case S_KW_RETURN:
        case S_KW_ELSE:
        case S_KW_DEF:
        case S_KW_WHILE:
            *type = (enum token_type)part;
            return true;

        default:
            return false;
    }
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
