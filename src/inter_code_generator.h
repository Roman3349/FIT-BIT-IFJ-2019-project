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

enum frameType {
    GF, // global frame
    LF, // local frame
    TF  // temporary frame
};

/*
 * Process body of the program
 */
int processCode(treeElement_t codeElement);

int processEToken(treeElement_t eTokenElement);
int processFunctionDefinition(treeElement_t defElement);
int processCodeBlock(treeElement_t codeBlockElement);
int processExpression(treeElement_t expElement);
int processBinaryOperation(treeElement_t operationElement);
int processUnaryOperation(treeElement_t operationElement);
int processIf(treeElement_t ifElement);
int processElse(treeElement_t elseElement);
int processAssign(treeElement_t assignElement);
