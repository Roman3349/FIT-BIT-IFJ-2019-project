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
        int retval = 0;
        // process code content
        switch (codeElement.data.elements[i].type) {
            case E_TOKEN:
                retval = processEToken(codeElement.data.elements[i], NULL);
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

int processEToken(treeElement_t eTokenElement, dynStr_t** id) {

    if(eTokenElement.type == E_TOKEN) {
        return ERROR_SEMANTIC_OTHER;
    }

    // id have to be returned, but token type doesnt match
    if(id && eTokenElement.data.token->type != T_ID) {
        return ERROR_SEMANTIC_OTHER;
    }

    dynStr_t *element;
    if((element = dynStrInit()) == NULL) {
        return ERROR_INTERNAL;
    }
    unsigned strSize = 0;
    char* buffer = NULL;

    switch (eTokenElement.data.token->type) {
        case T_NUMBER:
            // add type
            if(dynStrAppendString(element, "int@")){
                dynStrFree(element);
                return ERROR_INTERNAL;
            }
            // get size of string where number will be converted to
            strSize = snprintf(NULL, 0, "%ld", eTokenElement.data.token->data.intval);
            // string buffer
            if((buffer = malloc(strSize + 2)) == NULL) { // + space and \0
                dynStrFree(element);
                return ERROR_INTERNAL;
            }
            // convert, add space and termination character
            snprintf(buffer, strSize, "%ld", eTokenElement.data.token->data.intval);
            buffer[strSize] = ' ';
            buffer[strSize + 1] = '\0';
            // add to rest of the code
            if(dynStrAppendString(element, buffer)) {
                dynStrFree(element);
                free(buffer);
                return ERROR_INTERNAL;
            }
            if(dynStrListPushBack(codeStrList, element)) {
                dynStrFree(element);
                free(buffer);
                return ERROR_INTERNAL;
            }
            free(buffer);
            break;
        case T_FLOAT:
            // add type
            if(dynStrAppendString(element, "float@")){
                dynStrFree(element);
                return ERROR_INTERNAL;
            }
            // get size of string where number will be converted to
            strSize = snprintf(NULL, 0, "%a", eTokenElement.data.token->data.floatval);
            // string buffer
            if((buffer = malloc(strSize + 2)) == NULL) { // + space and \0
                dynStrFree(element);
                return ERROR_INTERNAL;
            }
            // convert, add space and termination character
            snprintf(buffer, strSize, "%a", eTokenElement.data.token->data.floatval);
            buffer[strSize] = ' ';
            buffer[strSize + 1] = '\0';
            // add to rest of the code
            if(dynStrAppendString(element, buffer)) {
                dynStrFree(element);
                free(buffer);
                return ERROR_INTERNAL;
            }
            if(dynStrListPushBack(codeStrList, element)) {
                dynStrFree(element);
                free(buffer);
                return ERROR_INTERNAL;
            }
            free(buffer);
            break;
        case T_STRING_ML:
        case T_STRING:
            if(!dynStrEscape(eTokenElement.data.token->data.strval)) {
                dynStrFree(element);
                return ERROR_SEMANTIC_OTHER;
            }
            if(dynStrAppendString(element, "string@")){
                dynStrFree(element);
                return ERROR_INTERNAL;
            }
            if(dynStrAppendString(element, eTokenElement.data.token->data.strval->string)) {
                dynStrFree(element);
                return ERROR_INTERNAL;
            }
            break;
        case T_ID:
            if(id) { // return id
                dynStrFree(element); // no code is added
                *id = eTokenElement.data.token->data.strval;
            }
            else {
                // determine if variable is local or global
                symbolFrame_t idFrame = symTableGetFrame(table, eTokenElement.data.token->data.strval, current_context);
                if (dynStrAppendString(element, FRAME_NAME[idFrame])) {
                    dynStrFree(element);
                    return ERROR_INTERNAL;
                }
                if(dynStrAppendString(element, "@")) {
                    dynStrFree(element);
                    return ERROR_INTERNAL;
                }
                // add name
                if(dynStrAppendString(element, eTokenElement.data.token->data.strval->string)) {
                    dynStrFree(element);
                    return ERROR_INTERNAL;
                }
            }
            break;
        default:
            dynStrFree(element);
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
    retval = processEToken(defElement.data.elements[0], &function_context);

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
                retval = processExpression(codeBlockElement.data.elements[i]);
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

int processExpression(treeElement_t expElement) {

    if(expElement.type != E_S_EXPRESSION) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    switch (expElement.data.elements[0].type) {
        case E_TOKEN:
            //TODO
            // allocation / dealocation
            printf("PUSHS ");
            retval = processEToken(expElement.data.elements[0], NULL);
            if(retval) {
                break;
            }
            printf("\n");
            break;
        case E_S_EXPRESSION:
            retval = processExpression(expElement.data.elements[0]);
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
            retval = processBinaryOperation(expElement.data.elements[0]);
            break;
        case E_NOT:
			retval = processUnaryOperation(expElement.data.elements[0]);
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

int processBinaryOperation(treeElement_t operationElement) {

    if(operationElement.nodeSize != 2) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval;

    dynStr_t* temp;

    // extract data in reverse order and push them to stack
    for(unsigned i = operationElement.nodeSize; i > 0; i--) {
        switch (operationElement.data.elements[i].type) {
            case E_TOKEN:
                temp = dynStrInit();
                if(dynStrAppendString(temp, "PUSHS ")) {
                    dynStrFree(temp);
                    retval = ERROR_INTERNAL;
                    break;
                }
                if(dynStrListPushBack(codeStrList, temp)) {
                    dynStrFree(temp);
                    retval = ERROR_INTERNAL;
                    break;
                }
                retval = processEToken(operationElement.data.elements[i], NULL);
                if(retval) {
                    break;
                }
                if(dynStrAppendString(temp, "\n")){
                    retval = ERROR_INTERNAL;
                }
                break;
            case E_S_EXPRESSION:
                retval = processExpression(operationElement.data.elements[i]);
                break;
            default:
                return ERROR_SEMANTIC_OTHER;
        }
        if(retval) {
            return retval;
        }
    }

    temp = dynStrInit();

    switch (operationElement.type) {
        case E_ADD:
            retval = dynStrAppendString(temp, "ADDS\n");
            break;
        case E_SUB:
            retval = dynStrAppendString(temp, "SUBS\n");
            break;
        case E_MUL:
            retval = dynStrAppendString(temp, "MULS\n");
            break;
        case E_DIV:
            retval = dynStrAppendString(temp, "DIVS\n");
            break;
        case E_DIV_INT:
            retval = dynStrAppendString(temp, "IDIVS\n");
            break;
        case E_AND:
            retval = dynStrAppendString(temp, "ANDS\n");
            break;
        case E_OR:
            retval = dynStrAppendString(temp, "ORS\n");
            break;
        case E_EQ:
            retval = dynStrAppendString(temp, "EQS\n");
            break;
        case E_LT:
            retval = dynStrAppendString(temp, "LTS\n");
            break;
        case E_GT:
            retval = dynStrAppendString(temp, "GTS\n");
            break;
        default:
            return ERROR_SEMANTIC_OTHER;
    }

    if(retval) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    // add to list
    if(dynStrListPushBack(codeStrList, temp)) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    return ERROR_SUCCESS;
}

int processUnaryOperation(treeElement_t operationElement){

    if(operationElement.nodeSize != 1) {
        return ERROR_SEMANTIC_OTHER;
    }

    int retval = ERROR_SUCCESS;

    dynStr_t* temp;

    switch (operationElement.data.elements[0].type) {
        case E_TOKEN:
            temp = dynStrInit();
            if(dynStrAppendString(temp, "PUSHS ")) {
                dynStrFree(temp);
                retval = ERROR_INTERNAL;
                break;
            }
            if(dynStrListPushBack(codeStrList, temp)) {
                dynStrFree(temp);
                retval = ERROR_INTERNAL;
                break;
            }
            //TODO
            // apend data returned from token
            retval = processEToken(operationElement.data.elements[0], NULL);
            if(retval)
                break;
            if(dynStrAppendString(temp, "\n")) {
                retval = ERROR_INTERNAL;
            }
            break;
        case E_S_EXPRESSION:
            processExpression(operationElement.data.elements[0]);
            break;
        default:
            return ERROR_SEMANTIC_OTHER;
    }
    if(retval)
        return retval;

    temp = dynStrInit();

    switch (operationElement.type){
        case E_NOT:
            if(dynStrAppendString(temp, "NOTS\n")) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            if(dynStrListPushBack(codeStrList, temp)) {
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            break;
        default:
            return ERROR_SEMANTIC_OTHER;
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
    if(processExpression(ifElement.data.elements[0])){
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

    switch (assignElement.data.elements[1].type) {
        case E_S_EXPRESSION: // func call/expression ( l = f() | l = a + b)
            retval = processExpression(assignElement.data.elements[1]);
            if(retval) {
                return retval;
            }
            temp = dynStrInit();
            if(dynStrAppendString(temp, "POPS ")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }

            // TODO
            //  process token name

            retval = processEToken(assignElement.data.elements[0], NULL);
            if(retval) {
                dynStrFree(temp);
                return retval;
            }
            if(dynStrAppendString(temp, "\n")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            break;
        case E_TOKEN: // value or id ( l = r )
            temp = dynStrInit();
            if(dynStrAppendString(temp, "MOVE ")){
                dynStrFree(temp);
                return ERROR_INTERNAL;
            }
            //TODO
            // process token names

            // process right side
            retval = processEToken(assignElement.data.elements[0], NULL);
            if(retval) {
                dynStrFree(temp);
                return retval;
            }
            // process left side
            retval = processEToken(assignElement.data.elements[1], NULL);
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

    dynStr_t* temp = dynStrInit();
    if(dynStrAppendString(temp, "CREATEFRAME\n")) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    symbolFrame_t temp_frame = current_frame;
    current_frame = FRAME_TEMP;

    // create frame and process params
    retval = processFunctionCallParams(callElement.data.elements[1]);
    if(retval) {
        return retval;
    }

    temp = dynStrInit();
    if(dynStrAppendString(temp, "PUSHFRAME\n")) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    //TODO
    // this part have to be processed before params to know function name
    // can be generated to string after params...

    temp = dynStrInit();
    if(dynStrAppendString(temp, "CALL ")) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    //TODO
    // add token name to the string

    // function name
    retval = processEToken(callElement.data.elements[0], NULL);
    if(retval) {
        return retval;
    }
    if(dynStrAppendString(temp, "\n")) {
        dynStrFree(temp);
        return ERROR_INTERNAL;
    }

    printf("POPFRAME\n");

    //TODO
    // this will be in expression processin:
    // printf("PUSH %s@$retval\n", FRAME_NAME[current_frame]);

    // restore frame
    current_frame = temp_frame;

    return ERROR_SUCCESS;
}

int processFunctionCallParams(treeElement_t callParamsElement) {
    if(callParamsElement.type != E_S_FUNCTION_CALL_PARAMS) {
        return ERROR_SEMANTIC_OTHER;
    }

    dynStr_t* temp = dynStrInit();
    if(!temp) {
        return ERROR_INTERNAL;
    }
    // process arg[i]
    for(unsigned i = 0; i < callParamsElement.nodeSize; i++) {

        //TODO
        // add dynamic string print
        // add determine name of target variable (arg)

        if(processExpression(callParamsElement.data.elements[i])) {
            return ERROR_SEMANTIC_OTHER;
        }
        //printf("POPS %s@arg%d\n", FRAME_NAME[current_frame],i);
    }

    return ERROR_SUCCESS;
}
