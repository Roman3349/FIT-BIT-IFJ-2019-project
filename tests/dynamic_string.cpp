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
#include "../src/dynamic_string.h"
}

namespace Tests {

	class DynamicStringTest : public ::testing::Test {
	protected:
		void SetUp() override {
			string = dynStrInit();
		}
		void TearDown() override {
			dynStrFree(string);
		}
		dynStr_t *string;
	};

	TEST_F(DynamicStringTest, Init) {
		ASSERT_STREQ(string->string, "");
		ASSERT_EQ(string->size, 0);
		ASSERT_EQ(string->alloc_size, DYN_STR_LENGTH);
	}

	TEST_F(DynamicStringTest, Clear) {
		dynStrAppendChar(string, 'A');
		dynStrClear(string);
		ASSERT_STREQ(string->string, "");
		ASSERT_EQ(string->size, 0);
		ASSERT_EQ(string->alloc_size, DYN_STR_LENGTH);
	}

	TEST_F(DynamicStringTest, AppendChar) {
		dynStrAppendChar(string, 'A');
		for (char i = 0; i < 10; ++i) {
			dynStrAppendChar(string, '0' + i);
		}
		ASSERT_STREQ(string->string, "A0123456789");
		ASSERT_EQ(string->size, 11);
		ASSERT_EQ(string->alloc_size, 2 * DYN_STR_LENGTH);
	}

	TEST_F(DynamicStringTest, AppendString) {
		bool retVal = dynStrAppendString(string, "ABCD0123456789");
		unsigned size = 14;
		ASSERT_TRUE(retVal);
		ASSERT_STREQ(string->string, "ABCD0123456789");
		ASSERT_EQ(string->size, size);
		ASSERT_EQ(string->alloc_size, 2 * DYN_STR_LENGTH + size);
	}

	TEST_F(DynamicStringTest, EqualString) {
		dynStrAppendString(string, "ABCD");
		ASSERT_TRUE(dynStrEqualString(string, "ABCD"));
		ASSERT_FALSE(dynStrEqualString(string, "ABCDE"));
	}

	TEST_F(DynamicStringTest, IsEmpty) {
		ASSERT_TRUE(dynStrIsEmpty(string));
		dynStrAppendString(string, "ABCD");
		ASSERT_FALSE(dynStrIsEmpty(string));
		dynStrClear(string);
		ASSERT_TRUE(dynStrIsEmpty(string));
	}
}
