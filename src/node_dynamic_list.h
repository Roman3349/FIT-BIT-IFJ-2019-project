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
#include "parser.h"

typedef struct listElement {
    treeElement_t* element;
    struct listElement* next;
} listElement_t;



/**
 * Initializes list
 * @return list
 */
listElement_t* listInit();

/**
 * Adds a new node to the end of list
 * @param list List
 * @param treeElement tree element
 * @return added successfully
 */
bool listAddElement(listElement_t* list, treeElement_t* treeElement);


/**
 * Removes last element from list
 * @param list
 * @return
 */
bool listRemoveLast(listElement_t* list);

/**
 * Frees list
 * @param list list
 */
void listFree(listElement_t* list);
