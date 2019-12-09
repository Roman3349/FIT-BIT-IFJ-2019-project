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
#include "dynamic_string_list.h"
#include "error.h"

#define EMBEDDED_FUNCTIONS 8
#define TABLE_SIZE 8191 // 2^13 - 1

typedef struct symbol symbol_t;
typedef struct symTable symTable_t;

typedef struct functionSymbol {
	int argc;
	dynStrList_t *argv;
	bool defined;
} functionSymbol_t;

typedef struct variableSymbol {
	bool assigned;
} variableSymbol_t;

typedef enum symbolFrame {
	FRAME_GLOBAL,
	FRAME_LOCAL,
	FRAME_TEMP,
	FRAME_ERROR
} symbolFrame_t;

typedef union symbolInfo {
	functionSymbol_t function;
	variableSymbol_t variable;
} symbolInfo_t;

typedef enum symbolType {
	SYMBOL_FUNCTION,
	SYMBOL_VARIABLE
} symbolType_t;

struct symbol {
	dynStr_t *name;
	symbolType_t type;
	symbolInfo_t info;
	dynStr_t *context;
	bool used;
	symbol_t *next;
};

struct symTable {
	size_t size;
	size_t allocated;
	symbol_t *array[];
};

typedef struct symIterator {
	symbol_t *symbol;
	const symTable_t *table;
	size_t index;
} symIterator_t;

/**
 * UNIX ELF hash function
 * @param string String to hash
 * @return UNIX ELF hash
 */
uint32_t symTableHash(dynStr_t *string);

/**
 * Creates a new symbol table
 * @return Initializes a new symbol table
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
 * Removes a symbol from the symbol table
 * @param table Symbol table
 * @param name Symbol name
 * @param context Symbol context (NULL = global, others = function name)
 */
void symTableRemove(symTable_t *table, dynStr_t *name, dynStr_t *context);

/**
 * Inserts the embedded functions
 * @param table Symbol table
 * @return Execution status
 */
errorCode_t symTableInsertEmbedFunctions(symTable_t *table);

/**
 * Inserts a function
 * @param table Symbol table
 * @param name Function name
 * @param argc Argument count
 * @return Execution status
 */
errorCode_t symTableInsertFunction(symTable_t *table, dynStr_t *name, int argc);

/**
 * Inserts a function definition
 * @param table Symbol table
 * @param name Function name
 * @param argc Argument count
 * @param argv Argument list
 * @return Execution status
 */
errorCode_t symTableInsertFunctionDefinition(symTable_t *table, dynStr_t *name, int argc, dynStrList_t *argv);

/**
 * Inserts a variable
 * @param table Symbol table
 * @param name Variable name
 * @param context Symbol context (NULL = global, others = function name)
 * @param assigment Is assigment?
 * @return Execution status
 */
errorCode_t symTableInsertVariable(symTable_t *table, dynStr_t *name, dynStr_t *context, bool assigment);

/**
 * Inserts a symbol into the table
 * @param table Symbol table
 * @param symbol Symbol to insert
 * @param unique Unique insert?
 * @return Execution status
 */
errorCode_t symTableInsert(symTable_t *table, symbol_t *symbol, bool unique);

/**
 * Returns size of symbol table
 * @param table Symbol table
 * @return Size of symbol table
 */
size_t symTableSize(symTable_t *table);

/**
 * Returns a symbol
 * @param table Symbol table
 * @param name Symbol name
 * @param context Symbol context
 * @return Symbol
 */
symbol_t *symTableFind(symTable_t *table, dynStr_t *name, dynStr_t *context);

/**
 * Checks if the variable is assigned
 * @param table Symbol table
 * @param name Symbol name
 * @param context Symbol context
 * @return Is the variable assigned?
 */
bool symTableIsVariableAssigned(symTable_t *table, dynStr_t *name, dynStr_t *context);

/**
 * Returns the argument name of the function
 * @param table Symbol table
 * @param function Function name
 * @param index Argument index
 * @return Function's argument name
 */
dynStr_t *symTableGetArgumentName(symTable_t *table, dynStr_t *function, unsigned long index);

/**
 * Returns the symbol frame type
 * @param table Symbol table
 * @param name Symbol name
 * @param context Symbol context (NULL = global, others = function name)
 * @return Symbol frame type
 */
symbolFrame_t symTableGetFrame(symTable_t *table, dynStr_t *name, dynStr_t *context);

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
bool symIteratorValidate(symIterator_t iterator);

/**
 * Initializes the symbol
 * @param name Symbol name
 * @param type Symbol type
 * @param info Symbol info
 * @param context Symbol context
 * @return Initialized symbol
 */
symbol_t *symbolInit(dynStr_t *name, symbolType_t type, symbolInfo_t info, dynStr_t *context);

/**
 * Frees the symbol
 * @param symbol Symbol to free
 */
void symbolFree(symbol_t* symbol);
