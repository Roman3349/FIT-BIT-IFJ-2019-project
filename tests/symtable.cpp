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
#include "symtable.h"
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

		symbol_t *createFunction(const std::string &name, int argc, bool defined, bool insert = false, dynStrList_t *argv = nullptr) {
			if (argv == nullptr) {
				argv = dynStrListInit();
			}
			symbolInfo_t info = {.function = {.argc = argc, .argv = argv, .defined = defined}};
			symbol_t *symbol = symbolInit(createDynStr(name), SYMBOL_FUNCTION, info, nullptr);
			if (insert) {
				symTableInsert(table, symbol, defined);
			}
			return symbol;
		}

		symbol_t *createVariable(const std::string &name, const std::string &context, bool insert = false) {
			symbolInfo_t info = {.variable = {.assigned = true}};
			dynStr_t *sName = createDynStr(name);
			dynStr_t *sContext = createDynStr(context);
			symbol_t *symbol = symbolInit(sName, SYMBOL_VARIABLE, info, sContext);
			if (insert) {
				symTableInsert(table, symbol, false);
			}
			return symbol;
		}

		symTable_t *table = nullptr;
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

	TEST_F(SymTableTest, insertNullSymbol) {
		ASSERT_EQ(symTableInsert(table, nullptr, true), ERROR_INTERNAL);
	}

	TEST_F(SymTableTest, insertNullTable) {
		symbol_t *symbol = createFunction("main", 0, true);
		ASSERT_EQ(symTableInsert(nullptr, symbol, true), ERROR_INTERNAL);
		symbolFree(symbol);
	}

	TEST_F(SymTableTest, insert) {
		symbol_t *symbol = createFunction("main", 0, true);
		ASSERT_EQ(symTableInsert(table, symbol, true), ERROR_SUCCESS);
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

	TEST_F(SymTableTest, insertEmbedFunctionsNullTable) {
		ASSERT_FALSE(symTableInsertEmbedFunctions(nullptr));
	}

	TEST_F(SymTableTest, insertEmbedFunctions) {
		ASSERT_EQ(symTableInsertEmbedFunctions(table), ERROR_SUCCESS);
		ASSERT_EQ(symTableSize(table), EMBEDDED_FUNCTIONS);
	}

	TEST_F(SymTableTest, insertEmbedFunctionsDuplicate) {
		ASSERT_EQ(symTableInsertEmbedFunctions(table), ERROR_SUCCESS);
		ASSERT_EQ(symTableSize(table), EMBEDDED_FUNCTIONS);
		ASSERT_EQ(symTableInsertEmbedFunctions(table), ERROR_INTERNAL);
		ASSERT_EQ(symTableSize(table), EMBEDDED_FUNCTIONS);
	}

	TEST_F(SymTableTest, insertFunctionDefinitionNullName) {
		ASSERT_EQ(symTableInsertFunctionDefinition(table, nullptr, 0, nullptr), ERROR_INTERNAL);
	}

	TEST_F(SymTableTest, insertFunctionNullName) {
		ASSERT_EQ(symTableInsertFunction(table, nullptr, 0), ERROR_INTERNAL);
	}

	TEST_F(SymTableTest, insertFunctionDefinitionNullTable) {
		dynStr_t *name = createDynStr("main");
		ASSERT_EQ(symTableInsertFunctionDefinition(nullptr, name, 0, nullptr), ERROR_INTERNAL);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, insertFunctionNullTable) {
		dynStr_t *name = createDynStr("main");
		ASSERT_EQ(symTableInsertFunction(nullptr, name, 0), ERROR_INTERNAL);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, insertFunction) {
		dynStr_t *name = createDynStr("main");
		ASSERT_EQ(symTableInsertFunctionDefinition(table, name, 0, dynStrListInit()), ERROR_SUCCESS);
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
		ASSERT_EQ(symTableInsertFunction(table, name, 0), ERROR_SUCCESS);
		ASSERT_EQ(symTableInsertFunctionDefinition(table, name, 0, dynStrListInit()), ERROR_SEMANTIC_FUNCTION);
		ASSERT_EQ(symTableInsertFunction(table, name, 1), ERROR_SEMANTIC_ARGC);
		ASSERT_EQ(symTableInsertVariable(table, name, nullptr, false), ERROR_SEMANTIC_FUNCTION);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, insertFunctionDynamicArgc) {
		dynStr_t *name = createDynStr("main");
		ASSERT_EQ(symTableInsertFunctionDefinition(table, name, -1, dynStrListInit()), ERROR_SUCCESS);
		ASSERT_EQ(symTableSize(table), 1);
		ASSERT_EQ(symTableInsertFunction(table, name, 2), ERROR_SUCCESS);
		ASSERT_EQ(symTableSize(table), 1);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, insertVariableNullName) {
		ASSERT_EQ(symTableInsertVariable(table, nullptr, nullptr, false), ERROR_INTERNAL);
	}

	TEST_F(SymTableTest, insertVariableNullTable) {
		dynStr_t *name = createDynStr("i");
		ASSERT_EQ(symTableInsertVariable(nullptr, name, nullptr, false), ERROR_INTERNAL);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, insertVariable) {
		dynStr_t *name = createDynStr("i");
		ASSERT_EQ(symTableInsertVariable(table, name, nullptr, false), ERROR_SUCCESS);
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
		ASSERT_FALSE(iterator.symbol->info.variable.assigned);
		ASSERT_EQ(symTableInsertVariable(table, name, nullptr, false), ERROR_SUCCESS);
		ASSERT_EQ(symTableInsertFunction(table, name, 0), ERROR_SEMANTIC_FUNCTION);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, findNullName) {
		ASSERT_EQ(symTableFind(table, nullptr, nullptr), nullptr);
	}

	TEST_F(SymTableTest, findNullTable) {
		dynStr_t *name = createDynStr("main");
		ASSERT_EQ(symTableFind(nullptr, nullptr, name), nullptr);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, find) {
		createFunction("main", 0, true, true);
		dynStr_t *name = createDynStr("main");
		symbol_t *symbol = symTableFind(table, name, nullptr);
		ASSERT_NE(symbol, nullptr);
		ASSERT_STREQ(symbol->name->string, "main");
		ASSERT_EQ(symbol->next, nullptr);
		ASSERT_EQ(symbol->type, SYMBOL_FUNCTION);
		symbol = symTableFind(table, name, name);
		ASSERT_EQ(symbol, nullptr);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, removeNullName) {
		createFunction("main", 0, true, true);
		ASSERT_EQ(symTableSize(table), 1);
		symTableRemove(table, nullptr, nullptr);
		ASSERT_EQ(symTableSize(table), 1);
	}

	TEST_F(SymTableTest, removeNullTable) {
		dynStr_t *name = createDynStr("i");
		symTableRemove(nullptr, nullptr, name);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, removeNullContext) {
		dynStr_t *name = createDynStr("main");
		createFunction("main", 0, true, true);
		ASSERT_EQ(symTableSize(table), 1);
		symTableRemove(table, name, nullptr);
		ASSERT_EQ(symTableSize(table), 0);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, getArgumentName) {
		dynStrList_t *args = dynStrListInit();
		ASSERT_TRUE(dynStrListPushBack(args, createDynStr("a")));
		ASSERT_TRUE(dynStrListPushBack(args, createDynStr("b")));
		createFunction("f", 2, true, true, args);
		dynStr_t *function = createDynStr("f");
		dynStr_t *argument = symTableGetArgumentName(table, function, 0);
		ASSERT_NE(argument, nullptr);
		ASSERT_STREQ(dynStrGetString(argument), "a");
		argument = symTableGetArgumentName(table, function, 1);
		ASSERT_NE(argument, nullptr);
		ASSERT_STREQ(dynStrGetString(argument), "b");
		argument = symTableGetArgumentName(table, function, 2);
		ASSERT_EQ(argument, nullptr);
	}

	TEST_F(SymTableTest, isVariableAssignedNull) {
		ASSERT_FALSE(symTableIsVariableAssigned(nullptr, nullptr, nullptr));
		ASSERT_FALSE(symTableIsVariableAssigned(table, nullptr, nullptr));
		dynStr_t *name = createDynStr("a");
		ASSERT_FALSE(symTableIsVariableAssigned(nullptr, name, nullptr));
		dynStrFree(name);
	}

	TEST_F(SymTableTest, isVariableAssigned0) {
		createFunction("main", 0, true, true);
		dynStr_t *function = createDynStr("main");
		dynStr_t *varName = createDynStr("a");
		symTableInsertVariable(table, varName, function, false);
		ASSERT_FALSE(symTableIsVariableAssigned(table, varName, function));
		dynStrFree(function);
		dynStrFree(varName);
	}

	TEST_F(SymTableTest, isVariableAssigned1) {
		createFunction("main", 0, true, true);
		dynStr_t *function = createDynStr("main");
		dynStr_t *varName = createDynStr("a");
		symTableInsertVariable(table, varName, nullptr, false);
		symTableInsertVariable(table, varName, function, false);
		ASSERT_FALSE(symTableIsVariableAssigned(table, varName, function));
		dynStrFree(function);
		dynStrFree(varName);
	}

	TEST_F(SymTableTest, isVariableAssigned2) {
		createFunction("main", 0, true, true);
		dynStr_t *function = createDynStr("main");
		dynStr_t *varName = createDynStr("a");
		symTableInsertVariable(table, varName, nullptr, true);
		symTableInsertVariable(table, varName, function, false);
		ASSERT_FALSE(symTableIsVariableAssigned(table, varName, function));
		dynStrFree(function);
		dynStrFree(varName);
	}

	TEST_F(SymTableTest, isVariableAssigned3) {
		createFunction("main", 0, true, true);
		dynStr_t *function = createDynStr("main");
		dynStr_t *varName = createDynStr("a");
		symTableInsertVariable(table, varName, nullptr, true);
		ASSERT_TRUE(symTableIsVariableAssigned(table, varName, function));
		dynStrFree(function);
		dynStrFree(varName);
	}

	TEST_F(SymTableTest, isVariableAssigned4) {
		createFunction("main", 0, true, true);
		dynStr_t *function = createDynStr("main");
		dynStr_t *varName = createDynStr("a");
		symTableInsertVariable(table, varName, function, true);
		ASSERT_TRUE(symTableIsVariableAssigned(table, varName, function));
		dynStrFree(function);
		dynStrFree(varName);
	}

	TEST_F(SymTableTest, getFrameNull) {
		dynStr_t *name = createDynStr("main");
		ASSERT_EQ(symTableGetFrame(nullptr, nullptr, nullptr), FRAME_ERROR);
		ASSERT_EQ(symTableGetFrame(nullptr, nullptr, name), FRAME_ERROR);
		ASSERT_EQ(symTableGetFrame(table, nullptr, nullptr), FRAME_ERROR);
		dynStrFree(name);
	}

	TEST_F(SymTableTest, getFrame) {
		createFunction("main", 0, true, true);
		dynStr_t *name = createDynStr("main");
		dynStr_t *varName = createDynStr("a");
		symTableInsertVariable(table, varName, name, false);
		ASSERT_EQ(symTableGetFrame(table, name, nullptr), FRAME_GLOBAL);
		ASSERT_EQ(symTableGetFrame(table, varName, name), FRAME_LOCAL);
		ASSERT_EQ(symTableGetFrame(table, varName, nullptr), FRAME_ERROR);
		dynStrFree(name);
		dynStrFree(varName);
	}

	TEST_F(SymTableTest, iteratorBeginNullTable) {
		symIterator_t iterator = symIteratorBegin(nullptr);
		ASSERT_EQ(iterator.table, nullptr);
		ASSERT_EQ(iterator.index, 0);
		ASSERT_EQ(iterator.symbol, nullptr);
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
		symbol_t *symbol = createFunction("main", 0, true, true);
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

	TEST_F(SymTableTest, iteratorEndNullTable) {
		symIterator_t iterator = symIteratorEnd(nullptr);
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
		symbolInfo_t info = {.function = {.argc = 0, .argv = nullptr,.defined = true}};
		ASSERT_EQ(symbolInit(nullptr, SYMBOL_FUNCTION, info, nullptr), nullptr);
	}

	TEST_F(SymTableTest, symbolInit) {
		dynStr_t *name = createDynStr("main");
		symbolInfo_t info = {.function = {.argc = 0, .argv = nullptr, .defined = true}};
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
