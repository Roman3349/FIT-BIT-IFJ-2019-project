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

#include "dynamic_array.h"

void ListInit (List *L) {
    L->First = NULL;
    L->Act = NULL;
    L->Last = NULL;
}

void ListDelete (List *L) {
    ElemPtr Elem = L->First;
    while (L->First != NULL){
      L->First = L->First->rptr;
      free(Elem);
      Elem = L->First;
    }
    L->First = NULL;
    L->Act = NULL;
    L->Last = NULL;
}

void InsertFirstElem (List *L, dynStr_t *dynString) {
    ElemPtr Elem = (ElemPtr) malloc(sizeof(struct Elem));
    if (Elem == NULL){
      return;
    }
    Elem->string = dynString;
    Elem->lptr= NULL;
    Elem->rptr= L->First;

    if (L->First != NULL){
        L->First->lptr = Elem; 
    }
    else {
        L->Last = Elem; 
    }
    L->First = Elem; 

}

void InsertLastElem(DList *L, dynStr_t *dynString) {
    ElemPtr Elem = (ElemPtr) malloc(sizeof(struct Elem));
    if (Elem == 0){
      return;
    }
    Elem->string = dynString;
    Elem->lptr= L->Last;
    Elem->rptr= NULL;

    if (L->Last != NULL){
        L->Last->rptr = Elem; 
    }
    else {
        L->First = Elem; 
    }
    L->Last = Elem;
}

void DeleteFirstElem (List *L) {
    ElemPtr LElem;
    if (L->First != NULL){
        Elem = L->First;
        if (L->Act == L->First){ 
            L->Act = NULL; 
        }
        if (L->First == L->Last){
            L->First = NULL;
            L->Last = NULL;
        }
        else {
            L->First = L->First->rptr;
            L->First->lptr = NULL; 
        }
        free(Elem);
    }
}

void DeleteLastElem (List *L) {
    ElemPtr Elem;
    if (L->First != NULL){
        Elem = L->First;
        if (L->Act == L->First){ 
            L->Act = NULL;
        }
        if (L->First == L->Last){
            L->First = NULL;
            L->Last = NULL;
        }
        else {
            L->First = L->First->rptr;
            L->First->lptr = NULL;
        }
        free(Elem);
    }
}

void InsertPostElem (List *L, dynStr_t *dynString) {
    if (L->Act != NULL){
        ElemPtr Elem = (ElemPtr) malloc(sizeof(struct Elem));
        if (Elem == NULL){
            return;
        }
        Elem->string = dynString;
        Elem->rptr = L->Act->rptr;
        Elem->lptr = L->Act;
        L->Act->rptr = Elem;
        if (L->Act == L->Last){
            L->Last = Elem;
        }
        else {
            Elem->rptr->lptr = Elem;
        }
    }
}

void DeletePostElem (List *L) {
    if(L->Act != NULL){
       if(L->Act->rptr != NULL){
          ElemPtr Elem;
          Elem = L->Act->rptr; 
          L->Act->rptr = Elem->rptr;
          if (Elem == L->Last){
              L->Last = L->Act;
          }
          else {
              Elem->rptr->lptr = L->Act;
          }
          free(Elem);
       }
    }
}

void DeletePreElem (List *L) {
    if(L->Act != NULL){
       if(L->Act->lptr != NULL){
          ElemPtr Elem;
          Elem = L->Act->lptr; 
          L->Act->lptr = Elem->lptr;
          if (Elem == L->First){ 
              L->Last = L->Act;
          }
          else {
              Elem->lptr->rptr = L->Act;
          }
          free(Elem);
       }
    }
}

void DLActualize (List *L, dynStr_t *dynString) {
    if (L->Act != NULL){
        L->Act->string = dynString;
    }
}