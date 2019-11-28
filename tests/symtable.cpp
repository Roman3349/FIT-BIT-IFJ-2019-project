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

#include "gtest/gtest.h"

extern "C" {
#include "../src/symtable.h"
}

namespace Tests {

	class SymTableTest : public ::testing::Test {
	protected:
		void SetUp() override {
			table = symTableInit(nullptr);
		}

		void TearDown() override {
			symTableFree(table);
		}

		dynStr_t *createDynStr(std::string str) {
			dynStr_t *string = dynStrInit();
			dynStrAppendString(string, str.c_str());
			return string;
		}

		symbol_t *createFunction(std::string name, int argc, bool defined) {
			symbolInfo_t info = {.function = {.argc = argc, .defined = defined, .table = nullptr}};
			symbol_t *symbol = symbolInit(createDynStr(name), SYMBOL_FUNCTION, info);
			return symbol;
		}

		symTable_t *table = NULL;
	};

	TEST_F(SymTableTest, hash) {
		ASSERT_EQ(symTableHash(createDynStr("main")), 0x737fe % TABLE_SIZE);
	}

	TEST_F(SymTableTest, init) {
		ASSERT_EQ(table->allocated, TABLE_SIZE);
		for (size_t i = 0; i < TABLE_SIZE; i++) {
			ASSERT_EQ(table->array[i], nullptr);
		}
		ASSERT_EQ(table->parent, nullptr);
		ASSERT_EQ(table->size, 0);
	}

	TEST_F(SymTableTest, size) {
		ASSERT_EQ(symTableSize(nullptr), 0);
		ASSERT_EQ(symTableSize(table), 0);
	}

	TEST_F(SymTableTest, insertFunctionDefinitions) {
		ASSERT_TRUE(symTableInsertFunction(table, createDynStr("main"), 0, true));
		ASSERT_EQ(symTableSize(table), 1);
		symIterator_t iterator = symIteratorBegin(table);
		iterator = symIteratorNext(iterator);
		ASSERT_EQ(symTableSize(table), 1);
		ASSERT_EQ(iterator.table, table);
		ASSERT_EQ(iterator.index, 0x737fe % TABLE_SIZE);
		ASSERT_NE(iterator.symbol, nullptr);
		ASSERT_STREQ(iterator.symbol->name->string, "main");
		ASSERT_EQ(iterator.symbol->next, nullptr);
		ASSERT_EQ(iterator.symbol->type, SYMBOL_FUNCTION);
		ASSERT_EQ(iterator.symbol->info.function.argc, 0);
		ASSERT_TRUE(iterator.symbol->info.function.defined);
		ASSERT_FALSE(symTableInsertFunction(table, createDynStr("main"), 0, true));
		ASSERT_TRUE(symTableInsertFunction(table, createDynStr("main"), 0, false));
		ASSERT_FALSE(symTableInsertFunction(table, createDynStr("main"), 1, false));
		ASSERT_FALSE(symTableInsertVariable(table, createDynStr("main")));
	}

	TEST_F(SymTableTest, insertVariable) {
		ASSERT_TRUE(symTableInsertVariable(table, createDynStr("i")));
		ASSERT_EQ(symTableSize(table), 1);
		symIterator_t iterator = symIteratorBegin(table);
		iterator = symIteratorNext(iterator);
		ASSERT_EQ(symTableSize(table), 1);
		ASSERT_EQ(iterator.table, table);
		ASSERT_EQ(iterator.index, 105);
		ASSERT_NE(iterator.symbol, nullptr);
		ASSERT_STREQ(iterator.symbol->name->string, "i");
		ASSERT_EQ(iterator.symbol->next, nullptr);
		ASSERT_EQ(iterator.symbol->type, SYMBOL_VARIABLE);
		ASSERT_TRUE(iterator.symbol->info.variable.assigned);
		ASSERT_TRUE(symTableInsertVariable(table, createDynStr("i")));
		ASSERT_FALSE(symTableInsertFunction(table, createDynStr("i"), 0, false));
	}

	TEST_F(SymTableTest, insert) {
		symbol_t *symbol = createFunction("main", 0, true);
		ASSERT_TRUE(symTableInsert(table, symbol, true));
		ASSERT_EQ(symTableSize(table), 1);
		symIterator_t iterator = symIteratorBegin(table);
		iterator = symIteratorNext(iterator);
		ASSERT_EQ(symTableSize(table), 1);
		ASSERT_EQ(iterator.table, table);
		ASSERT_EQ(iterator.index, 0x737fe % TABLE_SIZE);
		ASSERT_NE(iterator.symbol, nullptr);
		ASSERT_STREQ(iterator.symbol->name->string, "main");
		ASSERT_EQ(iterator.symbol->next, nullptr);
		ASSERT_EQ(iterator.symbol->type, SYMBOL_FUNCTION);
	}

	TEST_F(SymTableTest, iteratorBegin) {
		symIterator_t iterator = symIteratorBegin(table);
		ASSERT_EQ(iterator.table, table);
		ASSERT_EQ(iterator.index, 0);
		ASSERT_EQ(iterator.symbol, nullptr);
	}

	TEST_F(SymTableTest, iteratorNextEmpty) {
		symIterator_t iterator = symIteratorBegin(table);
		iterator = symIteratorNext(iterator);
		ASSERT_EQ(iterator.table, table);
		ASSERT_EQ(iterator.index, TABLE_SIZE - 1);
		ASSERT_EQ(iterator.symbol, nullptr);
	}

	TEST_F(SymTableTest, iteratorNextNonEmpty) {
		symbol_t *symbol = createFunction("main", 0, true);
		symTableInsert(table, symbol, true);
		symIterator_t iterator = symIteratorBegin(table);
		iterator = symIteratorNext(iterator);
		ASSERT_EQ(iterator.table, table);
		ASSERT_EQ(iterator.index, 0x737fe % TABLE_SIZE);
		ASSERT_EQ(iterator.symbol, symbol);
		iterator = symIteratorNext(iterator);
		ASSERT_EQ(iterator.table, table);
		ASSERT_EQ(iterator.index, TABLE_SIZE - 1);
		ASSERT_EQ(iterator.symbol, nullptr);
	}

	TEST_F(SymTableTest, iteratorNextInvalid) {
		symIterator_t iterator = {.symbol = nullptr, .table = nullptr, .index = 0};
		iterator = symIteratorNext(iterator);
		ASSERT_EQ(iterator.table, nullptr);
		ASSERT_EQ(iterator.index, 0);
		ASSERT_EQ(iterator.symbol, nullptr);
	}

	TEST_F(SymTableTest, iteratorEnd) {
		symIterator_t iterator = symIteratorEnd(table);
		ASSERT_EQ(iterator.table, table);
		ASSERT_EQ(iterator.index, TABLE_SIZE - 1);
		ASSERT_EQ(iterator.symbol, nullptr);
	}

	TEST_F(SymTableTest, iteratorValidate) {
		symIterator_t iterator = {.symbol = nullptr, .table = nullptr, .index = 0};
		ASSERT_FALSE(symIteratorValidate(iterator));
		iterator.table = table;
		ASSERT_FALSE(symIteratorValidate(iterator));
		iterator.symbol = createFunction("main", 0, true);
		ASSERT_TRUE(symIteratorValidate(iterator));
	}

	TEST_F(SymTableTest, symbolInit) {
		symbolInfo_t info = {.function = {.argc = 0, .defined = true, .table = nullptr}};
		ASSERT_EQ(symbolInit(nullptr, SYMBOL_FUNCTION, info), nullptr);
	}

}
