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

// current frame
symbolFrame_t current_frame = FRAME_GLOBAL;

// current context
dynStr_t* current_context = NULL;

// symbol table
symTable_t* table;

int processCode(treeElement_t codeElement) {

    assert(codeElement.type == E_CODE);

    // IFJcode19 shebang
    printf(".IFJcode19\n");

    for (unsigned i = 0; i < codeElement.nodeSize; i++) {
        // process code content
        switch (codeElement.data.elements[i].type) {
            case E_TOKEN:
                processEToken(codeElement.data.elements[i], NULL);
                break;
            case E_S_FUNCTION_DEF:
                processFunctionDefinition(codeElement.data.elements[i]);
                break;
            case E_CODE_BLOCK:
                processCodeBlock(codeElement.data.elements[i], NULL);
                break;
            default:
                return -1; // failed to process the code
        }
    }

    //TODO
    // process code content ending

    return 0;
}

int processEToken(treeElement_t eTokenElement, dynStr_t** context) {

    if(eTokenElement.type == E_TOKEN) {
        return -1;
    }

    switch (eTokenElement.data.token->type) {
        case T_NUMBER:
            printf("int@%ld ", eTokenElement.data.token->data.intval);
            break;
        case T_FLOAT:
            printf("float@%a ", eTokenElement.data.token->data.floatval);
            break;
        case T_STRING_ML:
        case T_STRING:
            if(!dynStrEscape(eTokenElement.data.token->data.strval)) {
                return -1;
            }
            printf("string@%s ", eTokenElement.data.token->data.strval->string);
            break;
        case T_ID:
            if(context) {
                *context = eTokenElement.data.token->data.strval;
            }
            //TODO
            // this line may not work for function params
            symbolFrame_t idFrame = symTableGetFrame(table, eTokenElement.data.token->data.strval, current_context);
            printf("%s@%s ", FRAME_NAME[idFrame],eTokenElement.data.token->data.strval->string);
            break;
        default:
            return -1;
    }

    return 0;
}

int processFunctionDefinition(treeElement_t defElement) {

    if(defElement.type != E_S_FUNCTION_DEF) {
        return -1;
    }

    // processing function name
    // (label to jump to when function is called)
    printf("LABEL %s@:", FRAME_NAME[current_frame]);
    dynStr_t* function_context = NULL;
    if(processEToken(defElement.data.elements[0], &function_context)) {
        return -1;
    }
    printf("\n");

    //TODO
    // process params
    //current_frame = TF;
    //if(processFunctionDefParams(defElement.data.elements[1])) {
    //    return -1;
    //}

    // process function body
    if(processCodeBlock(defElement.data.elements[2], function_context)) {
        return -1;
    }

    //TODO
    // add print return?

    return 0;
}

int processCodeBlock(treeElement_t codeBlockElement, dynStr_t* context) {

    if(codeBlockElement.type != E_CODE_BLOCK) {
        return -1;
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
        switch (codeBlockElement.data.elements[i].type) {
            case E_S_EXPRESSION:
                processExpression(codeBlockElement.data.elements[i]);
                break;
            case E_S_IF:
                processIf(codeBlockElement.data.elements[i]);
                break;
            case E_S_WHILE:
                 //processWhile(codeBlockElement.data.elements[i]);
                break;
            default:
                return -1;
        }
    }

    // restore state
    if(context) {
        current_context = temp_context;
        current_frame = temp_frame;
    }

    // BLOCK END

    return 0;
}

int processExpression(treeElement_t expElement) {

    if(expElement.type != E_S_EXPRESSION) {
        return -1;
    }

    switch (expElement.data.elements[0].type) {
        case E_TOKEN:
            printf("PUSHS ");
            if(processEToken(expElement.data.elements[0], NULL)) {
                return -1;
            }
            printf("\n");
            break;
        case E_S_EXPRESSION:
            if(processExpression(expElement.data.elements[0])) {
                return -1;
            }
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
            if(processBinaryOperation(expElement.data.elements[0])) {
                return -1;
            }
            break;
        case E_NOT:
			if(processUnaryOperation(expElement.data.elements[0])) {
			    return -1;
			}
            break;
        case E_S_FUNCTION_CALL:
            //processFunctionCall(expElement.data.elements[0]);
            break;
        case E_ASSIGN:
            if(processAssign(expElement.data.elements[0])) {
                return -1;
            }
            break;
        default:
            return -1;
    }

    return 0;
}

int processBinaryOperation(treeElement_t operationElement) {

    if(operationElement.nodeSize != 2) {
        return -1;
    }

    // extract data in reverse order and push them to stack
    for(unsigned i = operationElement.nodeSize; i > 0; i--) {
        switch (operationElement.data.elements[i].type) {
            case E_TOKEN:
                printf("PUSHS ");
                processEToken(operationElement.data.elements[i], NULL);
                printf("\n");
                break;
            case E_S_EXPRESSION:
                processExpression(operationElement.data.elements[i]);
                break;
            default:
                return -1;
        }
    }

    switch (operationElement.type) {
        case E_ADD:
            printf("ADDS\n");
            break;
        case E_SUB:
            printf("SUBS\n");
            break;
        case E_MUL:
            printf("MULS\n");
            break;
        case E_DIV:
            printf("DIVS\n");
            break;
        case E_DIV_INT:
            printf("IDIVS\n");
            break;
        case E_AND:
            printf("ANDS\n");
            break;
        case E_OR:
            printf("ORS\n");
            break;
        case E_EQ:
            printf("EQS\n");
            break;
        case E_LT:
            printf("LTS\n");
            break;
        case E_GT:
            printf("GTS\n");
            break;
        default:
            return -1;
    }

    return 0;
}

int processUnaryOperation(treeElement_t operationElement){

    if(operationElement.nodeSize != 1) {
        return -1;
    }

    switch (operationElement.data.elements[0].type) {
        case E_TOKEN:
            printf("PUSHS ");
            processEToken(operationElement.data.elements[0], NULL);
            printf("\n");
            break;
        case E_S_EXPRESSION:
            processExpression(operationElement.data.elements[0]);
            break;
        default:
            return -1;
    }

    switch (operationElement.type){
        case E_NOT:
            printf("NOTS\n");
            break;
        default:
            return -1;
    }

    return 0;
}

int processIf(treeElement_t ifElement) {
    if(ifElement.type != E_S_IF) {
        return -1;
    }

    // used to make labels unique
    static unsigned ifCounter = 0;

    // expression
    if(processExpression(ifElement.data.elements[0])){
        return -1;
    }

    // jump to if body if condition is true
    printf("PUSHS bool@true\nJUMPIFEQS %s@:if%d\n", FRAME_NAME[current_frame],ifCounter);

    // fall through to else

    // else body
    if(ifElement.nodeSize > 2) {
        if(processElse(ifElement.data.elements[2])) {
            return -1;
        }
    }

    // add jump to fi
    printf("JUMP %s@:fi%d\n", FRAME_NAME[current_frame],ifCounter);

    // add if label
    printf("LABEL %s@:if%d\n", FRAME_NAME[current_frame],ifCounter);

    // if body
    if(processCodeBlock(ifElement.data.elements[1], NULL)){
        return -1;
    }

    // add fi (end of if-else)
    printf("LABEL %s@:fi%d\n", FRAME_NAME[current_frame],ifCounter);

    return 0;
}

int processElse(treeElement_t elseElement) {
    if(elseElement.type != E_S_ELSE) {
        return -1;
    }

    if(elseElement.nodeSize != 1) {
        return -1;
    }

    if(processCodeBlock(elseElement.data.elements[0], NULL)) {
        return -1;
    }

    return 0;
}

int processAssign(treeElement_t assignElement) {
    if(assignElement.type != E_ASSIGN) {
        return  -1;
    }

    if(assignElement.nodeSize != 2) {
        return -1;
    }

    // left side of assignment must be id (variable)
    if(assignElement.data.elements[0].type != E_TOKEN) {
        return -1;
    }


    // TODO
    //  frame processing in var names


    switch (assignElement.data.elements[1].type) {
        case E_S_EXPRESSION: // func call/expression ( l = f() | l = a + b)
            if(processExpression(assignElement.data.elements[1])) {
                return -1;
            }
            printf("POPS ");
            if(processEToken(assignElement.data.elements[0], NULL)) {
                return -1;
            }
            printf("\n");
            break;
        case E_TOKEN: // value or id ( l = r )
            printf("MOVE ");
            // process right side
            if(processEToken(assignElement.data.elements[0], NULL)) {
                return -1;
            }
            // process left side
            if(processEToken(assignElement.data.elements[1], NULL)) {
                return -1;
            }
            printf("\n");
            break;
        default:
            return -1;
    }

    return 0;
}

int processFunctionDefParams(treeElement_t defParamsElement) {
    if(defParamsElement.type != E_S_FUNCTION_DEF_PARAMS) {
        return -1;
    }

    //TODO
    // add temp frame

    for(unsigned i = 0; i < defParamsElement.nodeSize; i++) {
        if(processEToken(defParamsElement.data.elements[i], NULL)) {
            return -1;
        }
    }

    return 0;
}

int processFunctionCall(treeElement_t callElement) {
    if(callElement.type != E_S_FUNCTION_CALL) {
        return -1;
    }

    if(callElement.nodeSize != 2) {
        return -1;
    }

    printf("CREATEFRAME\n");
    symbolFrame_t temp_frame = current_frame;
    current_frame = FRAME_TEMP;

    // create frame and process params
    if(processFunctionCallParams(callElement.data.elements[1])) {
        return -1;
    }

    current_frame = temp_frame;

    printf("PUSHFRAME\n");

    printf("CALL ");

    // function name
    if(processEToken(callElement.data.elements[0], NULL)) {
        return -1;
    }
    printf("\n");

    printf("POPFRAME\n");
    printf("PUSH TF@retval\n");

    return 0;
}

int processFunctionCallParams(treeElement_t callParamsElement) {
    if(callParamsElement.type != E_S_FUNCTION_CALL_PARAMS) {
        return -1;
    }

    // process arg[i]
    for(unsigned i = 0; i < callParamsElement.nodeSize; i++) {
        if(processExpression(callParamsElement.data.elements[i])) {
            return -1;
        }
        printf("POPS %s@arg%d\n", FRAME_NAME[current_frame],i);
    }

    return 0;
}
