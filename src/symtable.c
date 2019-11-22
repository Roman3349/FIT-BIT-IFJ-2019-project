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

uint32_t symTableHash(const char *str) {
	uint32_t hash = 0, high;
	while (*str) {
		hash = (hash << 4) + *str++;
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
	for (size_t i = 0; i < table->size; ++i) {
		symbol_t *current = table->array[i];
		symbol_t *next = NULL;
		while (current != NULL) {
			next = current->next;
			dynStrFree(current->name);
			free(current);
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

symIterator_t symTableInsert(symTable_t *table, dynStr_t *name, symbolType_t type) {
	symIterator_t iterator;
	iterator.table = table;
	iterator.symbol = NULL;
	iterator.index = 0;
	if (table == NULL || name == NULL) {
		return iterator;
	}

	char *nameStr = name->string;
	size_t index = symTableHash(nameStr);
	symbol_t *current = table->array[index], *previous = NULL;

	while (current != NULL) {
		if (dynStrEqualString(current->name, nameStr)) {
			if (current->type == SYMBOL_UNDEFINED) {
				current->type = type;
			}
			return iterator;
		}
		previous = current;
		current = current->next;
		iterator.index++;
	}

	symbol_t *newSymbol = malloc(sizeof(symbol_t));
	if (newSymbol == NULL) {
		return symIteratorEnd(table);
	}
	newSymbol->name = name;
	newSymbol->type = type;
	iterator.symbol = newSymbol;

	if (previous == NULL) {
		table->array[index] = newSymbol;
		table->size++;
	} else {
		previous->next = newSymbol;
	}
	return iterator;
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
	iterator.symbol = table->array[table->allocated - 1];
	symbol_t *current = iterator.symbol;
	while (current != NULL) {
		current = current->next;
		iterator.index++;
	}
	return iterator;
}

symIterator_t symIteratorNext(symIterator_t iterator) {
	if (symIteratorValidate(&iterator)) {
		return iterator;
	}
	iterator.symbol = iterator.symbol->next;
	return iterator;
}

bool symIteratorValidate(const symIterator_t *iterator) {
	return (iterator->table != NULL) && (iterator->symbol != NULL);
}
