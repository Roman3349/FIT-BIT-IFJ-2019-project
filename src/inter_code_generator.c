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
        [GF] = "GF",
        [LF] = "LF",
        [TF] = "TF"
};

// current frame
enum frameType current_frame = GF;

int processCode(treeElement_t codeElement) {

    assert(codeElement.type == E_CODE);

    // IFJcode19 shebang
    printf(".IFJcode19\n");

    for (unsigned i = 0; i < codeElement.nodeSize; i++) {
        // process code content
        switch (codeElement.data.elements[i].type) {
            case E_TOKEN:
                processEToken(codeElement.data.elements[i]);
                break;
            case E_S_FUNCTION_DEF:
                processFunctionDefinition(codeElement.data.elements[i]);
                break;
            case E_CODE_BLOCK:
                processCodeBlock(codeElement.data.elements[i]);
                break;
            default:
                return -1; // failed to process the code
        }
    }

    //TODO
    // process code content ending

    return 0;
}

int processEToken(treeElement_t eTokenElement) {

    if(eTokenElement.type == E_TOKEN) {
        return -1;
    }

    switch (eTokenElement.data.token->type) {
        case T_NUMBER:
            printf("int@%ld", eTokenElement.data.token->data.intval);
            break;
        case T_FLOAT:
            printf("float@%a", eTokenElement.data.token->data.floatval);
            break;
        case T_STRING_ML:
        case T_STRING:
            printf("%s", "string@");
            if(!dynStrEscape(eTokenElement.data.token->data.strval)) {
                return -1;
            }
            printf("%s", eTokenElement.data.token->data.strval->string);
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

    //TODO
    // process function name
    // processID();

    //TODO
    // process function params
    // processFunctionParams();

    // process function body
    processCodeBlock(defElement.data.elements[2]);

    // add print return?

    return 0;
}

int processCodeBlock(treeElement_t codeBlockElement) {

    if(codeBlockElement.type != E_CODE_BLOCK) {
        return -1;
    }

    // BLOCK START

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
            processEToken(expElement.data.elements[0]);
            printf("\n");
            break;
        case E_S_EXPRESSION:
            processExpression(expElement.data.elements[0]);
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
			processBinaryOperation(expElement.data.elements[0]);
            break;
        case E_NOT:
			processUnaryOperation(expElement.data.elements[0]);
            break;
        case E_S_FUNCTION_CALL:
            //processFunctionCall(expElement.data.elements[0]);
            break;
        case E_ASSIGN:
            processAssign(expElement.data.elements[0]);
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
    for(unsigned i = 2; i > 0; i--) {
        switch (operationElement.data.elements[i].type) {
            case E_TOKEN:
                printf("PUSHS ");
                processEToken(operationElement.data.elements[i]);
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
            processEToken(operationElement.data.elements[0]);
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
    printf("%s@:if%d\n", FRAME_NAME[current_frame],ifCounter);

    // if body
    if(processCodeBlock(ifElement.data.elements[1])){
        return -1;
    }

    // add fi (end of if-else)
    printf("%s@:fi%d\n", FRAME_NAME[current_frame],ifCounter);

    return 0;
}

int processElse(treeElement_t elseElement) {
    if(elseElement.type != E_S_ELSE) {
        return -1;
    }

    if(elseElement.nodeSize != 1) {
        return -1;
    }

    if(processCodeBlock(elseElement.data.elements[0])) {
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

    //TODO
    // add switch

    return 0;
}
