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
statementPart_t while_s[] = {S_KW_WHILE, S_EXPRESSION, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT};
statementPart_t if_s[] = {S_KW_IF, S_EXPRESSION, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT};
statementPart_t pass_s[] = {S_KW_PASS, S_EOL};
statementPart_t return_s[] = {S_KW_RETURN, S_EXPRESSION, S_EOL};
statementPart_t else_s[] = {S_KW_ELSE, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT};
statementPart_t functionDef_s[] = {S_KW_DEF, S_ID, S_LPAR, S_DEF_PARAMS, S_COLON, S_EOL, S_INDENT, S_BLOCK, S_DEDENT};
statementPart_t functionCall_s[] = {S_ID, S_LPAR, S_CALL_PARAMS, S_RPAR};

signed int precedenceTable[19][19] =
	{
	//    *  /  // +  -  <  > <= >= == && ||  ! !=  (  ) id  $
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1,-1, 1,-1, 1},// *
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1,-1, 1,-1, 1},// /
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1,-1, 1,-1, 1},// //
		{-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1,-1, 1,-1, 1},// +
		{-1,-1,-1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1, 1,-1, 1,-1, 1},// -
		{-1,-1,-1,-1,-1,-5,-5,-5,-5,-5, 1, 1,-1,-5,-1, 1,-1, 1},// <
		{-1,-1,-1,-1,-1,-5,-5,-5,-5,-5, 1, 1,-1,-5,-1, 1,-1, 1},// >
		{-1,-1,-1,-1,-1,-5,-5,-5,-5,-5, 1, 1,-1,-5,-1, 1,-1, 1},// <=
		{-1,-1,-1,-1,-1,-5,-5,-5,-5,-5, 1, 1,-1,-5,-1, 1,-1, 1},// >=
		{-1,-1,-1,-1,-1,-5,-5,-5,-5,-5, 1, 1,-1,-5,-1, 1,-1, 1},// ==
		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1, 1,-1, 1},// &&
		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1, 1,-1, 1},// ||
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-1,-1, 1,-1, 1,-1, 1},// !
		{-1,-1,-1,-1,-1,-5,-5,-5,-5,-5, 1, 1,-1,-5,-1, 1,-1, 1},// !=
		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0,-1, 0},// (
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,-5, 1,-5, 1},// )
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1,-5, 1},// id
		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-5,-1,-5} // $
	};

treeElement_t syntaxParse(FILE* file) {

    intStack_t* intStack = stackInit();
    stackPush(intStack, 0);
    tokenStack_t* tokenStack = tokenStackInit(file, intStack);
    treeElement_t tree;
    treeInit(&tree, E_CODE);

    int errCode = ERROR_SUCCESS;
    while(tokenStackTop(tokenStack, &errCode).type != T_EOF) {
    	if(errCode != ERROR_SUCCESS) { //Lexical analysis error
    		break;
    	}
		errCode = parseBlock(tokenStack, &tree);
        if (errCode != ERROR_SUCCESS) {
            break;
        }

        token_t token = tokenStackTop(tokenStack, &errCode);
        if(errCode != ERROR_SUCCESS) //Lexical analysis error
        	break;

        if (token.type == T_KW_DEF) {
            parseFunctionDef(tokenStack, &tree);
        } else if(token.type == T_EOF) {
            break;
        } else {
            fprintf(stderr, "Syntax error. Expected 'def', got %s \n", tokenToString(tokenStackTop(tokenStack, &errCode).type));
            break;
        }

    }
    if(errCode == ERROR_SUCCESS) {
		errCode = processToken(tokenStack, T_EOF, &tree);
	}
    tokenStackFree(tokenStack);
    stackFree(intStack);

    if(errCode != ERROR_SUCCESS){
    	treeFree(tree);
    	exit(errCode); //Exit with correct error code
    }
    return tree;
}

int processToken(tokenStack_t* stack, enum token_type type, treeElement_t* tree) {
	int errCode = ERROR_SUCCESS;
    token_t token = tokenStackPop(stack, &errCode);
    if(errCode != ERROR_SUCCESS) {
		return errCode;
    }
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
        return ERROR_SYNTAX;
    }

    switch (token.type) {
    	case T_ID:
    	case T_NUMBER:
		case T_FLOAT:
		case T_STRING:
    	case T_STRING_ML:
    	case T_KW_NONE:
		case T_KW_PASS:
			treeAddToken(tree, token);
    		break;

		default:
			break;
    }
    return ERROR_SUCCESS;
}

int processStatementPart(tokenStack_t* stack, statementPart_t part, treeElement_t* tree) {
    enum token_type tokenType;
    int errCode = ERROR_SUCCESS;
    switch(part) {
        case S_BLOCK:
			errCode = parseBlock(stack, tree);
			if (errCode != ERROR_SUCCESS)
				return errCode;
			break;

        case S_EXPRESSION:
        	errCode = parseExpression(stack, tree);
            if(errCode != ERROR_SUCCESS)
                return errCode;
            break;

        case S_DEF_PARAMS:
        	errCode = parseFunctionDefParams(stack, tree);
            if(errCode != ERROR_SUCCESS)
                return errCode;
            break;

        case S_CALL_PARAMS:
        	errCode = parseFunctionCallParams(stack, tree);
            if(errCode != ERROR_SUCCESS)
                return errCode;
            break;

        default:
        	errCode = statementPartToTokenType(part, &tokenType);
            if(errCode == ERROR_SUCCESS) {
				errCode = processToken(stack, tokenType, tree);
                if(errCode != ERROR_SUCCESS)
                    return errCode;
            } else {
                return errCode;
            }
    }

    return ERROR_SUCCESS;
}

int parseFunctionDef(tokenStack_t* stack, treeElement_t* tree) {
	int errCode = ERROR_SUCCESS;
    size_t partSize = sizeof(functionDef_s) / sizeof(functionDef_s[0]); //Get number of statement parts
    treeElement_t* defFunTree = treeAddElement(tree, E_S_FUNCTION_DEF);

    for(size_t i = 0; i < partSize; i++){
		errCode = processStatementPart(stack, functionDef_s[i], defFunTree);
        if(errCode != ERROR_SUCCESS)
            return errCode;
    }

    return errCode;
}

int parseFunctionDefParams(tokenStack_t* stack, treeElement_t* tree) {
	int errCode = ERROR_SUCCESS;
    bool parameters = true;
    treeElement_t* defParamTree = NULL;
    while(parameters) {
        token_t token = tokenStackPop(stack, &errCode);
        if(errCode != ERROR_SUCCESS) {
			return errCode;
        }
        switch (token.type) {
            case T_ID: // new local variable as function parameter
            	if(defParamTree == NULL) {
					defParamTree = treeAddElement(tree, E_S_FUNCTION_DEF_PARAMS);
            	}
                treeAddToken(defParamTree, token);
            	token_t lookAheadToken = tokenStackTop(stack, &errCode);
                if(lookAheadToken.type == T_COMMA) {
					tokenStackPop(stack, &errCode); // multiple parameters pop comma
					if(errCode != ERROR_SUCCESS)
						return errCode;
				}
                break;

            case T_RPAR:
                parameters = false;
                break;

            default:
            	lookAheadToken = tokenStackTop(stack, &errCode);
            	if(errCode != ERROR_SUCCESS) {
					return errCode;
            	}
                fprintf(stderr, "Syntax error. Expected identifier, got %s \n", tokenToString(lookAheadToken.type));
                return ERROR_SYNTAX;
        }
    }
    return ERROR_SUCCESS;
}

int getTokenTableId(enum token_type type) {
	switch (type){
		case T_OP_NEG:
			return 0;
		case T_OP_ADD:
			return 3;
		case T_OP_SUB:
			return 4;
		case T_OP_MUL:
			return 0;
		case T_OP_DIV:
			return 1;
		case T_OP_IDIV:
			return 2;
		case T_OP_ADD:
			return 3;
		case T_OP_SUB:
			return 4;
		case T_OP_LESS:
			return 5;
		case T_OP_GREATER:
			return 6;
		case T_OP_GREATER_EQ:
			return 9;
		case T_OP_LESS_EQ:
			return 7;
		case T_OP_GREATER_EQ:
			return 8;
		case T_OP_EQ:
			return 9;
		case T_BOOL_AND:
			return 10;
		case T_BOOL_OR:
			return 11;
		case T_BOOL_NEG:
			return 12;
		case T_OP_NOT_EQ:
			return 13;
		case T_LPAR:
			return 14;
		case T_RPAR:
			return 15;
		case T_NUMBER:
			return 17;
		case T_FLOAT:
			return 17;
		case T_STRING:
			return 17;
		case T_STRING_ML:
			return 17;
		case T_ID:
			return 17;
		case T_ASSIGN:
			return 5;
		case T_LPAR:
			return 15;
		case T_KW_NONE:
			return 17;
		case T_RPAR:
			return 16;
		default:
			return 17;
	}
}

bool isTokenGreater(enum token_type a, enum token_type b, int* result) {
	int tableResult = precedenceTable[getTokenTableId(a)][getTokenTableId(b)];
	if(tableResult == -5)
		return false;

	*result = tableResult;
	return true;
}

enum token_type tokenTypeFromElement(treeElement_t element) {
	switch(element.type) {
		case E_ADD:
			return T_OP_ADD;
		case E_SUB:
			return T_OP_SUB;
		case E_MUL:
			return T_OP_MUL;
		case E_DIV:
			return T_OP_DIV;
		case E_EQ:
			return T_OP_EQ;
		case E_GT:
			return T_OP_GREATER_EQ;
		case E_GTE:
			return T_OP_GREATER_EQ;
		case E_LT:
			return T_OP_LESS;
		case E_LTE:
			return T_OP_LESS_EQ;
		case E_AND:
			return T_BOOL_AND;
		case E_OR:
			return T_BOOL_OR;
		case E_NOT:
			return T_BOOL_NEG;
		case E_ASSIGN:
			return T_ASSIGN;
		case E_NEQ:
			return T_OP_NOT_EQ;
		case E_DIV_INT:
			return T_OP_IDIV;
		case E_S_FUNCTION_CALL:
			return T_ID;

		case E_TOKEN:
			return ((token_t*)element.data)->type;
		default:
			return T_EOL; //TODO: Return something meaningful on error
	}
}

int parseExpression(tokenStack_t* stack, treeElement_t* tree){
	int errCode = 0;
int parseExpression(tokenStack_t* stack, treeElement_t* tree, bool includeRoot){
	int errCode = ERROR_SUCCESS;
	token_t topToken = tokenStackTop(stack, &errCode);
	if(errCode != ERROR_SUCCESS) {
		return errCode;
	}
	if(getTokenTableId(topToken.type) == 17) // If token is unrecognized by precedence analysis
		return ERROR_SYNTAX; //Syntax error
	treeElement_t* expressionTree;
	expressionTree = treeAddElement(tree, E_S_EXPRESSION);

    treeStack_t* resultStack = treeStackInit();// stack for result tree
    treeStack_t* precedenceStack = treeStackInit();

    token_t firstToken = {.type = T_EOL, .data = {.strval = NULL}}; // Initial token set to $
    treeElement_t firstElement;
    initTokenTreeElement(&firstElement, firstToken);
    treeStackPush(precedenceStack, firstElement);

    while(true) { // Probably not the best approach
		token_t token = tokenStackPop(stack, &errCode);
		if(errCode != ERROR_SUCCESS){
			treeStackFree(resultStack);
			treeStackFree(precedenceStack);
			return errCode;
		}
		treeElement_t element;
		int greater = 0;

		topToken = tokenStackTop(stack, &errCode);
		if(errCode != ERROR_SUCCESS) { // Lexical analysis error
			treeStackFree(resultStack);
			treeStackFree(precedenceStack);
			return errCode;
		}

		if (token.type == T_ID && topToken.type == T_LPAR) {
			tokenStackPush(stack, token);
			errCode = parseFunctionCall(stack, &element);
			if(errCode != ERROR_SUCCESS) {
				treeFree(element);
				treeStackFree(resultStack);
				treeStackFree(precedenceStack);
				return errCode;
			}
		}else {
			treeElementType_t type;
			tokenToTreeElement(token.type, &type);
			if(type == E_TOKEN){
				initTokenTreeElement(&element, token);
			} else {
				treeInit(&element, type);
			}
		}

		if(tokenTypeFromElement(treeStackTop(precedenceStack)) == T_EOL && ((token_t*)element.data)->type == T_RPAR) {
			element = treeStackPop(resultStack);
			treeInsertElement(expressionTree, element);
			tokenStackPush(stack, token); // Push back last token (Not part of expression)
			treeStackFree(resultStack);
			treeStackFree(precedenceStack);
			return 0;
		} else if(!isTokenGreater(tokenTypeFromElement(treeStackTop(precedenceStack)), tokenTypeFromElement(element), &greater)) {
			fprintf(stderr, "EXPRESSION: Token unrecognized");
			treeStackFree(resultStack);
			treeStackFree(precedenceStack);
			return ERROR_SYNTAX;
		}

		if(greater == -1){
			treeStackPush(precedenceStack, element);
		} else {
			while(greater == 1) {
				treeElement_t operation = treeStackPop(precedenceStack);
				switch (operation.type) {

					case E_ADD:
					case E_SUB:
					case E_MUL:
					case E_DIV:
					case E_EQ:
					case E_GT:
					case E_GTE:
					case E_LT:
					case E_LTE:
					case E_AND:
					case E_OR:
					case E_NOT:
					case E_ASSIGN:
					case E_NEQ:
					case E_DIV_INT: {
						treeElement_t second = treeStackPop(resultStack); //If popped element is operator
						treeElement_t first = treeStackPop(resultStack);

						treeInsertElement(&operation, first);
						treeInsertElement(&operation, second);

						if (tokenTypeFromElement(treeStackTop(precedenceStack)) == T_EOL && getTokenTableId(token.type) == 17) { // Expression END
							treeInsertElement(expressionTree, operation);
							tokenStackPush(stack, token); // Push back last token (Not part of expression)
							treeFree(element);
							treeStackFree(resultStack);
							treeStackFree(precedenceStack);
							return ERROR_SUCCESS;
						} else {
							treeStackPush(resultStack, operation);
						}
						break;
					}

					default:
						treeStackPush(resultStack, operation);
						break;
				}

				if(!isTokenGreater(tokenTypeFromElement(treeStackTop(precedenceStack)), tokenTypeFromElement(element), &greater)) { // Expression END
					greater = false;
					operation = treeStackPop(resultStack);
					treeFree(element);
					treeInsertElement(expressionTree, operation);
					tokenStackPush(stack, token); // Push back last token (Not part of expression)
					treeStackFree(resultStack);
					treeStackFree(precedenceStack);
					return ERROR_SUCCESS;
				}
			}

			if(greater == 0){
				if(treeStackTop(precedenceStack).type == E_TOKEN && ((token_t*)treeStackTop(precedenceStack).data)->type == T_EOL) {
					greater = false;
					treeFree(element);
					treeInsertElement(expressionTree, treeStackPop(resultStack)); // Insert final operation into expression tree
					tokenStackPush(stack, token); // Push back last token (Not part of expression)
					treeStackFree(resultStack);
					treeStackFree(precedenceStack);
					return ERROR_SUCCESS;
				}
				treeStackPop(precedenceStack); // Pop ( operator from stack
			} else {
				treeStackPush(precedenceStack, element); //Push operator to stack after popping
			}
		}
    }
}

int parseWhile(tokenStack_t* stack, treeElement_t* tree) {
	treeElement_t* whileTree = treeAddElement(tree, E_S_WHILE);
	int errCode = ERROR_SUCCESS;
    size_t partSize = sizeof(while_s) / sizeof(while_s[0]); //Get number of statement parts

    for(size_t i = 0; i < partSize; i++){
		errCode = processStatementPart(stack, while_s[i], whileTree);
        if(errCode != ERROR_SUCCESS)
            return errCode;
    }

    return ERROR_SUCCESS;
}

int parseBlock(tokenStack_t* stack, treeElement_t* tree) {
    /*
     * If parsing of any block-statement fails block parsing fails.
     * unexpected block-tokens are not popped from stack
    **/
    int errCode = ERROR_SUCCESS;
    bool tokenRecognized = true;
    token_t token;
	treeElement_t* blockTree = NULL;
    while(tokenRecognized) {
        token = tokenStackTop(stack, &errCode);
        if(errCode != ERROR_SUCCESS) {
			return errCode;
        }
        switch (token.type) {
            case T_KW_IF:
				if(blockTree == NULL)
					blockTree = treeAddElement(tree, E_CODE_BLOCK);

				errCode = parseIf(stack, blockTree);
                if(errCode != ERROR_SUCCESS)
                    return errCode;
                break;

            case T_KW_WHILE:
				if(blockTree == NULL)
					blockTree = treeAddElement(tree, E_CODE_BLOCK);

				errCode = parseWhile(stack, blockTree);
                if (errCode != ERROR_SUCCESS)
                    return errCode;
                break;

            case T_KW_PASS:
				if(blockTree == NULL)
					blockTree = treeAddElement(tree, E_CODE_BLOCK);

				errCode = parsePass(stack, blockTree);
                if (errCode != ERROR_SUCCESS)
                    return errCode;
                break;

            case T_KW_RETURN:
				if(blockTree == NULL)
					blockTree = treeAddElement(tree, E_CODE_BLOCK);

				errCode = parseReturn(stack, blockTree);
                if(errCode != ERROR_SUCCESS)
                    return errCode;
                break;

            case T_ID:
				if(blockTree == NULL)
					blockTree = treeAddElement(tree, E_CODE_BLOCK);

				token = tokenStackPop(stack, &errCode);
				if(tokenStackTop(stack, &errCode).type == T_ASSIGN){
					tokenStackPush(stack, token);
					errCode = parseAssignment(stack, blockTree);
					if(errCode != ERROR_SUCCESS)
						return errCode;
				} else {
					tokenStackPush(stack, token);
					errCode = parseExpression(stack, blockTree);
					if(errCode != ERROR_SUCCESS)
						return errCode;
				}

                errCode = processToken(stack, T_EOL, blockTree);
                if(errCode != ERROR_SUCCESS) // Newline after expression
                        return errCode;
                break;

                case T_NUMBER:
                case T_LPAR:
                case T_STRING:
                case T_STRING_ML:
                case T_FLOAT: { // ignore expressions if their result is not used
					do{
						token = tokenStackPop(stack, &errCode);
						if(errCode != ERROR_SUCCESS) {
							return errCode;
						}
						switch(token.type){
							case T_STRING:
							case T_ID:
							case T_STRING_ML:
								dynStrFree(token.data.strval);
								break;

							default:
								break;
						}
					} while (token.type != T_EOL);
					break;
				}

            default:
                tokenRecognized = false;
        }
    }
    return ERROR_SUCCESS;
}


int parseIf(tokenStack_t* stack, treeElement_t* tree) {
	int errCode = ERROR_SUCCESS;
	treeElement_t* ifTree = treeAddElement(tree, E_S_IF);
    size_t partSize = sizeof(if_s) / sizeof(if_s[0]); //Get number of statement parts

    for(size_t i = 0; i < partSize; i++){
    	errCode = processStatementPart(stack, if_s[i], ifTree);
        if(errCode != ERROR_SUCCESS)
            return errCode;
    }

    token_t token = tokenStackTop(stack, &errCode);
    if(errCode != ERROR_SUCCESS) {
		return errCode;
    }
    if(token.type == T_KW_ELSE) {
		errCode = parseElse(stack, ifTree);
        if(errCode != ERROR_SUCCESS)
			return errCode;
    }

    return ERROR_SUCCESS;
}

int parseElse(tokenStack_t* stack, treeElement_t* tree) {
	int errCode = ERROR_SUCCESS;
    size_t partSize = sizeof(else_s) / sizeof(else_s[0]); //Get number of statement parts

    treeElement_t* elseTree = treeAddElement(tree, E_S_ELSE);

    for(size_t i = 0; i < partSize; i++){
		errCode = processStatementPart(stack, else_s[i], elseTree);
        if(errCode != ERROR_SUCCESS)
            return errCode;
    }

    return ERROR_SUCCESS;
}

int parseReturn(tokenStack_t* stack, treeElement_t* tree) {
	int errCode = ERROR_SUCCESS;
    size_t partSize = sizeof(return_s) / sizeof(return_s[0]); //Get number of statement parts

    treeElement_t* returnTree = treeAddElement(tree, E_S_RETURN);

    for(size_t i = 0; i < partSize; i++){
    	errCode = processStatementPart(stack, return_s[i], returnTree);
        if(errCode != ERROR_SUCCESS)
            return errCode;
    }

    return ERROR_SUCCESS;
}

int parseFunctionCall(tokenStack_t* stack, treeElement_t* tree) {
	int errCode = ERROR_SUCCESS;
    size_t partSize = sizeof(functionCall_s) / sizeof(functionCall_s[0]); //Get number of statement parts

     treeInit(tree, E_S_FUNCTION_CALL);

    for(size_t i = 0; i < partSize; i++){
    	errCode = processStatementPart(stack, functionCall_s[i], tree);
        if(errCode != ERROR_SUCCESS)
            return errCode;
    }

    return ERROR_SUCCESS;
}

int parseFunctionCallParams(tokenStack_t* stack, treeElement_t* tree) {
    bool parameters = true;
    treeElement_t* paramsTree = NULL;
    int errCode = ERROR_SUCCESS;
    token_t token;
    while(parameters) {
		token = tokenStackTop(stack, &errCode);
		if(errCode != ERROR_SUCCESS) {
			return errCode;
		}
		if(token.type == T_RPAR) {
			break;
		}
		if(paramsTree == NULL)
			paramsTree = treeAddElement(tree, E_S_FUNCTION_CALL_PARAMS);

		errCode = parseExpression(stack, paramsTree);
        if(errCode != ERROR_SUCCESS)
            return errCode;

        token = tokenStackTop(stack, &errCode);
        if(errCode != ERROR_SUCCESS)
			return errCode;

        if(token.type == T_COMMA) {
			tokenStackPop(stack, &errCode);// pop comma token if multiple parameters
			if(errCode != ERROR_SUCCESS)
				return errCode;
		} else if(token.type == T_RPAR) {
			parameters = false;
		}
    }
    return ERROR_SUCCESS;
}


int parsePass(tokenStack_t* stack, treeElement_t* tree) {
	int errCode = ERROR_SUCCESS;
    size_t partSize = sizeof(pass_s) / sizeof(pass_s[0]); //Get number of statement parts

    treeElement_t* pass = treeAddElement(tree, E_S_PASS);

    for(size_t i = 0; i < partSize; i++){
		errCode = processStatementPart(stack, pass_s[i], pass);
        if(errCode != ERROR_SUCCESS)
            return errCode;
    }

    return ERROR_SUCCESS;
}

int parseAssignment(tokenStack_t* stack, treeElement_t* tree) {
	int errCode = ERROR_SUCCESS;
	treeElement_t* assignTree = treeAddElement(tree, E_ASSIGN);
	errCode = processToken(stack, T_ID, assignTree);
	if(errCode != ERROR_SUCCESS)
		return errCode;

	errCode = processToken(stack, T_ASSIGN, assignTree);
	if(errCode != ERROR_SUCCESS)
		return errCode;

	token_t token = tokenStackTop(stack, &errCode);
	if(errCode != ERROR_SUCCESS)
		return errCode;

	if(token.type == T_ID) {
		token = tokenStackPop(stack, &errCode);
		token_t topToken = tokenStackTop(stack, &errCode);
		if(errCode != ERROR_SUCCESS)
			return errCode;
		if(topToken.type == T_ASSIGN){
			tokenStackPush(stack, token);
			parseAssignment(stack, assignTree);
		} else {
			tokenStackPush(stack, token);
			parseExpression(stack, assignTree);
		}

	} else {
		errCode = parseExpression(stack, assignTree);
	}

	return errCode;
}

int statementPartToTokenType(statementPart_t part, enum token_type* type){
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
            return ERROR_SUCCESS;

        default:
            return ERROR_SYNTAX;
    }
}
