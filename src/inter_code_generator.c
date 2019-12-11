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

#include "inter_code_generator.h"

// string names of the frames
const char* FRAME_NAME[] = {
        [FRAME_GLOBAL] = "GF",
        [FRAME_LOCAL] = "LF",
        [FRAME_TEMP] = "TF"
};

// strlen(s, i, n)  // variables are already defined in temp frame
const char* substr_def =
        "LABEL SUBSTR\n"        // function label
        "DEFVAR $length\n"
        "STRLEN $length s\n"    // get string length
        "DEFVAR $temp\n"

        "GT $temp i $length\n"  // check if i > string length
        "JUMPIFEQ $retNone $temp bool@true\n"

        "LT $temp i int@0\n"    // check if substring begining is in string
        "JUMPIFEQ $retNone $temp bool@true\n" // jump to return none if i < 0

        "LT $temp n int@0\n"    // check if n < 0
        "JUMPIFEQ $retNone $temp bool@true\n"

        "GT $temp n $length\n"  // n > length of string
        "JUMPIFNEQ $notmaxlen $temp bool@true\n"
            "MOVE n length\n"       // n = length of string (copy all)
        "LABEL $notmaxlen\n"

        "EQ $temp n int@0\n"    // check if n == 0
        "JUMPIFNEQ $notempty $temp bool@true\n"
            "MOVE LF@$retval string@\\000\n" // add NUL char to return variable
            "RETURN\n"              // return \0 (empty string)
        "$notempty\n"

        "DEFVAR $tempchar\n"
        "LABEL $for\n"
            "JUMPIFEQ $endfor n int@0\n"
            ""

            // TODO
            //  add implementation for ord

            "SUB n n int@1\n"
            "JUMP $for\n"
        "LABEL $endfor\n"

        "LABEL $retNone\n"      // return None
        "MOVE LF@$retval nil@nil\n"
        "RETURN\n"
        ;

int processCode(treeElement_t codeElement, symTable_t* symTable) {

    if(codeElement.type != E_CODE){
        return ERROR_SEMANTIC_OTHER;
    }

    //TODO
    // list dealocation!

    // string list where code is generated to
    dynStrList_t* codeStrList;
    codeStrList = dynStrListInit();

    if(codeStrList == NULL) {
        return ERROR_INTERNAL;
    }

    // name of function to determine if variable is global or local
    dynStr_t* context = NULL;

	generateEmbeddedFunctions(codeStrList);

	// Main function label
	dynStr_t *mainLabel = dynStrInitString("LABEL $$main\n");
	if (mainLabel == NULL) {
		return ERROR_INTERNAL;
	}
	if(!dynStrListPushBack(codeStrList, mainLabel)) {
		dynStrFree(mainLabel);
		return ERROR_INTERNAL;
	}

	static int cblockVarNameCounter = 0;

    for (unsigned i = 0; i < codeElement.nodeSize; i++) {
        int retval = ERROR_SUCCESS;

        bool pushToStack = true;
        dynStr_t* temp;

        // process code content
        switch (codeElement.data.elements[i].type) {
            case E_TOKEN:
                retval = processExpression(codeElement.data.elements[i], &pushToStack, symTable, context, codeStrList);
                temp = dynStrInit();
                if(numberToDynStr(temp, "DEFVAR GF@cblockVar%d\nPOP GF@cblockVar%d\n", cblockVarNameCounter)){
                    dynStrFree(temp);
                    retval = ERROR_INTERNAL;
                    break;
                }
                if(dynStrListPushBack(codeStrList, temp)) {
                    dynStrFree(temp);
                    retval = ERROR_INTERNAL;
                    break;
                }
                break;
            case E_S_FUNCTION_DEF:
                retval = processFunctionDefinition(codeElement.data.elements[i], symTable, context, codeStrList);
                break;
            case E_CODE_BLOCK:
                retval = processCodeBlock(codeElement.data.elements[i], symTable, context, codeStrList);
                break;
            default:
                return ERROR_SEMANTIC_OTHER; // failed to process the code
        }
        if(retval)
            return retval;
    }

	// Main function jump
	dynStr_t *mainJump = dynStrInitString("JUMP $$main\n");
	if (mainJump == NULL) {
		return ERROR_INTERNAL;
	}
	if(!dynStrListPushFront(codeStrList, mainJump)) {
		dynStrFree(mainJump);
		return ERROR_INTERNAL;
	}

	// IFJcode19 shebang
	dynStr_t* header = dynStrInitString(".IFJcode19\n");
	if(header == NULL) {
		return ERROR_INTERNAL;
	}
	if(!dynStrListPushFront(codeStrList, header)) {
		dynStrFree(header);
		return ERROR_INTERNAL;
	}

    //TODO
    // process code content ending

	dynStrListPrint(codeStrList);

    return ERROR_SUCCESS;
}

int processEToken(treeElement_t eTokenElement, dynStr_t* outputDynStr, bool id_only,
        symTable_t* symTable, dynStr_t* context, bool* varDefined) {

    if(eTokenElement.type != E_TOKEN) {
        return ERROR_SEMANTIC_OTHER;
    }

    if(!outputDynStr) {
        return ERROR_INTERNAL;
    }

    if(id_only && eTokenElement.data.token->type != T_ID) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval;

    switch (eTokenElement.data.token->type) {
        case T_NUMBER:
            // add type
            if(numberToDynStr(outputDynStr, "int@%ld", eTokenElement.data.token->data.intval)){
                dynStrFree(outputDynStr);
                return ERROR_INTERNAL;
            }
            break;
        case T_FLOAT:
            // add type
            if(floatToDynStr(outputDynStr, "float@%a", eTokenElement.data.token->data.floatval)){
                dynStrFree(outputDynStr);
                return ERROR_INTERNAL;
            }
            break;
        case T_STRING_ML:
        case T_STRING:
            if(!dynStrEscape(eTokenElement.data.token->data.strval)) {
                return ERROR_INTERNAL;
            }
            if(!dynStrAppendString(outputDynStr, "string@")){
                return ERROR_INTERNAL;
            }
            if(!dynStrAppendString(outputDynStr, eTokenElement.data.token->data.strval->string)) {
                return ERROR_INTERNAL;
            }
            break;
        case T_ID:
            if(!id_only) { // variable - add  FRAME_TYPE@
                // determine if variable is local or global
                symbolFrame_t idFrame = symTableGetFrame(symTable, eTokenElement.data.token->data.strval, context);
                if (!dynStrAppendString(outputDynStr, FRAME_NAME[idFrame])) {
                    return ERROR_INTERNAL;
                }
                if (!dynStrAppendString(outputDynStr, "@")) {
                    return ERROR_INTERNAL;
                }
            }
            // add name
            if(!dynStrAppendString(outputDynStr, dynStrGetString(eTokenElement.data.token->data.strval))) {
                return ERROR_INTERNAL;
            }
            if(varDefined) {
                // tells if variable is needs to be defined
                *varDefined = !symTableIsVariableAssigned(symTable, eTokenElement.data.token->data.strval, context);
                // sets value to false
                retval = symTableInsertVariable(symTable, eTokenElement.data.token->data.strval, context, false);
                if(retval) {
                    return retval;
                }
            }
            break;
		case T_BOOL_TRUE:
		    if(!dynStrAppendString(outputDynStr, "bool@true")){
			    dynStrFree(outputDynStr);
			    return ERROR_INTERNAL;
		    }
		    break;
		case T_BOOL_FALSE:
		    if(!dynStrAppendString(outputDynStr, "bool@false")){
			    dynStrFree(outputDynStr);
			    return ERROR_INTERNAL;
		    }
		    break;
		case T_KW_NONE:
			if(!dynStrAppendString(outputDynStr, "nil@nil")){
				dynStrFree(outputDynStr);
				return ERROR_INTERNAL;
			}
			break;
        default:
            return ERROR_SEMANTIC_OTHER;
    }

    return ERROR_SUCCESS;
}

int processFunctionDefinition(treeElement_t defElement,symTable_t *symTable, dynStr_t* context,
        dynStrList_t* codeStrList) {

    if(defElement.type != E_S_FUNCTION_DEF) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    // processing function name
    // (label to jump to when function is called)
    dynStr_t* temp = dynStrInitString("LABEL ");
    if(temp == NULL) {
        return ERROR_INTERNAL;
    }
    // dynamic string with function name, to determine variable context
    dynStr_t* function_name = dynStrInit();
    retval = processEToken(defElement.data.elements[0], function_name, true, symTable, context, NULL);

    if(retval) {
        dynStrFree(function_name);
        return retval;
    }
    if(!dynStrAppendString(temp, dynStrGetString(function_name))) {
        dynStrFree(function_name);
        return ERROR_INTERNAL;
    }
    if(!dynStrAppendString(temp, " \n")) {
        dynStrFree(function_name);
        return ERROR_INTERNAL;
    }

    // process function body
    retval = processCodeBlock(defElement.data.elements[2], symTable, function_name, codeStrList);

    dynStrFree(function_name);
    //TODO
    // add print return?
    // add error check

    return retval;
}

int processCodeBlock(treeElement_t codeBlockElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList) {

    if(codeBlockElement.type != E_CODE_BLOCK) {
        return ERROR_SEMANTIC_OTHER;
    }

    // BLOCK START

    for(unsigned i = 0; i < codeBlockElement.nodeSize; i++) {

        int retval = ERROR_SUCCESS;
	    bool pushToStack = false;

        switch (codeBlockElement.data.elements[i].type) {
            case E_S_EXPRESSION:
                retval = processExpression(codeBlockElement.data.elements[i], &pushToStack, symTable, context, codeStrList);
                break;
            case E_ASSIGN:
                retval = processAssign(codeBlockElement.data.elements[i], symTable, context, codeStrList);
                break;
            case E_S_IF:
                retval = processIf(codeBlockElement.data.elements[i], symTable, context, codeStrList);
                break;
            case E_S_WHILE:
                retval = processWhile(codeBlockElement.data.elements[i], symTable, context, codeStrList);
                break;
            default:
                return ERROR_SEMANTIC_OTHER;
        }
        if(retval)
            return retval;
    }

    // BLOCK END

    return ERROR_SUCCESS;
}

int processExpression(treeElement_t expElement, bool* pushToStack,
        symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList) {

    if(expElement.type != E_S_EXPRESSION) {
        return ERROR_SEMANTIC_OTHER;
    }

    //TODO
    // add pushToStack / use variables

    int retval = ERROR_SUCCESS;
    dynStr_t* temp;
	treeElement_t element = expElement.data.elements[0];

    switch (element.type) {
        case E_TOKEN:
            //TODO
            // allocation / dealocation
            temp = dynStrInit();
            if(*pushToStack) {
                if (!dynStrAppendString(temp, "PUSHS ")) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
            }
            retval = processEToken(element, temp, false, symTable, context, NULL);
            if(retval) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            // add eol after value
            if (!dynStrAppendString(temp, "\n")) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            // add move operation if not pushing
            if(!(*pushToStack)){
                // add to list
                if(!dynStrListPushBack(codeStrList, temp)) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
                // add operation
                temp = dynStrInit();
                if (!dynStrAppendString(temp, "MOVE ")) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
            }
            // add to list
            if(!dynStrListPushBack(codeStrList, temp)) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            break;
        case E_S_EXPRESSION:
            retval = processExpression(element, pushToStack, symTable, context, codeStrList);
            break;
        case E_ADD:
        case E_SUB:
        case E_MUL:
        case E_DIV:
        case E_DIV_INT:
        case E_AND:
        case E_OR:
        case E_EQ:
        case E_GT:
        case E_LT:
            retval = processBinaryOperation(element, pushToStack, symTable, context, codeStrList);
            break;
        case E_NOT:
			retval = processUnaryOperation(element, pushToStack, symTable, context, codeStrList);
            break;
        case E_S_FUNCTION_CALL:
            retval = processFunctionCall(element, symTable, context, codeStrList);
            break;
        case E_ASSIGN:
            retval = processAssign(element, symTable, context, codeStrList);
            break;
        default:
            return ERROR_SEMANTIC_OTHER;
    }

    return retval;
}

int processBinaryOperation(treeElement_t operationElement, bool* pushToStack, symTable_t* symTable, dynStr_t* context,
        dynStrList_t* codeStrList) {

    if (operationElement.nodeSize != 2) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    if (operationElement.data.elements[0].type != E_TOKEN ||
        operationElement.data.elements[1].type != E_TOKEN) {
        *pushToStack = true;
    }

    dynStr_t *temp[3] = {NULL, NULL, NULL};

    // extract data in reverse order (for pushing them to stack)
    for (int i = 1; i >= 0; i--) {
        switch (operationElement.data.elements[i].type) {
            case E_TOKEN:
                temp[i] = dynStrInit();
                if (*pushToStack) {
                    if (!dynStrAppendString(temp[i], "PUSHS ")) {
                        retval = ERROR_INTERNAL;
                        break;
                    }
                }
                retval = processEToken(operationElement.data.elements[i], temp[i], false, symTable, context, NULL);
                if (retval) {
                    break;
                }
                if (*pushToStack) {
                    if (!dynStrAppendString(temp[i], "\n")) {
                        retval = ERROR_INTERNAL;
                        break;
                    }
                    if (!dynStrListPushBack(codeStrList, temp[i])) {
                        retval = ERROR_INTERNAL;
                        break;
                    }
                }
                break;
            case E_S_EXPRESSION:
                retval = processExpression(operationElement.data.elements[i], pushToStack, symTable, context, codeStrList);
                break;
            case E_ADD:
            case E_SUB:
            case E_MUL:
            case E_DIV:
            case E_DIV_INT:
            case E_AND:
            case E_OR:
            case E_EQ:
            case E_GT:
            case E_LT:
                retval = processBinaryOperation(operationElement.data.elements[i], pushToStack, symTable, context, codeStrList);
                break;
            case E_NOT:
                retval = processUnaryOperation(operationElement.data.elements[i], pushToStack, symTable, context, codeStrList);
                break;
            case E_S_FUNCTION_CALL:
                retval = processFunctionCall(operationElement.data.elements[i], symTable, context, codeStrList);
                break;
            case E_ASSIGN:
                retval = processAssign(operationElement.data.elements[i], symTable, context, codeStrList);
                break;
            default:
                return ERROR_SEMANTIC_OTHER;
        }
        if (retval) {
            // free dynamic strings if they are not appended to the codeStrList
            if (operationElement.data.elements[i].type == E_TOKEN) {
                dynStrFree(temp[i]);
            }
            if(!(*pushToStack)) {
                if (operationElement.data.elements[(i + 1) % 2].type == E_TOKEN) {
                    dynStrFree(temp[(i + 1) % 2]);
                }
            }
            return retval;
        }
    }


    temp[2] = dynStrInit(); // init last string
    // determine operation
    // if value is not pushed, var name is added to operation in calling function
    // and strings are concatenated
    switch (operationElement.type) {
        case E_ADD:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[2], "ADDS\n");
            else
                retval = !dynStrAppendString(temp[2], "ADD ");
            break;
        case E_SUB:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[2], "SUBS\n");
            else
                retval = !dynStrAppendString(temp[2], "SUB ");
            break;
        case E_MUL:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[2], "MULS\n");
            else
                retval = !dynStrAppendString(temp[2], "MUL ");
            break;
        case E_DIV:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[2], "DIVS\n");
            else
                retval = !dynStrAppendString(temp[2], "DIV ");
            break;
        case E_DIV_INT:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[2], "IDIVS\n");
            else
                retval = !dynStrAppendString(temp[2], "IDIV ");
            break;
        case E_AND:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[2], "ANDS\n");
            else
                retval = !dynStrAppendString(temp[2], "AND ");
            break;
        case E_OR:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[2], "ORS\n");
            else
                retval = !dynStrAppendString(temp[2], "OR ");
            break;
        case E_EQ:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[2], "EQS\n");
            else
                retval = !dynStrAppendString(temp[2], "EQ ");
            break;
        case E_LT:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[2], "LTS\n");
            else
                retval = !dynStrAppendString(temp[2], "LT ");
            break;
        case E_GT:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[2], "GTS\n");
            else
                retval = !dynStrAppendString(temp[2], "GT ");
            break;
        default:
            return ERROR_SEMANTIC_OTHER;
    }
    // check for error
    if (retval) {
        dynStrFree(temp[2]);
        if (!(*pushToStack)) {
            dynStrFree(temp[1]);
            dynStrFree(temp[0]);
        }
        return ERROR_INTERNAL;
    }
    if(!(*pushToStack)) {
        // concatenate strings, add space and eol
        // output is like "str1 str2\n"
        if(!dynStrAppendString(temp[0], " ")
        || !dynStrAppendString(temp[0], dynStrGetString(temp[1]))
        || !dynStrAppendString(temp[0], "\n")) {
            dynStrFree(temp[0]);
            dynStrFree(temp[1]);
            dynStrFree(temp[2]);
            return ERROR_INTERNAL;
        }
        // free second string after concatenation
        dynStrFree(temp[1]);

        if(!dynStrListPushBack(codeStrList, temp[0])) {
            dynStrFree(temp[0]);
            dynStrFree(temp[2]);
            return ERROR_INTERNAL;
        }
    }

    // add to list
    if (!dynStrListPushBack(codeStrList, temp[2])) {
        dynStrFree(temp[2]);
        if (!(*pushToStack)) {
            dynStrFree(temp[1]);
            dynStrFree(temp[0]);
        }
        return ERROR_INTERNAL;
    }

    return ERROR_SUCCESS;
}

int processUnaryOperation(treeElement_t operationElement, bool* pushToStack, symTable_t* symTable,
        dynStr_t* context, dynStrList_t* codeStrList){

    if(operationElement.nodeSize != 1) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    dynStr_t* temp[2] = { NULL, NULL };

    switch (operationElement.data.elements[0].type) {
        case E_TOKEN:
            temp[0] = dynStrInit();
            if(*pushToStack) {
                if (!dynStrAppendString(temp[0], "PUSHS ")) {
                    dynStrFree(temp[0]);
                    retval = ERROR_INTERNAL;
                    break;
                }
            }
            retval = processEToken(operationElement.data.elements[0], temp[0], false, symTable, context, NULL);
            if(retval)
                break;
            if(*pushToStack) {
                if (!dynStrAppendString(temp[0], "\n")) {
                    retval = ERROR_INTERNAL;
                }
            }
            break;
        case E_S_EXPRESSION:
            *pushToStack = true;
            retval = processExpression(operationElement.data.elements[0], pushToStack, symTable, context, codeStrList);
            break;
        case E_ADD:
        case E_SUB:
        case E_MUL:
        case E_DIV:
        case E_DIV_INT:
        case E_AND:
        case E_OR:
        case E_EQ:
        case E_GT:
        case E_LT:
            *pushToStack = true;
            retval = processBinaryOperation(operationElement.data.elements[0], pushToStack, symTable, context, codeStrList);
            break;
        case E_NOT:
            *pushToStack = true;
            retval = processUnaryOperation(operationElement.data.elements[0], pushToStack, symTable, context, codeStrList);
            break;
        case E_S_FUNCTION_CALL:
            *pushToStack = true;
            retval = processFunctionCall(operationElement.data.elements[0], symTable, context, codeStrList);
            break;
        case E_ASSIGN:
            *pushToStack = true;
            retval = processAssign(operationElement.data.elements[0], symTable, context, codeStrList);
            break;
        default:
            return ERROR_SEMANTIC_OTHER;
    }
    if(retval) {
        dynStrFree(temp[0]);
        return retval;
    }

    temp[1] = dynStrInit();

    switch (operationElement.type){
        case E_NOT:
            if(*pushToStack)
                retval = !dynStrAppendString(temp[1], "NOTS\n");
            else
                retval = !dynStrAppendString(temp[1], "NOT ");
            break;
        default:
            retval = ERROR_SEMANTIC_OTHER;
    }

    // check for errors
    if(retval) {
        dynStrFree(temp[1]);
        dynStrFree(temp[0]);
        return retval;
    }

    // push value if any
    if(!(*pushToStack)) {
        if (!dynStrListPushBack(codeStrList, temp[0])) {
            dynStrFree(temp[0]);
            dynStrFree(temp[1]);
            return ERROR_INTERNAL;
        }
    }
    // push operation
    if(!dynStrListPushBack(codeStrList, temp[1])) {
        dynStrFree(temp[0]);
        dynStrFree(temp[1]);
        return ERROR_INTERNAL;
    }

    return ERROR_SUCCESS;
}

int processIf(treeElement_t ifElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList) {
    if(ifElement.type != E_S_IF) {
        return ERROR_SEMANTIC_OTHER;
    }

    // used to make labels unique
    static unsigned ifCounter = 0;

    bool pushToStack = true;

    int retval = ERROR_SUCCESS; // return value

    // expression
    retval = processExpression(ifElement.data.elements[0], &pushToStack, symTable, context, codeStrList);
    if(retval){
        return retval;
    }

    dynStr_t* temp = dynStrInit();
    if(!temp) {
        return ERROR_INTERNAL;
    }

    //TODO
    // add single line comparison
    retval = numberToDynStr(temp, "PUSHS bool@true\nJUMPIFEQS $if%d\n", ifCounter);

    if(retval){
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to code list
    retval = !dynStrListPushBack(codeStrList, temp);
    if(retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // fall through to else

    // else body
    if(ifElement.nodeSize > 2) {
        retval = processElse(ifElement.data.elements[2], symTable, context, codeStrList);
        if(retval){
            return retval;
        }
    }

    temp = dynStrInit();
    retval = numberToDynStr(temp, "JUMP $fi%d\n", ifCounter);

    if(retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to code list
    retval = !dynStrListPushBack(codeStrList, temp);
    if(retval){
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // next string have same size

    // add jump to fi
    temp = dynStrInit();
    retval = numberToDynStr(temp, "JUMP $fi%d\n", ifCounter);

    if(retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to code list
    retval = !dynStrListPushBack(codeStrList, temp);
    if(retval){
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // add if label
    temp = dynStrInit();
    retval = numberToDynStr(temp, "LABEL $if%d\n", ifCounter);

    if(retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to code list
    retval = !dynStrListPushBack(codeStrList, temp);
    if(retval){
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // if body
    retval = processCodeBlock(ifElement.data.elements[1], symTable, context, codeStrList);
    if(retval) {
        return retval;
    }

    // add fi (end of if-else)
    temp = dynStrInit();
    retval = numberToDynStr(temp, "LABEL $fi%d\n", ifCounter);

    if(retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to code list
    retval = !dynStrListPushBack(codeStrList, temp);
    if(retval){
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    ifCounter++;

    return ERROR_SUCCESS;
}

int numberToDynStr(dynStr_t* outputStr, char* formatString, long number) {
    int retval; // return value

    // get size of string
    unsigned strSize = snprintf(NULL, 0, formatString, number);
    char* buffer = malloc(++strSize); // + \0
    // check allocation
    if(!buffer) {
        return ERROR_INTERNAL;
    }

    // print string to buffer
    snprintf(buffer, strSize, formatString, number);

    // append buffer to dynamic string
    retval = !dynStrAppendString(outputStr, buffer);
    free(buffer);
    if(retval) {
        return ERROR_INTERNAL;
    }

    return retval; // ERROR_SUCCESS == 0, ERROR_INTERNAL == 99
}

int floatToDynStr(dynStr_t* outputStr, char* formatString, double number) {
    int retval; // return value

    // get size of string
    unsigned strSize = snprintf(NULL, 0, formatString, number);
    char* buffer = malloc(++strSize); // + \0
    // check allocation
    if(!buffer) {
        return ERROR_INTERNAL;
    }

    // print string to buffer
    snprintf(buffer, strSize, formatString, number);

    // append buffer to dynamic string
    retval = !dynStrAppendString(outputStr, buffer);
    free(buffer);
    if(retval) {
        return ERROR_INTERNAL;
    }

    return retval; // ERROR_SUCCESS == 0, ERROR_INTERNAL == 99
}

int processElse(treeElement_t elseElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList) {
    if(elseElement.type != E_S_ELSE) {
        return ERROR_SEMANTIC_OTHER;
    }

    if(elseElement.nodeSize != 1) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    retval = processCodeBlock(elseElement.data.elements[0], symTable, context, codeStrList);

    return retval;
}

int processAssign(treeElement_t assignElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList) {
    if(assignElement.type != E_ASSIGN) {
        return  ERROR_SEMANTIC_OTHER;
    }

    if(assignElement.nodeSize != 2) {
        return ERROR_SEMANTIC_OTHER;
    }

    // left side of assignment must be id (variable)
    if(assignElement.data.elements[0].type != E_TOKEN) {
        return ERROR_SEMANTIC_OTHER;
    }

    dynStr_t* temp; // temporary stores the line of code

    dynStr_t* varName; // temporary stores variable name

    dynStr_t* varDef; // temporary stores variable definition

    int retval = ERROR_SUCCESS; // return code

    bool varDefined = true;

    bool pushToStack = false;

    switch (assignElement.data.elements[1].type) {
        case E_S_EXPRESSION: // func call/expression ( l = f() | l = a + b)
            retval = processExpression(assignElement.data.elements[1], &pushToStack, symTable, context, codeStrList);
            if(retval) {
                return retval;
            }
            temp = dynStrInit();
            if(pushToStack) { // pop from stack
                if (!dynStrAppendString(temp, "POPS ")) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
            } else { // add binary operation from expression processing (last element of stack)
                dynStr_t* lastStr = dynStrListElGet(dynStrListBack(codeStrList));
                if(!dynStrAppendString(temp, dynStrGetString(lastStr))) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
                // remove operation from list
                dynStrListPopBack(codeStrList);
            }
            // get variable id
            varName = dynStrInit();
            retval = processEToken(assignElement.data.elements[0], varName, false, symTable, context, &varDefined);
            if(retval) {
                dynStrFree(temp);
                return retval;
            }
            if(dynStrAppendString(temp, dynStrGetString(varName))) {
                dynStrFree(temp);
                dynStrFree(varName);
                return ERROR_INTERNAL;
            }
            if(pushToStack) {
                // add eol to pop
                if (!dynStrAppendString(temp, "\n")) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    return ERROR_INTERNAL;
                }
            } else {
                if(!dynStrAppendString(temp, " ")) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    return ERROR_INTERNAL;
                }
                // add operands to binary operation
                dynStr_t* lastStr = dynStrListElGet(dynStrListBack(codeStrList));
                if(!dynStrAppendString(temp, dynStrGetString(lastStr))) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    return ERROR_INTERNAL;
                }
                // remove operands from list
                dynStrListPopBack(codeStrList);
            }
            // add definition
            if(!varDefined) {
                varDef = dynStrInit();
                if(!dynStrAppendString(varDef, "DEFVAR ")) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    dynStrFree(varDef);
                    return ERROR_INTERNAL;
                }
                // append var name
                if(!dynStrAppendString(varDef, dynStrGetString(varName))) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    dynStrFree(varDef);
                    return ERROR_INTERNAL;
                }
                // append eol
                if(!dynStrAppendString(varDef, "\n")) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    dynStrFree(varDef);
                    return ERROR_INTERNAL;
                }
                // add to list
                if(!dynStrListPushBack(codeStrList, temp)) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    dynStrFree(varDef);
                    return ERROR_INTERNAL;
                }
            }
            // free temp variable with name
            dynStrFree(varName);
            break;
        case E_TOKEN: // value or id ( l = r )
            temp = dynStrInit();
            if(!dynStrAppendString(temp, "MOVE ")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }

            // process left side
            varName = dynStrInit();
            retval = processEToken(assignElement.data.elements[0], varName, false, symTable, context, &varDefined);
            if(retval) {
                dynStrFree(temp);
                return retval;
            }
            if(dynStrAppendString(temp, dynStrGetString(varName))) {
                dynStrFree(temp);
                dynStrFree(varName);
                return ERROR_INTERNAL;
            }
            if(!dynStrAppendString(temp, " ")) {
                dynStrFree(temp);
                dynStrFree(varName);
                return ERROR_INTERNAL;
            }
            // process right side
            retval = processEToken(assignElement.data.elements[1], temp, false, symTable, context, NULL);
            if(retval) {
                dynStrFree(temp);
                dynStrFree(varName);
                return retval;
            }
            // add eol
            if(!dynStrAppendString(temp, "\n")){
                dynStrFree(temp);
                dynStrFree(varName);
                return ERROR_INTERNAL;
            }
            // add definition
            if(!varDefined) {
                varDef = dynStrInit();
                if(!dynStrAppendString(varDef, "DEFVAR ")) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    dynStrFree(varDef);
                    return ERROR_INTERNAL;
                }
                // append var name
                if(!dynStrAppendString(varDef, dynStrGetString(varName))) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    dynStrFree(varDef);
                    return ERROR_INTERNAL;
                }
                // append eol
                if(!dynStrAppendString(varDef, "\n")) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    dynStrFree(varDef);
                    return ERROR_INTERNAL;
                }
                // add to list
                if(!dynStrListPushBack(codeStrList, temp)) {
                    dynStrFree(temp);
                    dynStrFree(varName);
                    dynStrFree(varDef);
                    return ERROR_INTERNAL;
                }
            }
            // free name
            dynStrFree(varName);
            break;
        default:
            return ERROR_SEMANTIC_OTHER;
    }
    // add temp to list
    if(!dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    return retval;
}

int processFunctionCall(treeElement_t callElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList) {
    if(callElement.type != E_S_FUNCTION_CALL) {
        return ERROR_SEMANTIC_OTHER;
    }

    if(!(callElement.nodeSize == 1 || callElement.nodeSize == 2)) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval;

    // function name
    dynStr_t* fname = dynStrInit();
    retval = processEToken(callElement.data.elements[0], fname, true, symTable, context, NULL);
    if(retval) {
        dynStrFree(fname);
        return retval;
    }
	dynStr_t* temp = dynStrInit();

    // add create frame
    if(!dynStrAppendString(temp, "CREATEFRAME\n")) {
        dynStrFree(fname);
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to list
    if(!dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(fname);
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // TODO
    //  remove temporary frame and hardcode it in processFunctionCallParams?

	if (dynStrEqualString(fname, "print")) {
		long argc = callElement.nodeSize == 1 ? 0 : callElement.data.elements[1].nodeSize;
		dynStrList_t *printArgs = dynStrListInit();
		dynStr_t *string = dynStrInitString("PUSHS ");
		int retVal = numberToDynStr(string, "int@%ld", argc);
		if (retVal) {
			return retVal;
		}
		if (!dynStrListPushBack(printArgs, string)) {
			dynStrListFree(printArgs);
			dynStrFree(string);
			return ERROR_INTERNAL;
		}
		bool pushToStack = true;
		for (long i = 0; i < argc; ++i) {
			retVal = processExpression(callElement.data.elements[1].data.elements[i], &pushToStack, symTable, context, printArgs);
			if (retVal) {
				dynStrListFree(printArgs);
				return retval;
			}
		}
		for (long i = argc; i >= 0; --i) {
			dynStrListEl_t *element = dynStrListBack(printArgs);
			dynStrListPushBack(codeStrList, dynStrClone(dynStrListElGet(element)));
			dynStrListPopBack(printArgs);
		}
		dynStrListFree(printArgs);
		string = dynStrInitString("\n");
		if (!dynStrListPushBack(codeStrList, string)) {
			dynStrFree(string);
			return ERROR_INTERNAL;
		}
	} else if (callElement.nodeSize == 2) { // create frame and process params
	    retval = processFunctionCallParams(callElement.data.elements[1], symTable, fname, codeStrList);
	    if (retval) {
		    return retval;
	    }
    }

    // add pushframe
    temp = dynStrInitString("PUSHFRAME\n");
    if(temp == NULL) {
        dynStrFree(fname);
        return ERROR_INTERNAL;
    }
    // add to list
    if(!dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(fname);
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // call the function
    temp = dynStrInitString("CALL ");
    if(temp == NULL) {
        dynStrFree(fname);
        return ERROR_INTERNAL;
    }
    // function name
    if(!dynStrAppendString(temp, dynStrGetString(fname))){
        dynStrFree(fname);
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    dynStrFree(fname);
    if(!dynStrAppendString(temp, "\n")) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    if(!dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // pop frame
    temp = dynStrInitString("POPFRAME\n");
    if(temp == NULL) {
        return ERROR_INTERNAL;
    }
    if(!dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }


    //TODO
    // this will be in expression processing:
    // printf("PUSH %s@$retval\n", FRAME_NAME[current_frame]);

    return ERROR_SUCCESS;
}

int processFunctionCallParams(treeElement_t callParamsElement, symTable_t* symTable,
        dynStr_t* context, dynStrList_t* codeStrList) {
    if(callParamsElement.type != E_S_FUNCTION_CALL_PARAMS) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    dynStr_t* temp = NULL;
    dynStr_t* argName = NULL;
    // process arg[i]
    for(unsigned i = 0; i < callParamsElement.nodeSize; i++) {
        bool pushToStack = false;
        if (dynStrEqualString(context, "len")) {
        	pushToStack = true;
        }

        //TODO
        // process params and create assignment after loop (because of expressions and function calls)

        // get expression data
        retval = processExpression(callParamsElement.data.elements[i], &pushToStack, symTable, context, codeStrList);
        if(retval){
            return retval;
        }
        // get argument variable name
        argName = symTableGetArgumentName(symTable, context, i);
        // check if name exists
        if(!argName) {
            return ERROR_SEMANTIC_OTHER;
        }
        // create data assignment
        temp = dynStrInit();
        // value is on stack
        if(pushToStack) {
            if(!dynStrAppendString(temp, "POPS ")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(!dynStrAppendString(temp, "TF@")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(!dynStrAppendString(temp, dynStrGetString(argName))) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(!dynStrAppendString(temp, "\n")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
        }
        // value is assigned directly to variable
        else {
            // get operation from list
            dynStr_t* lastStr = dynStrListElGet(dynStrListBack(codeStrList));
            if(!dynStrAppendString(temp, dynStrGetString(lastStr))) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            // remove operation from list
            dynStrListPopBack(codeStrList);
            if(!dynStrAppendString(temp, "TF@")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(!dynStrAppendString(temp, dynStrGetString(argName))) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(!dynStrAppendString(temp, " ")) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            // add operands
            lastStr = dynStrListElGet(dynStrListBack(codeStrList));
            if(!dynStrAppendString(temp, dynStrGetString(lastStr))) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            // remove operands from list
            dynStrListPopBack(codeStrList);
        }
        // add arg to list
        if(!dynStrListPushBack(codeStrList, temp)) {
            dynStrFree(temp);
            return ERROR_INTERNAL;
        }

    } // for

    return ERROR_SUCCESS;
}

int generateEmbeddedFunctions(dynStrList_t *codeStrList) {
	int retVal = ERROR_SUCCESS;
	if ((retVal = generateLenFunction(codeStrList)) != ERROR_SUCCESS) {
		return retVal;
	}
	if ((retVal = generateInputiFunction(codeStrList)) != ERROR_SUCCESS) {
		return retVal;
	}
	if ((retVal = generateInputfFunction(codeStrList)) != ERROR_SUCCESS) {
		return retVal;
	}
	if ((retVal = generateInputsFunction(codeStrList)) != ERROR_SUCCESS) {
		return retVal;
	}
	if ((retVal = generatePrintFunction(codeStrList)) != ERROR_SUCCESS) {
		return retVal;
	}
	return retVal;
}

int generateLenFunction(dynStrList_t* codeStrList) {
	dynStr_t *string = dynStrInit();
	const char* code = "LABEL len\n"
					"DEFVAR LF@%retval\n"
					"STRLEN LF@%retval LF@%0\n"
					"RETURN\n";
	if(!dynStrAppendString(string, code)){
		dynStrFree(string);
		return ERROR_INTERNAL;
	}
	if(!dynStrListPushFront(codeStrList, string)) {
		dynStrFree(string);
		return ERROR_INTERNAL;
	}
	return ERROR_SUCCESS;

}

int generatePrintFunction(dynStrList_t* codeStrList) {
	dynStr_t *string = dynStrInit();
	const char* code = "LABEL print\n"
						"DEFVAR LF@%retval\n"
						"MOVE LF@%retval nil@nil\n"
						"DEFVAR LF@argc\n"
						"POPS LF@argc\n"
						"DEFVAR LF@lastArg\n"
						"DEFVAR LF@counter\n"
						"MOVE LF@counter int@0\n"
						"DEFVAR LF@string\n"
						"MOVE LF@string string@0\n"
						"DEFVAR LF@type\n"
						"LABEL $while\n"
						"JUMPIFEQ $end LF@counter LF@argc\n"
						"ADD LF@counter LF@counter int@1\n"
						"POPS LF@string\n"
						"TYPE LF@type LF@string\n"
						"JUMPIFEQ $none LF@type string@nil\n"
						"WRITE LF@string\n"
						"JUMP $endNone\n"
						"LABEL $none\n"
						"WRITE string@None\n"
						"LABEL $endNone\n"
						"LT LF@lastArg LF@counter LF@argc\n"
						"JUMPIFEQ $printNl LF@lastArg bool@false\n"
						"WRITE string@\\032\n"
						"JUMP $while\n"
						"LABEL $printNl\n"
						"WRITE string@\\010\n"
						"LABEL $end\n"
						"RETURN\n";
	if(!dynStrAppendString(string, code)) {
		dynStrFree(string);
		return ERROR_INTERNAL;
	}
	if(!dynStrListPushFront(codeStrList, string)) {
		dynStrFree(string);
		return ERROR_INTERNAL;
	}
	return ERROR_SUCCESS;
}

int processWhile(treeElement_t whileElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList) {
    if (whileElement.type != E_S_WHILE) {
        return ERROR_SEMANTIC_OTHER;
    }

    if (whileElement.nodeSize != 2) {
        return ERROR_SEMANTIC_OTHER;
    }

    // used to make labels unique
    static unsigned whileCounter = 0;

    bool pushToStack = true;

    int retval = ERROR_SUCCESS;

    retval = processExpression(whileElement.data.elements[0], &pushToStack, symTable, context, codeStrList);
    if (retval) {
        return retval;
    }

    dynStr_t *temp = dynStrInit();
    if (!temp) {
        return ERROR_INTERNAL;
    }

    retval = numberToDynStr(temp, "LABEL $while%d\n", whileCounter);
    if (retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to code list
    retval = !dynStrListPushBack(codeStrList, temp);
    if (retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    //TODO
    // add type detection

    temp = dynStrInit();
    retval = numberToDynStr(temp, "PUSHS bool@true\nJUMPIFNEQS $endWhile%d\n", whileCounter);
    if (retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to cede list
    retval = !dynStrListPushBack(codeStrList, temp);
    if (retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // Process While body
    retval = processCodeBlock(whileElement.data.elements[1], symTable, context, codeStrList);
    if (retval) {
        return retval;
    }

    // add jump to beginning
    temp = dynStrInit();
    retval = numberToDynStr(temp, "JUMP $while%d\n", whileCounter);
    if (retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to code block
    retval = !dynStrListPushBack(codeStrList, temp);
    if (retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // add end
    temp = dynStrInit();
    retval = numberToDynStr(temp, "LABEL $endWhile%d\n", whileCounter);
    if (retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    retval = !dynStrListPushBack(codeStrList, temp);
    if (retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    return ERROR_SUCCESS;
}

int generateInputiFunction(dynStrList_t* codeStrList) {
	const char* code = "LABEL inputi\n"
						"DEFVAR LF@%retval\n"
						"READ LF@%retval int\n"
						"RETURN\n";
	dynStr_t *string = dynStrInitString(code);
	if(string == NULL) {
		return ERROR_INTERNAL;
	}
	if(!dynStrListPushFront(codeStrList, string)) {
		dynStrFree(string);
		return ERROR_INTERNAL;
	}
	return ERROR_SUCCESS;
}

int generateInputfFunction(dynStrList_t* codeStrList) {
	const char* code = "LABEL inputf\n"
						"DEFVAR LF@%retval\n"
						"READ LF@%retval float\n"
						"RETURN\n";
	dynStr_t *string = dynStrInitString(code);
	if(string == NULL) {
		return ERROR_INTERNAL;
	}
	if(!dynStrListPushFront(codeStrList, string)) {
		dynStrFree(string);
		return ERROR_INTERNAL;
	}
	return ERROR_SUCCESS;
}

int generateInputsFunction(dynStrList_t* codeStrList) {
	const char* code = "LABEL inputs\n"
						"DEFVAR LF@%retval\n"
						"READ LF@%retval int\n"
						"RETURN\n";
	dynStr_t *string = dynStrInitString(code);
	if(string == NULL) {
		return ERROR_INTERNAL;
	}
	if(!dynStrListPushFront(codeStrList, string)) {
		dynStrFree(string);
		return ERROR_INTERNAL;
	}
	return ERROR_SUCCESS;
}
