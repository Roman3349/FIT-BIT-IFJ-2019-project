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

#pragma once

#include "parse_tree.h"
#include "parser.h"
#include "dynamic_string_list.h"


/*
 * Process body of the program
 * @param codeElement tree element containing the program code
 * @param symTable symbol table
 * @param context local scope function name
 * @return execution status
 */
int processCode(treeElement_t codeElement, symTable_t* symTable);

/*
 * Process element with token
 * @param eTokenElement tree element with token
 * @param outputDynStr dynamic string where output will be writen to
 * @param id_only tels if there should be FRAME_TYPE@id or id only in the output
 * @param symTable symbol table
 * @param context local scope function name
 * @return execution status
 */
int processEToken(treeElement_t eTokenElement, dynStr_t* outputDynStr, bool id_only,
        symTable_t* symTable, dynStr_t* context);

/*
 * Process definition of new function
 * @param defElement tree element with function definition
 * @param symTable symbol table
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @returns execution status
 */
int processFunctionDefinition(treeElement_t defElement, symTable_t* symTable,
        dynStr_t* context, dynStrList_t* codeStrList);

/*
 * Process block of code
 * @param codeBlockElement tree element containing block of code to process
 * @param symTable symbol table
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @return execution status
 */
int processCodeBlock(treeElement_t codeBlockElement, symTable_t* symTable,
        dynStr_t* context, dynStrList_t* codeStrList);

/*
 * Process expression
 * @param expElement tree element with expression
 * @param pushToStack if true, generates pushs instruction
 *                    for processed variables and constants
 *                    , is also used to return this value to calling function
 * @param symTable symbol table
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @return execution status
 */
int processExpression(treeElement_t expElement, bool* pushToStack, symTable_t* symTable,
        dynStr_t* context, dynStrList_t* codeStrList);

/*
 * Process operation with 2 operands
 * @param operationElement tree element containing operation to process
 * @param pushToStack if true, generates pushs instruction
 *                    for processed variables and constants
 *                    , is also used to return this value to calling function
 * @param symTable symbol table
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @return execution status
 */
int processBinaryOperation(treeElement_t operationElement, bool* pushToStack,
        symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList);

/*
 * Process operation operation with only 1 operand
 * @param operationElement tree element with operation to process
 * @param pushToStack if true, generates pushs instruction
 *                    for processed variables and constants
 *                    , is also used to return this value to calling function
 * @param symTable symbol table
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @return execution status
 */
int processUnaryOperation(treeElement_t operationElement, bool* pushToStack,
        symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList);

/*
 * Process if statement
 * @param ifElement tree element containing if statement
 * @param symTable symbol table
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @returns execution status
 */
int processIf(treeElement_t ifElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList);

/*
 * Convert number to dynamic string
 * @param outputStr dynamic string where number will be put to
 * @param formatString printf style format string
 * @param number number
 * @param codeStrList list of dynamic strings where code is generated to
 * @return execution status
 */
int numberToDynStr(dynStr_t* outputStr, char* formatString, long number);

/*
 * Convert float to dynamic string
 * @param outputStr dynamic string where number will be put to
 * @param formatString printf style format string
 * @param number float number
 * @param codeStrList list of dynamic strings where code is generated to
 * @return execution status
 */
int floatToDynStr(dynStr_t* outputStr, char* formatString, double number);

/*
 * Process else statement
 * @param elseElement tree element with else code block to process
 * @param symTable symbol table
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @return execution status
 */
int processElse(treeElement_t elseElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList);

/*
 * Process assignment of variable
 * @param assignElement element with assign expression
 * @param symTable symbol table
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @return execution status
 */
int processAssign(treeElement_t assignElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList);

/*
 * Create temporary frame and process function call
 * @param callElement tree element containing function call
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @return execution status
 */
int processFunctionCall(treeElement_t callElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList);

/*
 * Process function params in temporary frame
 * @param callParamsElement tree element containing called function parameters
 * @param symTable symbol table
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @return execution status
 */
int processFunctionCallParams(treeElement_t callParamsElement, symTable_t* symTable,
        dynStr_t* context, dynStrList_t* codeStrList);

/*
 * Process while function
 * @param whileElement tree element containing while function
 * @param symTable symbol table
 * @param context local scope function name
 * @param codeStrList list of dynamic strings where code is generated to
 * @return Execution status
 */
int processWhile(treeElement_t whileElement, symTable_t* symTable, dynStr_t* context, dynStrList_t* codeStrList);

/**
 * Generates an embedded functions
 * @param codeStrList List of dynamic string where the code is generated to
 * @return Execution status
 */
int generateEmbeddedFunctions(dynStrList_t *codeStrList);

/**
 * Generates len embedded function
 * @param codeStrList List of dynamic string where the code is generated to
 * @return Execution status
 */
int generateLenFunction(dynStrList_t* codeStrList);

/**
 * Generates inputi embedded function
 * @param codeStrList List of dynamic string where the code is generated to
 * @return Execution status
 */
int generateInputiFunction(dynStrList_t* codeStrList);

/**
 * Generates inputf embedded function
 * @param codeStrList List of dynamic string where the code is generated to
 * @return Execution status
 */
int generateInputfFunction(dynStrList_t* codeStrList);

/**
 * Generates inputs embedded function
 * @param codeStrList List of dynamic string where the code is generated to
 * @return Execution status
 */
int generateInputsFunction(dynStrList_t* codeStrList);
