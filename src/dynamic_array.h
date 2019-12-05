/*
 * Copyright (C) 2019 Radim Lipka <xlipka02@stud.fit.vutbr.cz>
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

#include "dynamic_string.h"

typedef struct Elem {                 
        dynStr_t *string;                                        
        struct Elem *lptr;         
        struct Elem *rptr;        
} *ElemPtr;

typedef struct {                                  
    ElemPtr First;                      
    ElemPtr Act;                     
    ElemPtr Last;                    
} List;

void ListInit (List *);
void ListDelete (List *);
void InsertFirstElem (List *, dynStr_t *);
void InsertLastElem(List *, dynStr_t *);
void DeleteFirstElem (List *);
void DeleteLastElem (List *);
void DeletePostElem (List *);
void DeletePreElem (List *);
void InsertPostElem (List *, dynStr_t *);
//TODO void InsertPreElem (List *, dynStr_t *);
void ElemOverwrite (List *, dynStr_t *);