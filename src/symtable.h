/*
 * Copyright (C) 2019 Roman Ondráček <xondra58@stud.fit.vutbr.cz>
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

#include <stdbool.h>
#include <stdint.h>

#include "dynamic_string.h"

#define TABLE_SIZE 8191 // 2^13 - 1

typedef struct functionSymbol {
	unsigned argc;
} functionSymbol_t;

typedef union symbolInfo {
	functionSymbol_t function;
} symbolInfo_t;

typedef enum symbolType {
	SYMBOL_UNDEFINED,
	SYMBOL_FUNCTION,
	SYMBOL_VARIABLE
} symbolType_t;

typedef struct symbol symbol_t;

struct symbol {
	dynStr_t *name;
	symbolType_t type;
	symbol_t *next;
};

typedef struct symTable {
	size_t size;
	size_t allocated;
	symbol_t *array[];
} symTable_t;

typedef struct symIterator {
	symbol_t *symbol;
	const symTable_t *table;
	int index;
} symIterator_t;

/**
 * UNIX ELF hash function
 * @param str String to hash
 * @return UNIX ELF hash
 */
uint32_t symTableHash(const char *str);

/**
 * Creates a new symbol table
 * @return
 */
symTable_t *symTableInit();

/**
 * Clears the symbol table
 * @param table Symbol table
 */
void symTableClear(symTable_t *table);

/**
 * Frees the symbol table
 * @param table Symbol table
 */
void symTableFree(symTable_t *table);

/**
 * Inserts a new symbol into the table
 * @param table Symbol table
 * @param name Symbol name
 * @param type Symbol type
 * @return Symbol table iterator
 */
symIterator_t symTableInsert(symTable_t *table, dynStr_t *name, symbolType_t type);

/**
 * Returns size of symbol table
 * @param table Symbol table
 * @return Size of symbol table
 */
size_t symTableSize(symTable_t *table);

/**
 * Returns an iterator to the beginning
 * @param table Symbol table
 * @return Symbol table iterator
 */
symIterator_t symIteratorBegin(const symTable_t *table);

/**
 * Returns an iterator to the end
 * @param table Symbol table
 * @return Symbol table iterator
 */
symIterator_t symIteratorEnd(const symTable_t *table);

/**
 * Returns an iterator to the next symbol
 * @param table Symbol table
 * @return Symbol table iterator
 */
symIterator_t symIteratorNext(symIterator_t iterator);

/**
 * Validates the symbol table iterator
 * @param iterator Symbol table iterator
 * @return Validity of the iterator
 */
bool symIteratorValidate(const symIterator_t *iterator);
