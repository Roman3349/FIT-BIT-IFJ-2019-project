/*
 * Copyright (C) 2019 Roman Ondráček <xondra58@stud.fit.vutbr.cz>
 * Copyright (C) 2019 Pavel Raur	 <xraurp00@stud.fit.vutbr.cz>
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

#define ASSERT_TOKEN(file, tokenType, value) \
	do {\
		token_t token = scan(file, stack);\
		ASSERT_EQ(token.type, tokenType);\
		switch (tokenType) {\
			case T_NUMBER:\
				EXPECT_EQ(token.data.intval, value);\
				break;\
			default:\
				;\
		}\
	} while (false);

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

		/**
		 * Sets up the test environment
		 */
		void SetUp() override {
			stack = stackInit();
		}

		/**
		 * Tear down the test environment
		 */
		void TearDown() override {
			stackFree(stack);
		}
		std::string dataPath = "../../tests/data/";
		intStack_t* stack;
	};

	TEST_F(ScannerTest, tokenEOF) {
		FILE* file = openFile("eof.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN(file, T_EOF, NULL);
	}

	TEST_F(ScannerTest, tokenEOL) {;
		FILE* file = openFile("eol.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN(file, T_EOF, NULL);
	}

	TEST_F(ScannerTest, tokenInt) {
		FILE* file = openFile("int.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN(file, T_NUMBER, 1555);
		ASSERT_TOKEN(file, T_EOL, NULL);
		ASSERT_TOKEN(file, T_EOF, NULL);
	}

	TEST_F(ScannerTest, tokenBinInt) {
		FILE* file = openFile("binInt.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN(file, T_NUMBER, 37);
		ASSERT_TOKEN(file, T_EOL, NULL);
		ASSERT_TOKEN(file, T_EOF, NULL);
	}

}
