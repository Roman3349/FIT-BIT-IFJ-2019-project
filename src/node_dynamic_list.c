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

#include "node_dynamic_list.h"

listElement_t* listInit() {
    listElement_t* element = malloc(sizeof(listElement_t));
    if(element == NULL) {
        return NULL;
    }

    element->element = NULL;
    element->next = NULL;
    return element;
}

bool listAddElement(listElement_t* list, treeElement_t* element) {
    if(list == NULL)
        return false;
    while(list->next != NULL) {
        list = list->next;
    }
    list->next = malloc(sizeof(listElement_t));
    list->element = element;
    return list->next != NULL;
}

bool listRemoveLast(listElement_t* list) {
    if(list == NULL)
        return false;

    listElement_t* previous = NULL;
    while(list->next != NULL) {
        previous = list;
        list = list->next;
    }

    free(list);
    if(previous != NULL) {
        previous->next = NULL;
    }

    return true;
}

void listFree(listElement_t * list) {
    while(list != NULL) {
        listElement_t* next = list->next;
        free(list);
        list = next;
    }
}
