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

/*
 * Process body of the program
 * @param codeElement tree element containing the program code
 * @return execution status
 */
int processCode(treeElement_t codeElement);

/*
 * Process element with token
 * @param eTokenElement tree element with token
 * @param context pointer to pointer to the function context
 *                is used to return function name
 * @return execution status
 */
int processEToken(treeElement_t eTokenElement, dynStr_t** context);

/*
 * Process definition of new function
 * @param defElement tree element with function definition
 * @returns execution status
 */
int processFunctionDefinition(treeElement_t defElement);

/*
 * Process block of code
 * @param codeBlockElement tree element containing block of code to process
 * @param context context function name to determine if vars are local or global
 * @return execution status
 */
int processCodeBlock(treeElement_t codeBlockElement, dynStr_t* context);

/*
 * Process expression
 * @param expElement tree element with expression
 * @return execution status
 */
int processExpression(treeElement_t expElement);

/*
 * Process operation with 2 operands
 * @param operationElement tree element containing operation to process
 * @return execution status
 */
int processBinaryOperation(treeElement_t operationElement);

/*
 * Process operation operation with only 1 operand
 * @param operationElement tree element with operation to process
 * @return execution status
 */
int processUnaryOperation(treeElement_t operationElement);

/*
 * Process if statement
 * @param ifElement tree element containing if statement
 * @returns execution status
 */
int processIf(treeElement_t ifElement);

/*
 * Process else statement
 * @param elseElement tree element with else code block to process
 * @return execution status
 */
int processElse(treeElement_t elseElement);

/*
 * Process assignment of variable
 * @param assignElement element with assign expression
 * @return execution status
 */
int processAssign(treeElement_t assignElement);

/*
 * Process function definition params
 * @param defParamsElement tree element with function params used in definition
 * @return execution status
 */
int processFunctionDefParams(treeElement_t defParamsElement);

/*
 * Create temporary frame and process function call
 * @param callElement tree element containing function call
 * @return execution status
 */
int processFunctionCall(treeElement_t callElement);

/*
 * Process function params in temporary frame
 * @param callParamsElement tree element containing called function parameters
 * @return execution status
 */
int processFunctionCallParams(treeElement_t callParamsElement);
