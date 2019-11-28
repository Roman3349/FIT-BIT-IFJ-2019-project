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

int ic_generator(FILE* file) {

    assert(file);

    treeElement_t code = syntaxParse(file);

    if(processCode(code)) {
        return -1; // code processing failed
    }

    return 0;
}

int processCode(treeElement_t codeElement) {

    assert(codeElement.type == E_CODE);

    //TODO
    // process code content precode (header?)

    // process code content
    switch (((treeElement_t*)codeElement.data)->type) {
        case E_TOKEN:
            processEToken(codeElement.data);
            break;
        case E_S_FUNCTION_DEF:
            processFunctionDefinition();
            break;
        case E_CODE_BLOCK:
            processCodeBlock();
            break;
        default:
            return -1; // failed to process the code
    }

    //TODO
    // process code content ending

    return 0;
}

int processEToken(treeElement_t* EToken, void* tval) {

    assert(EToken);
    assert(EToken->type == E_TOKEN);

    switch (((token_t*)EToken->data)->type) {
        case T_NUMBER:
            *(long *)tval = ((token_t*)EToken->data)->data.intval;
            break;
        case T_FLOAT:
            *(double *)tval = ((token_t*)EToken->data)->data.floatval;
            break;
        case T_STRING_ML:
        case T_STRING:
            *(dynStr_t **)tval = ((token_t*)EToken->data)->data.strval;
            break;
        default:
            return -1;
    }

    return 0;
}

int processFunctionDefinition() {

    return 0;
}

int processCodeBlock() {

    return 0;
}

int processAdd(treeElement_t* AddToken) {

    assert(AddToken->type == E_ADD);
    assert((((treeElement_t*)AddToken->data)[0]).type == E_TOKEN);
    assert((((treeElement_t*)AddToken->data)[1]).type == E_TOKEN);

    //TODO
    // get numbers from tokens and generate code

    return 0;
}