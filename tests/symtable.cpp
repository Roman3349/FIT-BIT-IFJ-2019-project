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

		symTable_t *table = NULL;
	};

	TEST_F(SymTableTest, hash) {
		const char* name = "main";
		ASSERT_EQ(symTableHash(name), 0x737fe % TABLE_SIZE);
	}

	TEST_F(SymTableTest, size) {
		ASSERT_EQ(symTableSize(table), 0);
	}

	TEST_F(SymTableTest, insert) {
		dynStr_t *name = dynStrInit();
		dynStrAppendString(name, "main");
		symIterator_t iterator = symTableInsert(table, name, SYMBOL_FUNCTION);
		ASSERT_EQ(symTableSize(table), 1);
		ASSERT_EQ(iterator.table, table);
		ASSERT_EQ(iterator.index, 0);
		ASSERT_EQ(iterator.table, table);
		ASSERT_NE(iterator.symbol, nullptr);
		ASSERT_STREQ(iterator.symbol->name->string, "main");
		ASSERT_EQ(iterator.symbol->next, nullptr);
		ASSERT_EQ(iterator.symbol->type, SYMBOL_FUNCTION);
	}


}
