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

int semanticCheck(treeElement_t* parseTree, symTable_t* symTable, int* errCode);

semanticType_t getOperatorType(treeElement_t operatorTree, int* errCode);

semanticType_t checkExpression(treeElement_t* expressionTree, symTable_t* symTable, int* errCode, dynStr_t* context);

void convertBoolToInt(treeElement_t* expressionTree, int* errCode);

void convertIntToFloat(treeElement_t* expressionTree, int* errCode);
