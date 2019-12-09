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

// current frame
symbolFrame_t current_frame = FRAME_GLOBAL;

// current context
dynStr_t* current_context = NULL;

// symbol table
symTable_t* table;

// string list where code is generated to
dynStrList_t* codeStrList;

int processCode(treeElement_t codeElement) {

    if(codeElement.type != E_CODE){
        return ERROR_SEMANTIC_OTHER;
    }

    //TODO
    // list dealocation!

    // dynamic list allocation
    codeStrList = dynStrListInit();

    if(codeStrList == NULL) {
        return ERROR_SEMANTIC_OTHER;
    }

    // IFJcode19 shebang
    dynStr_t* header = dynStrInit();
    if(header == NULL) {
        return ERROR_INTERNAL;
    }
    if(dynStrAppendString(header , ".IFJcode19\n")) {
        dynStrFree(header);
        return ERROR_INTERNAL;
    }
    if(dynStrListPushBack(codeStrList, header)) {
        dynStrFree(header);
        return ERROR_INTERNAL;
    }

    //TODO
    // jump na main funkci

    for (unsigned i = 0; i < codeElement.nodeSize; i++) {
        int retval = ERROR_SUCCESS;
        dynStr_t* temp;

        // process code content
        switch (codeElement.data.elements[i].type) {
            case E_TOKEN:
                temp = dynStrInit();
                retval = processEToken(codeElement.data.elements[i], temp, false);
                if(retval) {
                    dynStrFree(temp);
                    return retval;
                }
                if(dynStrListPushBack(codeStrList, temp)) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
                break;
            case E_S_FUNCTION_DEF:
                retval = processFunctionDefinition(codeElement.data.elements[i]);
                break;
            case E_CODE_BLOCK:
                retval = processCodeBlock(codeElement.data.elements[i], NULL);
                break;
            default:
                return ERROR_SEMANTIC_OTHER; // failed to process the code
        }
        if(retval)
            return retval;
    }

    //TODO
    // process code content ending

    return ERROR_SUCCESS;
}

int processEToken(treeElement_t eTokenElement, dynStr_t* outputDynStr, bool id_only) {

    if(eTokenElement.type == E_TOKEN) {
        return ERROR_SEMANTIC_OTHER;
    }

    if(!outputDynStr) {
        return ERROR_INTERNAL;
    }

    if(id_only && eTokenElement.data.token->type != T_ID) {
        return ERROR_SEMANTIC_OTHER;
    }

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
            if(dynStrAppendString(outputDynStr, "string@")){
                return ERROR_INTERNAL;
            }
            if(dynStrAppendString(outputDynStr, eTokenElement.data.token->data.strval->string)) {
                return ERROR_INTERNAL;
            }
            break;
        case T_ID:
            if(!id_only) { // variable - add  FRAME_TYPE@
                // determine if variable is local or global
                symbolFrame_t idFrame = symTableGetFrame(table, eTokenElement.data.token->data.strval, current_context);
                if (dynStrAppendString(outputDynStr, FRAME_NAME[idFrame])) {
                    return ERROR_INTERNAL;
                }
                if (dynStrAppendString(outputDynStr, "@")) {
                    return ERROR_INTERNAL;
                }
            }
            // add name
            if(dynStrAppendString(outputDynStr, dynStrGetString(eTokenElement.data.token->data.strval))) {
                return ERROR_INTERNAL;
            }
            break;
        default:
            return ERROR_SEMANTIC_OTHER;
    }

    return ERROR_SUCCESS;
}

int processFunctionDefinition(treeElement_t defElement) {

    if(defElement.type != E_S_FUNCTION_DEF) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    // processing function name
    // (label to jump to when function is called)
    dynStr_t* temp = dynStrInit();
    if(temp == NULL) {
        return ERROR_INTERNAL;
    }
    if(dynStrAppendString(temp, "LABEL ")) {
        return ERROR_INTERNAL;
    }
    // dynamic string with function name, to determine variable context
    dynStr_t* function_context = NULL;
    retval = processEToken(defElement.data.elements[0], function_context, true);

    if(retval) {
        dynStrFree(function_context);
        return retval;
    }
    if(dynStrAppendString(temp, dynStrGetString(function_context))) {
        dynStrFree(function_context);
        return ERROR_INTERNAL;
    }
    if(dynStrAppendString(temp, " \n")) {
        dynStrFree(function_context);
        return ERROR_INTERNAL;
    }

    // process function body
    retval = processCodeBlock(defElement.data.elements[2], function_context);

    dynStrFree(function_context);
    //TODO
    // add print return?
    // add error check

    return retval;
}

int processCodeBlock(treeElement_t codeBlockElement, dynStr_t* context) {

    if(codeBlockElement.type != E_CODE_BLOCK) {
        return ERROR_SEMANTIC_OTHER;
    }

    // BLOCK START
    
    // save state
    symbolFrame_t temp_frame = current_frame;
    dynStr_t* temp_context = current_context;

    // if local frame needs to be created
    if(context) {
        current_frame = FRAME_LOCAL;
        current_context = context;
    }

    for(unsigned i = 0; i < codeBlockElement.nodeSize; i++) {

        int retval = ERROR_SUCCESS;

        switch (codeBlockElement.data.elements[i].type) {
            case E_S_EXPRESSION:
                retval = processExpression(codeBlockElement.data.elements[i], false);
                break;
            case E_S_IF:
                retval = processIf(codeBlockElement.data.elements[i]);
                break;
            case E_S_WHILE:
                 //retval = processWhile(codeBlockElement.data.elements[i]);
                break;
            default:
                return ERROR_SEMANTIC_OTHER;
        }
        if(retval)
            return retval;
    }

    // restore state
    if(context) {
        current_context = temp_context;
        current_frame = temp_frame;
    }

    // BLOCK END

    return ERROR_SUCCESS;
}

int processExpression(treeElement_t expElement, bool* pushToStack) {

    if(expElement.type != E_S_EXPRESSION) {
        return ERROR_SEMANTIC_OTHER;
    }

    //TODO
    // add pushToStack / use variables

    int retval = ERROR_SUCCESS;
    dynStr_t* temp;

    switch (expElement.data.elements[0].type) {
        case E_TOKEN:
            //TODO
            // allocation / dealocation
            temp = dynStrInit();
            if(pushToStack) {
                if (dynStrAppendString(temp, "PUSHS ")) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
            }
            retval = processEToken(expElement.data.elements[0], temp, false);
            if(retval) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(pushToStack) {
                // add eol after pushs varname
                if (dynStrAppendString(temp, "\n")) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
            } else {
                // add space after varname
                if (dynStrAppendString(temp, " ")) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
            }
            // add to list
            if(dynStrListPushBack(codeStrList, temp)) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            break;
        case E_S_EXPRESSION:
            retval = processExpression(expElement.data.elements[0], pushToStack);
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
            retval = processBinaryOperation(expElement.data.elements[0], pushToStack);
            break;
        case E_NOT:
			retval = processUnaryOperation(expElement.data.elements[0], pushToStack);
            break;
        case E_S_FUNCTION_CALL:
            //retval = processFunctionCall(expElement.data.elements[0]);
            break;
        case E_ASSIGN:
            retval = processAssign(expElement.data.elements[0]);
            break;
        default:
            return ERROR_SEMANTIC_OTHER;
    }

    return retval;
}

int processBinaryOperation(treeElement_t operationElement, bool* pushToStack) {

    if (operationElement.nodeSize != 2) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    if (operationElement.data.elements[0].type == E_S_EXPRESSION ||
        operationElement.data.elements[1].type == E_S_EXPRESSION) {
        *pushToStack = true;
    }

    dynStr_t *temp[3] = {NULL, NULL, NULL};

    // extract data in reverse order (for pushing them to stack)
    for (unsigned i = 2; i > 0; i--) {
        switch (operationElement.data.elements[i].type) {
            case E_TOKEN:
                temp[i] = dynStrInit();
                if (*pushToStack) {
                    if (dynStrAppendString(temp[i], "PUSHS ")) {
                        retval = ERROR_INTERNAL;
                        break;
                    }
                }
                retval = processEToken(operationElement.data.elements[i], temp[i], pushToStack);
                if (retval) {
                    break;
                }
                if (*pushToStack) {
                    if (dynStrAppendString(temp[i], "\n")) {
                        retval = ERROR_INTERNAL;
                        break;
                    }
                    if (dynStrListPushBack(codeStrList, temp[i])) {
                        retval = ERROR_INTERNAL;
                        break;
                    }
                }
                break;
            case E_S_EXPRESSION:
                retval = processExpression(operationElement.data.elements[i], pushToStack);
                break;
            default:
                return ERROR_SEMANTIC_OTHER;
        }
        if (retval) {
            // free dynamic strings if they are not appended to the codeStrList
            if (i != 1 || operationElement.data.elements[1].type != E_S_EXPRESSION) {
                dynStrFree(temp[1]);
                dynStrFree(temp[0]);
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
                retval = dynStrAppendString(temp[2], "ADDS\n");
            else
                retval = dynStrAppendString(temp[2], "ADD ");
            break;
        case E_SUB:
            if(*pushToStack)
                retval = dynStrAppendString(temp[2], "SUBS\n");
            else
                retval = dynStrAppendString(temp[2], "SUB ");
            break;
        case E_MUL:
            if(*pushToStack)
                retval = dynStrAppendString(temp[2], "MULS\n");
            else
                retval = dynStrAppendString(temp[2], "MUL ");
            break;
        case E_DIV:
            if(*pushToStack)
                retval = dynStrAppendString(temp[2], "DIVS\n");
            else
                retval = dynStrAppendString(temp[2], "DIV ");
            break;
        case E_DIV_INT:
            if(*pushToStack)
                retval = dynStrAppendString(temp[2], "IDIVS\n");
            else
                retval = dynStrAppendString(temp[2], "IDIV ");
            break;
        case E_AND:
            if(*pushToStack)
                retval = dynStrAppendString(temp[2], "ANDS\n");
            else
                retval = dynStrAppendString(temp[2], "AND ");
            break;
        case E_OR:
            if(*pushToStack)
                retval = dynStrAppendString(temp[2], "ORS\n");
            else
                retval = dynStrAppendString(temp[2], "OR ");
            break;
        case E_EQ:
            if(*pushToStack)
                retval = dynStrAppendString(temp[2], "EQS\n");
            else
                retval = dynStrAppendString(temp[2], "EQ ");
            break;
        case E_LT:
            if(*pushToStack)
                retval = dynStrAppendString(temp[2], "LTS\n");
            else
                retval = dynStrAppendString(temp[2], "LT ");
            break;
        case E_GT:
            if(*pushToStack)
                retval = dynStrAppendString(temp[2], "GTS\n");
            else
                retval = dynStrAppendString(temp[2], "GT ");
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
        if(dynStrAppendString(temp[0], " ")
        && dynStrAppendString(temp[0], dynStrGetString(temp[1]))
        && dynStrAppendString(temp[0], "\n")) {
            dynStrFree(temp[0]);
            dynStrFree(temp[1]);
            dynStrFree(temp[2]);
            return ERROR_INTERNAL;
        }
        // free second string after concatenation
        dynStrFree(temp[1]);

        if(dynStrListPushBack(codeStrList, temp[0])) {
            dynStrFree(temp[0]);
            dynStrFree(temp[2]);
            return ERROR_INTERNAL;
        }
    }

    // add to list
    if (dynStrListPushBack(codeStrList, temp[2])) {
        dynStrFree(temp[2]);
        if (!(*pushToStack)) {
            dynStrFree(temp[1]);
            dynStrFree(temp[0]);
        }
        return ERROR_INTERNAL;
    }

    return ERROR_SUCCESS;
}

int processUnaryOperation(treeElement_t operationElement, bool* pushToStack){

    if(operationElement.nodeSize != 1) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    dynStr_t* temp[2] = { NULL, NULL };

    switch (operationElement.data.elements[0].type) {
        case E_TOKEN:
            temp[0] = dynStrInit();
            if(*pushToStack) {
                if (dynStrAppendString(temp[0], "PUSHS ")) {
                    dynStrFree(temp[0]);
                    retval = ERROR_INTERNAL;
                    break;
                }
            }
            retval = processEToken(operationElement.data.elements[0], temp[0], pushToStack);
            if(retval)
                break;
            if(*pushToStack) {
                if (dynStrAppendString(temp[0], "\n")) {
                    retval = ERROR_INTERNAL;
                }
            }
            break;
        case E_S_EXPRESSION:
            retval = processExpression(operationElement.data.elements[0], pushToStack);
            break;
        default:
            retval = ERROR_SEMANTIC_OTHER;
    }
    if(retval) {
        dynStrFree(temp[0]);
        return retval;
    }

    temp[1] = dynStrInit();

    switch (operationElement.type){
        case E_NOT:
            if(*pushToStack)
                retval = dynStrAppendString(temp[1], "NOTS\n");
            else
                retval = dynStrAppendString(temp[1], "NOT ");
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

    if(dynStrListPushBack(codeStrList, temp[0])
    && dynStrListPushBack(codeStrList, temp[1])) {
        dynStrFree(temp[0]);
        dynStrFree(temp[1]);
        return ERROR_INTERNAL;
    }

    return ERROR_SUCCESS;
}

int processIf(treeElement_t ifElement) {
    if(ifElement.type != E_S_IF) {
        return ERROR_SEMANTIC_OTHER;
    }

    // used to make labels unique
    static unsigned ifCounter = 0;

    // expression
    if(processExpression(ifElement.data.elements[0], false)){
        return ERROR_SEMANTIC_OTHER;
    }

    dynStr_t* temp = dynStrInit();
    if(!temp) {
        return ERROR_INTERNAL;
    }

    int retval = ERROR_SUCCESS;

    retval = numberToDynStr(temp, "PUSHS bool@true\nJUMPIFEQS $if%d\n", ifCounter);

    if(retval){
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to code list
    retval = dynStrListPushBack(codeStrList, temp);
    if(retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // fall through to else

    // else body
    if(ifElement.nodeSize > 2) {
        retval = processElse(ifElement.data.elements[2]);
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
    retval = dynStrListPushBack(codeStrList, temp);
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
    retval = dynStrListPushBack(codeStrList, temp);
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
    retval = dynStrListPushBack(codeStrList, temp);
    if(retval){
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // if body
    if(processCodeBlock(ifElement.data.elements[1], NULL)){
        return ERROR_SEMANTIC_OTHER;
    }

    // add fi (end of if-else)
    temp = dynStrInit();
    retval = numberToDynStr(temp, "LABEL $fi%d\n", ifCounter);

    if(retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to code list
    retval = dynStrListPushBack(codeStrList, temp);
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
    char* buffer = malloc(strSize + 1); // + \0
    // check allocation
    if(!buffer) {
        return ERROR_INTERNAL;
    }

    // print string to buffer
    snprintf(buffer, strSize, formatString, number);
    buffer[strSize] = '\0';

    // append buffer to dynamic string
    retval = dynStrAppendString(outputStr, buffer);
    free(buffer);
    if(retval) {
        return ERROR_INTERNAL;
    }

    return retval; // ERROR_SUCCESS == 0, ERROR_INTERNAL == 99
}

int floatToDynStr(dynStr_t* outputStr, char* formatString, float number) {
    int retval; // return value

    // get size of string
    unsigned strSize = snprintf(NULL, 0, formatString, number);
    char* buffer = malloc(strSize + 1); // + \0
    // check allocation
    if(!buffer) {
        return ERROR_INTERNAL;
    }

    // print string to buffer
    snprintf(buffer, strSize, formatString, number);
    buffer[strSize] = '\0';

    // append buffer to dynamic string
    retval = dynStrAppendString(outputStr, buffer);
    free(buffer);
    if(retval) {
        return ERROR_INTERNAL;
    }

    return retval; // ERROR_SUCCESS == 0, ERROR_INTERNAL == 99
}

int processElse(treeElement_t elseElement) {
    if(elseElement.type != E_S_ELSE) {
        return ERROR_SEMANTIC_OTHER;
    }

    if(elseElement.nodeSize != 1) {
        return ERROR_SEMANTIC_OTHER;
    }

    if(processCodeBlock(elseElement.data.elements[0], NULL)) {
        return ERROR_SEMANTIC_OTHER;
    }

    return ERROR_SUCCESS;
}

int processAssign(treeElement_t assignElement) {
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

    dynStr_t* temp;
    int retval = ERROR_SUCCESS;
    bool pushToStack = false;

    switch (assignElement.data.elements[1].type) {
        case E_S_EXPRESSION: // func call/expression ( l = f() | l = a + b)
            retval = processExpression(assignElement.data.elements[1], &pushToStack);
            if(retval) {
                return retval;
            }
            temp = dynStrInit();
            if(pushToStack) { // pop from stack
                if (dynStrAppendString(temp, "POPS ")) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
            } else { // add binary operation from expression processing (last element of stack)
                dynStr_t* lastStr = dynStrListElGet(dynStrListBack(codeStrList));
                if(dynStrAppendString(temp, dynStrGetString(lastStr))) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
                // remove operation from list
                dynStrListPopBack(codeStrList);
            }
            // get variable id
            retval = processEToken(assignElement.data.elements[0], temp, false);
            if(retval) {
                dynStrFree(temp);
                return retval;
            }
            if(pushToStack) {
                // add eol to pop
                if (dynStrAppendString(temp, "\n")) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
            } else {
                if(dynStrAppendString(temp, " ")) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
                // add operands to binary operation
                dynStr_t* lastStr = dynStrListElGet(dynStrListBack(codeStrList));
                if(dynStrAppendString(temp, dynStrGetString(lastStr))) {
                    dynStrFree(temp);
                    return ERROR_INTERNAL;
                }
                // remove operands from list
                dynStrListPopBack(codeStrList);
            }
            break;
        case E_TOKEN: // value or id ( l = r )
            temp = dynStrInit();
            if(dynStrAppendString(temp, "MOVE ")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }

            // process left side
            retval = processEToken(assignElement.data.elements[0], temp, false);
            if(retval) {
                dynStrFree(temp);
                return retval;
            }
            if(dynStrAppendString(temp, " ")) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            // process right side
            retval = processEToken(assignElement.data.elements[1], temp, false);
            if(retval) {
                dynStrFree(temp);
                return retval;
            }
            // add eol
            if(dynStrAppendString(temp, "\n")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            break;
        default:
            return ERROR_SEMANTIC_OTHER;
    }
    // add to list
    if(dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    return retval;
}

int processFunctionCall(treeElement_t callElement) {
    if(callElement.type != E_S_FUNCTION_CALL) {
        return ERROR_SEMANTIC_OTHER;
    }

    if(callElement.nodeSize != 2) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    // function name
    dynStr_t* fname = dynStrInit();
    retval = processEToken(callElement.data.elements[0], fname, true);
    if(retval) {
        dynStrFree(fname);
        return retval;
    }

    // add create frame
    dynStr_t* temp = dynStrInit();
    if(dynStrAppendString(temp, "CREATEFRAME\n")) {
        dynStrFree(fname);
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to list
    if(dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(fname);
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // TODO
    //  remove temporary frame and hardcode it in processFunctionCallParams?

    symbolFrame_t temp_frame = current_frame;
    current_frame = FRAME_TEMP;

    // create frame and process params
    retval = processFunctionCallParams(callElement.data.elements[1], fname);
    if(retval) {
        return retval;
    }

    // add pushframe
    temp = dynStrInit();
    if(dynStrAppendString(temp, "PUSHFRAME\n")) {
        dynStrFree(fname);
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // add to list
    if(dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(fname);
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // call the function
    temp = dynStrInit();
    if(dynStrAppendString(temp, "CALL ")) {
        dynStrFree(fname);
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    // function name
    if(dynStrAppendString(temp, dynStrGetString(fname))){
        dynStrFree(fname);
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    dynStrFree(fname);
    if(dynStrAppendString(temp, "\n")) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    if(dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // pop frame
    temp = dynStrInit();
    if(dynStrAppendString(temp, "POPFRAME\n")) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }
    if(dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }


    //TODO
    // this will be in expression processing:
    // printf("PUSH %s@$retval\n", FRAME_NAME[current_frame]);

    // restore frame
    current_frame = temp_frame;

    return ERROR_SUCCESS;
}

int processFunctionCallParams(treeElement_t callParamsElement, dynStr_t* functionName) {
    if(callParamsElement.type != E_S_FUNCTION_CALL_PARAMS) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    dynStr_t* temp = NULL;
    dynStr_t* argName = NULL;
    // process arg[i]
    for(unsigned i = 0; i < callParamsElement.nodeSize; i++) {
        bool pushToStack = false;

        //TODO
        // process params and create assignment after loop (because of expressions and function calls)

        // get expression data
        retval = processExpression(callParamsElement.data.elements[i], &pushToStack);
        if(retval){
            return retval;
        }
        // get argument variable name
        argName = symTableGetArgumentName(table, functionName, i);
        // check if name exists
        if(!argName) {
            return ERROR_SEMANTIC_OTHER;
        }
        // create data assignment
        temp = dynStrInit();
        // value is on stack
        if(pushToStack) {
            if(dynStrAppendString(temp, "POPS ")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(dynStrAppendString(temp, "TF@")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(dynStrAppendString(temp, dynStrGetString(argName))) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(dynStrAppendString(temp, "\n")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
        }
        // value is assigned directly to variable
        else {
            // get operation from list
            dynStr_t* lastStr = dynStrListElGet(dynStrListBack(codeStrList));
            if(dynStrAppendString(temp, dynStrGetString(lastStr))) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            // remove operation from list
            dynStrListPopBack(codeStrList);
            if(dynStrAppendString(temp, "TF@")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(dynStrAppendString(temp, dynStrGetString(argName))) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(dynStrAppendString(temp, " ")) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            // add operands
            lastStr = dynStrListElGet(dynStrListBack(codeStrList));
            if(dynStrAppendString(temp, dynStrGetString(lastStr))) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            // remove operands from list
            dynStrListPopBack(codeStrList);
        }
        // add arg to list
        if(dynStrListPushBack(codeStrList, temp)) {
            dynStrFree(temp);
            return ERROR_INTERNAL;
        }

    } // for

    return ERROR_SUCCESS;
}
