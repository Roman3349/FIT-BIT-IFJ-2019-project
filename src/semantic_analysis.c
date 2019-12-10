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

#include "semantic_analysis.h"

void test(treeElement_t* element, symTable_t* symtable, int* errCode) {
	if(element->type == E_ADD){
		checkOperation(element, symtable, errCode, NULL);
		if(*errCode != ERROR_SUCCESS){
			return;
		}
	}
	if(element->nodeSize > 0){
		for(unsigned int i = 0; i < element->nodeSize; i++) {
			test(&element->data.elements[i], symtable, errCode);
		}
	}
}

int semanticCheck(treeElement_t* parseTree, symTable_t* symTable, int* errCode) {
	printTree(*parseTree,0);
	printf("=========================\n");
	test(parseTree, symTable, errCode);
	if(*errCode != ERROR_SUCCESS){
		printf("!!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!!\n");
	}
	printTree(*parseTree,0);
	return 0;
}

void tokenBoolToInt(token_t * token, int* errCode) {
	if(token->type != T_BOOL_FALSE && token->type != T_BOOL_TRUE) {
		*errCode = ERROR_INTERNAL;
		return;
	}


	token->data.intval = (token->type == T_BOOL_TRUE)? 1 : 0;
	token->type = T_NUMBER;
}

void tokenIntToFloat(token_t * token, int* errCode){
	if(token->type != T_NUMBER) {
		*errCode = ERROR_INTERNAL;
		return;
	}

	token->type = T_FLOAT;
	token->data.floatval = (double) token->data.intval;
}



semanticType_t getOperatorType(treeElement_t operatorTree, int* errCode) {
	switch (operatorTree.type){
		case E_ADD:
		case E_SUB:
		case E_MUL:
		case E_DIV:
		case E_GT:
		case E_GTE:
		case E_LT:
		case E_LTE:
		case E_EQ:
		case E_NEQ:
		case E_NOT:
		case E_OR:
		case E_AND:
		case E_DIV_INT:
			return SEMANTIC_EXPRESSION;
		case E_TOKEN:
			switch (operatorTree.data.token->type){
				case T_NUMBER:
					return SEMANTIC_INT;
				case T_FLOAT:
					return SEMANTIC_FLOAT;
				case T_STRING:
				case T_STRING_ML:
					return SEMANTIC_STRING;
				case T_ID:
					return SEMANTIC_VARIABLE;
				case T_BOOL_FALSE:
				case T_BOOL_TRUE:
					return SEMANTIC_BOOL;
				default:
					*errCode = ERROR_SEMANTIC_OTHER;
					return SEMANTIC_UNKNOWN;
			}
		case E_S_FUNCTION_CALL:
			//TODO: IMPLEMENT function call semantic analysis
			return SEMANTIC_UNKNOWN;

		default:
			*errCode = ERROR_SEMANTIC_OTHER;
			return SEMANTIC_UNKNOWN;
	}
}


semanticType_t checkOperation(treeElement_t* expressionTree, symTable_t* symTable, int* errCode, dynStr_t* context){
	switch(expressionTree->type) {

		case E_ADD:
		case E_SUB:
		case E_MUL:
		case E_DIV:
		case E_EQ:
		case E_DIV_INT: {
			treeElement_t operator1 = expressionTree->data.elements[0];
			semanticType_t op1Type = getOperatorType(operator1, errCode);
			if (op1Type == SEMANTIC_EXPRESSION)
				op1Type = checkOperation(&operator1, symTable, errCode, context);

			if (*errCode != ERROR_SUCCESS)
				return SEMANTIC_UNKNOWN;

			treeElement_t operator2 = expressionTree->data.elements[1];
			semanticType_t op2Type = getOperatorType(operator2, errCode);
			if (op2Type == SEMANTIC_EXPRESSION) {
				op2Type = checkOperation(&operator1, symTable, errCode, context);
			}

			if (expressionTree->type == E_ADD) {
				if (op1Type == SEMANTIC_STRING) {
					if (op2Type != SEMANTIC_STRING) {
						*errCode = ERROR_SEMANTIC_EXPRESSION;
						return SEMANTIC_UNKNOWN;
					} else {
						return SEMANTIC_STRING;
					}
				}
			}

			if (*errCode != ERROR_SUCCESS)
				return SEMANTIC_UNKNOWN;

			switch (op1Type) {
				case SEMANTIC_INT:
					switch (op2Type) {
						case SEMANTIC_INT:
							return SEMANTIC_INT;

						case SEMANTIC_FLOAT:
							convertIntToFloat(&operator1, errCode);
							if (*errCode != ERROR_SUCCESS)
								return SEMANTIC_UNKNOWN;
							return SEMANTIC_FLOAT;

						case SEMANTIC_BOOL:
							convertBoolToInt(&operator2, errCode);
							if (*errCode != ERROR_SUCCESS)
								return SEMANTIC_UNKNOWN;
							break;

						case SEMANTIC_UNKNOWN:
							return SEMANTIC_UNKNOWN;
						case SEMANTIC_VARIABLE:
							if (!symTableIsVariableAssigned(symTable, operator2.data.token->data.strval, context))
								*errCode = ERROR_SEMANTIC_FUNCTION;
							return SEMANTIC_UNKNOWN;


						default:
							*errCode = ERROR_SEMANTIC_EXPRESSION;
							return SEMANTIC_UNKNOWN;
					}
					break;

				case SEMANTIC_FLOAT:
					switch (op2Type) {

						case SEMANTIC_INT:
							convertIntToFloat(&operator2, errCode);
							if (*errCode != ERROR_SUCCESS)
								return SEMANTIC_UNKNOWN;
							return SEMANTIC_FLOAT;

						case SEMANTIC_FLOAT:
							return SEMANTIC_FLOAT;

						case SEMANTIC_BOOL:
							convertBoolToInt(&operator2, errCode);
							if (*errCode != ERROR_SUCCESS)
								return SEMANTIC_UNKNOWN;

							convertIntToFloat(&operator2, errCode);
							if (*errCode != ERROR_SUCCESS)
								return SEMANTIC_UNKNOWN;
							return SEMANTIC_FLOAT;

						case SEMANTIC_UNKNOWN:
						case SEMANTIC_VARIABLE:
							if (!symTableIsVariableAssigned(symTable, operator2.data.token->data.strval, context))
								*errCode = ERROR_SEMANTIC_FUNCTION;
							return SEMANTIC_UNKNOWN;

						default:
							*errCode = ERROR_SEMANTIC_EXPRESSION;
							return SEMANTIC_UNKNOWN;
					}

				case SEMANTIC_BOOL:
					switch (op2Type) {
						case SEMANTIC_INT:
							*errCode = ERROR_SEMANTIC_EXPRESSION;
							return SEMANTIC_UNKNOWN;

						case SEMANTIC_STRING:
							*errCode = ERROR_SEMANTIC_EXPRESSION;
							return SEMANTIC_UNKNOWN;

						case SEMANTIC_FLOAT:
							*errCode = ERROR_SEMANTIC_EXPRESSION;
							return SEMANTIC_UNKNOWN;

						case SEMANTIC_BOOL:
							return SEMANTIC_BOOL;

						case SEMANTIC_UNKNOWN:
						case SEMANTIC_VARIABLE:
							if (!symTableIsVariableAssigned(symTable, operator2.data.token->data.strval, context))
								*errCode = ERROR_SEMANTIC_FUNCTION;
							return SEMANTIC_UNKNOWN;

						default:
							*errCode = ERROR_SEMANTIC_EXPRESSION;
							return SEMANTIC_UNKNOWN;
					}

				case SEMANTIC_VARIABLE:
					if (!symTableIsVariableAssigned(symTable, operator2.data.token->data.strval, context))
						*errCode = ERROR_SEMANTIC_FUNCTION;
					return SEMANTIC_UNKNOWN;

				case SEMANTIC_UNKNOWN:
					return SEMANTIC_UNKNOWN;

				default:
					*errCode = ERROR_SEMANTIC_EXPRESSION;
					return SEMANTIC_UNKNOWN;
			}
			break;
		}

		case E_NOT: {
			treeElement_t operator1 = expressionTree->data.elements[0];
			semanticType_t op1Type = getOperatorType(operator1, errCode);
			if(op1Type == SEMANTIC_EXPRESSION){
				op1Type = checkOperation(&operator1, symTable, errCode, context);
			}

			if (*errCode != ERROR_SUCCESS)
				return SEMANTIC_UNKNOWN;

			if(op1Type == SEMANTIC_VARIABLE) {
				return SEMANTIC_UNKNOWN;
			} else if(op1Type == SEMANTIC_UNKNOWN) {
				return SEMANTIC_UNKNOWN;
			} else if(op1Type == SEMANTIC_BOOL) {
				return SEMANTIC_BOOL;
			}

			*errCode = ERROR_SEMANTIC_EXPRESSION;
			return SEMANTIC_UNKNOWN;
		}

		case E_GT:
		case E_GTE:
		case E_LT:
		case E_LTE:
		case E_NEQ:

			break;
		case E_AND:
		case E_OR: {
			treeElement_t operator1 = expressionTree->data.elements[0];
			semanticType_t op1Type = getOperatorType(operator1, errCode);
			if(op1Type == SEMANTIC_EXPRESSION){
				op1Type = checkOperation(&operator1, symTable, errCode, context);
			}

			if(op1Type == SEMANTIC_VARIABLE) {
				return SEMANTIC_UNKNOWN;
			} else if(op1Type == SEMANTIC_UNKNOWN) {
				return SEMANTIC_UNKNOWN;
			} else if(op1Type == SEMANTIC_BOOL) {
				return SEMANTIC_BOOL;
			}

			*errCode = ERROR_SEMANTIC_EXPRESSION;
			return SEMANTIC_UNKNOWN;
		}

		default:
			*errCode = ERROR_SEMANTIC_OTHER;
			return SEMANTIC_UNKNOWN;
	}
	return SEMANTIC_UNKNOWN;
}

void convertIntToFloat(treeElement_t* element, int* errCode) {
	if(element->type == E_TOKEN){
		if(element->data.token->type == T_NUMBER) {
			tokenIntToFloat(element->data.token, errCode);
			if(*errCode != ERROR_SUCCESS){
				return;
			}
		}
	} else {
		if(element->nodeSize > 0){
			for(unsigned int i = 0; i < element->nodeSize; i++) {
				convertIntToFloat(&element->data.elements[i], errCode);
			}
		}
	}
}


void convertBoolToInt(treeElement_t* element, int* errCode) {
	if(element->type == E_TOKEN){
		if(element->data.token->type == T_BOOL_TRUE || element->data.token->type == T_BOOL_FALSE) {
			tokenBoolToInt(element->data.token, errCode);
			if(*errCode != ERROR_SUCCESS){
				return;
			}
		}
	} else {
		if(element->nodeSize > 0){
			for(unsigned int i = 0; i < element->nodeSize; i++) {
				convertBoolToInt(&element->data.elements[i], errCode);
			}
		}
	}
}
