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
			table = symTableInit();
		}

		void TearDown() override {
			symTableFree(table);
		}

		dynStr_t *createDynStr(const std::string &str) {
			dynStr_t *string = dynStrInit();
			dynStrAppendString(string, str.c_str());
			return string;
		}

		symbol_t *createFunction(const std::string &name, int argc, bool defined) {
			symbolInfo_t info = {.function = {.argc = argc, .defined = defined}};
			symbol_t *symbol = symbolInit(createDynStr(name), SYMBOL_FUNCTION, info, nullptr);
			return symbol;
		}

		symTable_t *table = NULL;
	};

	TEST_F(SymTableTest, hash) {
		dynStr_t *name = createDynStr("main");
		ASSERT_EQ(symTableHash(name), 0x737fe % TABLE_SIZE);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, init) {
		ASSERT_EQ(table->allocated, TABLE_SIZE);
		for (size_t i = 0; i < TABLE_SIZE; i++) {
			ASSERT_EQ(table->array[i], nullptr);
		}
		ASSERT_EQ(table->size, 0);
	}

	TEST_F(SymTableTest, size) {
		ASSERT_EQ(symTableSize(nullptr), 0);
		ASSERT_EQ(symTableSize(table), 0);
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

	TEST_F(SymTableTest, insertEmbedFunctions) {
		ASSERT_TRUE(symTableInsertEmbedFunctions(table));
		ASSERT_EQ(symTableSize(table), EMBEDDED_FUNCTIONS);
	}

	TEST_F(SymTableTest, insertFunction) {
		dynStr_t *name = createDynStr("main");
		ASSERT_TRUE(symTableInsertFunction(table, name, 0, true));
		ASSERT_EQ(symTableSize(table), 1);
		symIterator_t iterator = symIteratorBegin(table);
		iterator = symIteratorNext(iterator);
		ASSERT_EQ(symTableSize(table), 1);
		ASSERT_EQ(iterator.table, table);
		ASSERT_EQ(iterator.index, 0x737fe % TABLE_SIZE);
		ASSERT_NE(iterator.symbol, nullptr);
		ASSERT_STREQ(iterator.symbol->name->string, name->string);
		ASSERT_STREQ(iterator.symbol->name->string, "main");
		ASSERT_EQ(iterator.symbol->next, nullptr);
		ASSERT_EQ(iterator.symbol->type, SYMBOL_FUNCTION);
		ASSERT_EQ(iterator.symbol->info.function.argc, 0);
		ASSERT_TRUE(iterator.symbol->info.function.defined);
		ASSERT_TRUE(symTableInsertFunction(table, name, 0, false));
		ASSERT_FALSE(symTableInsertFunction(table, name, 0, true));
		ASSERT_FALSE(symTableInsertFunction(table, name, 1, false));
		ASSERT_FALSE(symTableInsertVariable(table, name, nullptr));
		dynStrFree(name);
	}

	TEST_F(SymTableTest, insertVariable) {
		dynStr_t *name = createDynStr("i");
		ASSERT_TRUE(symTableInsertVariable(table, name, nullptr));
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
		ASSERT_TRUE(symTableInsertVariable(table, name, nullptr));
		ASSERT_FALSE(symTableInsertFunction(table, name, 0, false));
		dynStrFree(name);
	}

	TEST_F(SymTableTest, find) {
		symbol_t *symbol = createFunction("main", 0, true);
		ASSERT_TRUE(symTableInsert(table, symbol, true));
		dynStr_t *name = createDynStr("main");
		symbol = symTableFind(table, name, nullptr);
		ASSERT_NE(symbol, nullptr);
		ASSERT_STREQ(symbol->name->string, "main");
		ASSERT_EQ(symbol->next, nullptr);
		ASSERT_EQ(symbol->type, SYMBOL_FUNCTION);
		symbol = symTableFind(table, name, name);
		ASSERT_EQ(symbol, nullptr);
		dynStrFree(name);
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

	TEST_F(SymTableTest, iteratorValidateNullTableAndSymbol) {
		symIterator_t iterator = {.symbol = nullptr, .table = nullptr, .index = 0};
		ASSERT_FALSE(symIteratorValidate(iterator));
	}

	TEST_F(SymTableTest, iteratorValidateNullSymbol) {
		symIterator_t iterator = {.symbol = nullptr, .table = table, .index = 0};
		ASSERT_FALSE(symIteratorValidate(iterator));
	}

	TEST_F(SymTableTest, iteratorValidateNullTable) {
		symbol_t *symbol = createFunction("main", 0, true);
		symIterator_t iterator = {.symbol = nullptr, .table = nullptr, .index = 0};
		ASSERT_FALSE(symIteratorValidate(iterator));
		symbolFree(symbol);
	}

	TEST_F(SymTableTest, iteratorValidate) {
		symbol_t *symbol = createFunction("main", 0, true);
		symIterator_t iterator = {.symbol = symbol, .table = table, .index = 0};
		ASSERT_TRUE(symIteratorValidate(iterator));
		symbolFree(symbol);
	}

	TEST_F(SymTableTest, symbolInitNull) {
		symbolInfo_t info = {.function = {.argc = 0, .defined = true}};
		ASSERT_EQ(symbolInit(nullptr, SYMBOL_FUNCTION, info, nullptr), nullptr);
	}

	TEST_F(SymTableTest, symbolInit) {
		dynStr_t *name = createDynStr("main");
		symbolInfo_t info = {.function = {.argc = 0, .defined = true}};
		symbol_t *symbol = symbolInit(name, SYMBOL_FUNCTION, info, nullptr);
		ASSERT_NE(symbol, nullptr);
		ASSERT_EQ(symbol->context, nullptr);
		ASSERT_EQ(symbol->info.function.argc, info.function.argc);
		ASSERT_EQ(symbol->info.function.defined, info.function.defined);
		ASSERT_EQ(symbol->name, name);
		ASSERT_EQ(symbol->next, nullptr);
		ASSERT_EQ(symbol->type, SYMBOL_FUNCTION);
		ASSERT_FALSE(symbol->used);
		symbolFree(symbol);
	}

}
