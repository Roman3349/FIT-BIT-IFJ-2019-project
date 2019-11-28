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
	while (string->string[i]) {
		uint32_t high;
		hash = (hash << 4) + string->string[i++];
		high = hash & 0xF0000000;
		if (high) {
			hash ^= high >> 24;
		}
		hash &= ~high;
	}
	return hash % TABLE_SIZE;
}

symTable_t *symTableInit(symTable_t *parent) {
	size_t size = TABLE_SIZE;
	symTable_t *table = malloc(size * sizeof(symTable_t));
	if (table == NULL) {
		return NULL;
	}
	table->allocated = size;
	table->parent = parent;
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
	symTableClear(table);
	free(table);
}

size_t symTableSize(symTable_t *table) {
	if (table == NULL) {
		return 0;
	}
	return table->size;
}

bool symTableInsertEmbedFunctions(symTable_t *table) {
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
		dynStrAppendString(name, names[i]);
		symbolInfo_t info = {.function = {.argc = argc[i], .defined = true}};
		symbol_t *symbol = symbolInit(name, SYMBOL_FUNCTION, info);
		if (symbol == NULL) {
			dynStrFree(name);
			return false;
		}
		if (!symTableInsert(table, symbol, true)) {
			return false;
		}
	}
	return true;
}

bool symTableInsertFunction(symTable_t *table, dynStr_t *name, int argc, bool definition) {
	symTable_t *localTable = symTableInit(table);
	symbolInfo_t info = {.function = {.argc = argc, .defined = true, .table = localTable}};
	symbol_t *symbol = symbolInit(name, SYMBOL_FUNCTION, info);
	if (symbol == NULL) {
		return false;
	}
	return symTableInsert(table, symbol, definition);
}

bool symTableInsertVariable(symTable_t *table, dynStr_t *name) {
	symbolInfo_t info = {.variable = {.assigned = true}};
	symbol_t *symbol = symbolInit(name, SYMBOL_VARIABLE, info);
	if (symbol == NULL) {
		return false;
	}
	return symTableInsert(table, symbol, false);
}

bool symTableInsert(symTable_t *table, symbol_t *symbol, bool unique) {
	if (table == NULL || symbol == NULL) {
		return false;
	}

	size_t index = symTableHash(symbol->name);
	symbol_t *current = table->array[index], *previous = NULL;

	while (current != NULL) {
		if (dynStrEqualString(current->name, symbol->name->string)) {
			if (unique) {
				return false;
			}
			if (current->type != symbol->type) {
				return false;
			}
			if (current->type == SYMBOL_VARIABLE) {
				current->used = true;
				current->info.variable.assigned = true;
			}
			if (current->type == SYMBOL_FUNCTION) {
				if (current->info.function.argc != symbol->info.function.argc) {
					return false;
				}
				current->used = true;
			}
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
	return true;
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
	iterator.index = iterator.table->allocated - 1;
	iterator.symbol = NULL;
	if (table == NULL) {
		return iterator;
	}
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

symbol_t *symbolInit(dynStr_t *name, symbolType_t type, symbolInfo_t info) {
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
	return symbol;
}

void symbolFree(symbol_t *symbol) {
	if (symbol == NULL) {
		return;
	}
	if (symbol->name != NULL) {
		dynStrFree(symbol->name);
		symbol->name = NULL;
	}
	if (symbol->type == SYMBOL_FUNCTION) {
		if (symbol->info.function.table != NULL) {
			symTableFree(symbol->info.function.table);
			symbol->info.function.table = NULL;
		}
	}
	free(symbol);
}
