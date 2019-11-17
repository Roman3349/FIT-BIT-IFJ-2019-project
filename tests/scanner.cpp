/*
 * Copyright (C) 2019 Roman Ondráček <xondra58@stud.fit.vutbr.cz>
 * Copyright (C) 2019 Pavel Raur     <xraurp00@stud.fit.vutbr.cz>
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

#include <unordered_map>

#include "gtest/gtest.h"

extern "C" {
#include "../src/scanner.h"
}

namespace Tests {

	class ScannerTest : public ::testing::Test {
	protected:
		/**
		 * Returns opened file
		 * @param fileName File to open
		 * @return Opened file
		 */
		FILE* openFile(const std::string& fileName) {
			return std::fopen(dataPath.append(fileName).c_str(), "r");
		}
		std::string dataPath = "../../tests/data/";
	};

	TEST_F(ScannerTest, tokenEOF) {
	    intStack_t* stack = stackInit();
		FILE* file = openFile("eof.ifj19");
		ASSERT_NE(file, nullptr);
		token_t token = scan(file, stack);
		ASSERT_EQ(token.type, T_EOF);
		stackFree(stack);
	}

	TEST_F(ScannerTest, tokenEOL) {;
        intStack_t* stack = stackInit();
		FILE* file = openFile("eol.ifj19");
		ASSERT_NE(file, nullptr);
		std::vector<int> tokens = {
			T_EOL,
			T_EOF
		};
		for (int tokenVal: tokens) {
			token_t token = scan(file, stack);
			ASSERT_EQ(token.type, tokenVal);
		}
		stackFree(stack);
	}

	TEST_F(ScannerTest, tokenInt) {
        intStack_t* stack = stackInit();
		FILE* file = openFile("int.ifj19");
		ASSERT_NE(file, nullptr);
		std::unordered_map<int, const char*> tokens = {
			{T_NUM_INT, "1555"},
			{T_EOF, nullptr}
		};
		for (auto t: tokens) {
			token_t token = scan(file, stack);
			ASSERT_EQ(token.type, t.first);
			if (t.second != nullptr) {
				EXPECT_STREQ(token.data.strval->string, t.second);
			}
		}
		stackFree(stack);
	}

	TEST_F(ScannerTest, tokenBinInt) {
        intStack_t* stack = stackInit();
		FILE* file = openFile("binInt.ifj19");
		ASSERT_NE(file, nullptr);
		std::unordered_map<int, const char*> tokens = {
			{T_NUM_BIN, "0b100101"},
			{T_EOF, nullptr}
		};
		for (auto t: tokens) {
			token_t token = scan(file, stack);
			ASSERT_EQ(token.type, t.first);
			if (t.second != nullptr) {
				EXPECT_STREQ(token.data.strval->string, t.second);
			}
		}
		stackFree(stack);
	}

}
