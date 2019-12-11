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

#pragma once

#include "parse_tree.h"
#include "symtable.h"
#include "scanner.h"

enum semanticType{
	SEMANTIC_INT,
	SEMANTIC_STRING,
	SEMANTIC_FLOAT,
	SEMANTIC_BOOL,
	SEMANTIC_VARIABLE,
	SEMANTIC_NONE,
	SEMANTIC_EXPRESSION,
	SEMANTIC_UNKNOWN
};

typedef enum semanticType semanticType_t;


/**
 * Check semantic in tree element
 * @param parseTree parse tree to check
 * @param symTable symbol table
 * @param errCode error code
 */
void semanticCheck(treeElement_t* parseTree, symTable_t* symTable, int* errCode);

/**
 * Returns semantic type for expression operators
 * @param operatorTree operator tree element
 * @param errCode error code
 * @return semantic type
 */
semanticType_t getOperatorType(treeElement_t operatorTree, int* errCode);

/**
 * Checks semantic for expression
 * @param expressionTree expression tree
 * @param symTable symbol table
 * @param errCode error code
 * @param context expression context
 * @return expression result type
 */
semanticType_t checkExpression(treeElement_t* expressionTree, symTable_t* symTable, int* errCode, dynStr_t* context);

/**
 * Converts tree boolean values to integer
 * @param expressionTree tree to convert
 * @param errCode error code
 */
void convertBoolToInt(treeElement_t* expressionTree, int* errCode);


/**
 * Converts tree integer values to float
 * @param expressionTree tree to convert
 * @param errCode error code
 */
void convertIntToFloat(treeElement_t* expressionTree, int* errCode);

/**
 * Helper function for recursive semantic check
 * @param element tree element
 * @param symtable symbol table
 * @param errCode error code
 * @param context context
 */
void semanticCheckTree(treeElement_t* element, symTable_t* symtable, int* errCode, dynStr_t* context);
