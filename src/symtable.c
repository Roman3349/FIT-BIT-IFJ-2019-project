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

#include "symtable.h"

uint32_t symTableHash(dynStr_t *string) {
	uint32_t hash = 0;
	unsigned long i = 0;
	while (dynStrGetChar(string, i)) {
		uint32_t high;
		hash = (hash << 4) + dynStrGetChar(string, i++);
		high = hash & 0xF0000000;
		if (high) {
			hash ^= high >> 24;
		}
		hash &= ~high;
	}
	return hash % TABLE_SIZE;
}

symTable_t *symTableInit() {
	size_t size = TABLE_SIZE;
	symTable_t *table = malloc(size * sizeof(symTable_t));
	if (table == NULL) {
		return NULL;
	}
	table->allocated = size;
	table->size = 0;
	for (size_t i = 0; i < table->allocated; ++i) {
		table->array[i] = NULL;
	}
	return table;
}

void symTableClear(symTable_t *table) {
	if (table == NULL) {
		return;
	}
	for (size_t i = 0; i < table->allocated; ++i) {
		symbol_t *current = table->array[i];
		symbol_t *next = NULL;
		while (current != NULL) {
			next = current->next;
			symbolFree(current);
			current = next;
		}
	}
}

void symTableFree(symTable_t *table) {
	if (table == NULL) {
		return;
	}
	symTableClear(table);
	free(table);
}

void symTableRemove(symTable_t *table, dynStr_t *name, dynStr_t *context) {
	if (table == NULL || name == NULL) {
		return;
	}

	size_t index = symTableHash(name);
	symbol_t *current = table->array[index], *previous = NULL;

	while (current != NULL) {
		if (dynStrEqual(current->name, name) &&
			dynStrEqual(current->context, context)) {
			if (previous != NULL) {
				previous->next = current->next;
			} else {
				table->array[index] = NULL;
			}
			symbolFree(current);
			table->size--;
			return;
		}
		previous = current;
		current = current->next;
	}
}

size_t symTableSize(symTable_t *table) {
	if (table == NULL) {
		return 0;
	}
	return table->size;
}

symbol_t *symTableFind(symTable_t *table, dynStr_t *name, dynStr_t *context) {
	if (table == NULL || name == NULL) {
		return NULL;
	}

	size_t index = symTableHash(name);
	symbol_t *current = table->array[index];

	while (current != NULL) {
		if (dynStrEqual(current->name, name) &&
		    dynStrEqual(current->context, context)) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}

symbolFrame_t symTableGetFrame(symTable_t *table, dynStr_t *name, dynStr_t *context) {
	if (table == NULL || name == NULL) {
		return FRAME_ERROR;
	}
	symbol_t *symbol = symTableFind(table, name, context);
	if (symbol == NULL) {
		return FRAME_ERROR;
	}
	return symbol->context == NULL ? FRAME_GLOBAL : FRAME_LOCAL;
}

errorCode_t symTableInsertEmbedFunctions(symTable_t *table) {
	if (table == NULL) {
		return false;
	}
	const char* names[EMBEDDED_FUNCTIONS] = {
		"inputs",
		"inputi",
		"inputf",
		"print",
		"len",
		"substr",
		"ord",
		"chr",
	};
	const int argc[EMBEDDED_FUNCTIONS] = {
		0,
		0,
		0,
		-1,
		1,
		3,
		2,
		1,
	};
	for (size_t i = 0; i < EMBEDDED_FUNCTIONS; ++i) {
		dynStr_t *name = dynStrInit();
		if (name == NULL) {
			return ERROR_INTERNAL;
		}
		dynStrAppendString(name, names[i]);
		symbolInfo_t info = {.function = {.argc = argc[i], .defined = true}};
		symbol_t *symbol = symbolInit(name, SYMBOL_FUNCTION, info, NULL);
		if (symbol == NULL) {
			dynStrFree(name);
			return ERROR_INTERNAL;
		}
		if (symTableInsert(table, symbol, true) != ERROR_SUCCESS) {
			symbolFree(symbol);
			return ERROR_INTERNAL;
		}
	}
	return ERROR_SUCCESS;
}

errorCode_t symTableInsertFunction(symTable_t *table, dynStr_t *name, int argc) {
	if (table == NULL || name == NULL) {
		return ERROR_INTERNAL;
	}
	symbolInfo_t info = {.function = {.argc = argc, .argv = NULL, .defined = false}};
	symbol_t *symbol = symbolInit(dynStrClone(name), SYMBOL_FUNCTION, info, NULL);
	if (symbol == NULL) {
		return ERROR_INTERNAL;
	}
	errorCode_t retVal = symTableInsert(table, symbol, false);
	if (retVal != ERROR_SUCCESS) {
		symbolFree(symbol);
	}
	return retVal;
}

errorCode_t symTableInsertFunctionDefinition(symTable_t *table, dynStr_t *name, int argc, dynStrList_t *argv) {
	if (table == NULL || name == NULL || argv == NULL) {
		return ERROR_INTERNAL;
	}
	symbolInfo_t info = {.function = {.argc = argc, .argv = argv, .defined = true}};
	symbol_t *symbol = symbolInit(dynStrClone(name), SYMBOL_FUNCTION, info, NULL);
	if (symbol == NULL) {
		dynStrListFree(argv);
		return ERROR_INTERNAL;
	}
	errorCode_t retVal = symTableInsert(table, symbol, true);
	if (retVal != ERROR_SUCCESS) {
		symbolFree(symbol);
	}
	return retVal;
}

errorCode_t symTableInsertVariable(symTable_t *table, dynStr_t *name, dynStr_t *context) {
	if (table == NULL || name == NULL) {
		return ERROR_INTERNAL;
	}
	symbolInfo_t info = {.variable = {.assigned = true}};
	symbol_t *symbol = symbolInit(dynStrClone(name), SYMBOL_VARIABLE, info, dynStrClone(context));
	if (symbol == NULL) {
		symbolFree(symbol);
		return ERROR_INTERNAL;
	}
	errorCode_t retVal = symTableInsert(table, symbol, false);
	if (retVal != ERROR_SUCCESS) {
		symbolFree(symbol);
	}
	return retVal;
}

errorCode_t symTableInsert(symTable_t *table, symbol_t *symbol, bool unique) {
	if (table == NULL || symbol == NULL) {
		return ERROR_INTERNAL;
	}

	size_t index = symTableHash(symbol->name);
	symbol_t *current = table->array[index], *previous = NULL;

	while (current != NULL) {
		if (dynStrEqual(current->name, symbol->name) &&
			dynStrEqual(current->context, symbol->context)) {
			if (unique && current->type == SYMBOL_FUNCTION &&
				current->info.function.defined) {
				return ERROR_SEMANTIC_FUNCTION;
			}
			if (current->type != symbol->type) {
				return ERROR_SEMANTIC_FUNCTION;
			}
			if (current->type == SYMBOL_VARIABLE) {
				current->used = true;
				current->info.variable.assigned = true;
			}
			if (current->type == SYMBOL_FUNCTION) {
				if (current->info.function.argc != symbol->info.function.argc &&
					current->info.function.argc != -1) {
					return ERROR_SEMANTIC_ARGC;
				}
				current->used = true;
			}
			symbolFree(symbol);
			return ERROR_SUCCESS;
		}
		previous = current;
		current = current->next;
	}

	if (previous == NULL) {
		table->array[index] = symbol;
		table->size++;
	} else {
		previous->next = symbol;
	}
	return ERROR_SUCCESS;
}

dynStr_t *symTableGetArgumentName(symTable_t *table, dynStr_t *function, unsigned long index) {
	if (table == NULL || function == NULL) {
		return NULL;
	}
	symbol_t *symbol = symTableFind(table, function, NULL);
	if (symbol == NULL || symbol->type != SYMBOL_FUNCTION) {
		return NULL;
	}
	functionSymbol_t info = symbol->info.function;
	if (info.argc == -1 || info.argc == 0 ||
		(unsigned long) info.argc < index || info.argv == NULL) {
		return NULL;
	}
	dynStrListEl_t *element = dynStrListFront(info.argv);
	for (unsigned long i = 0; i < index; ++i) {
		element = dynStrListElNext(element);
	}
	if (element == NULL) {
		return NULL;
	}
	return element->string;
}

symIterator_t symIteratorBegin(const symTable_t *table) {
	symIterator_t iterator;
	iterator.table = table;
	iterator.index = 0;
	iterator.symbol = NULL;
	if (table == NULL) {
		return iterator;
	}
	iterator.symbol = table->array[0];
	return iterator;
}

symIterator_t symIteratorEnd(const symTable_t *table) {
	symIterator_t iterator;
	iterator.table = table;
	iterator.index = 0;
	iterator.symbol = NULL;
	if (table == NULL) {
		return iterator;
	}
	iterator.index = iterator.table->allocated - 1;
	iterator.symbol = table->array[table->allocated - 1];
	symbol_t *current = iterator.symbol;
	while (current != NULL) {
		current = current->next;
		iterator.index++;
	}
	return iterator;
}

symIterator_t symIteratorNext(symIterator_t iterator) {
	if (iterator.table == NULL) {
		return iterator;
	}
	if (iterator.symbol != NULL) {
		iterator.symbol = iterator.symbol->next;
	}
	while (iterator.symbol == NULL) {
		if (iterator.index < iterator.table->allocated - 1) {
			iterator.symbol = iterator.table->array[++iterator.index];
		} else {
			return iterator;
		}
	}
	return iterator;
}

bool symIteratorValidate(symIterator_t iterator) {
	return (iterator.table != NULL) && (iterator.symbol != NULL);
}

symbol_t *symbolInit(dynStr_t *name, symbolType_t type, symbolInfo_t info, dynStr_t *context) {
	if (name == NULL) {
		return NULL;
	}
	symbol_t *symbol = malloc(sizeof(symbol_t));
	if (symbol == NULL) {
		return NULL;
	}
	symbol->name = name;
	symbol->info = info;
	symbol->next = NULL;
	symbol->type = type;
	symbol->used = false;
	symbol->context = context;
	return symbol;
}

void symbolFree(symbol_t *symbol) {
	if (symbol == NULL) {
		return;
	}
	if (symbol->type == SYMBOL_FUNCTION) {
		dynStrListFree(symbol->info.function.argv);
	}
	if (symbol->context != NULL) {
		dynStrFree(symbol->context);
		symbol->context = NULL;
	}
	if (symbol->name != NULL) {
		dynStrFree(symbol->name);
		symbol->name = NULL;
	}
	free(symbol);
}
