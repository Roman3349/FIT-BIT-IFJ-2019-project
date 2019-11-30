/*
 * Copyright (C) 2019 Roman Ondráček <xondra58@stud.fit.vutbr.cz>
 * Copyright (C) 2019 Pavel Raur	 <xraurp00@stud.fit.vutbr.cz>
 * Copyright (C) 2019 Radim Lipka	 <xlipka02@stud.fit.vutbr.cz>
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
#include "../src/dynamic_string.h"
#include "../src/scanner.h"
}

#define ASSERT_TOKEN_FLOAT(file, tokenType, value) \
	do {\
		token_t token = scan(file, stack);\
		ASSERT_EQ(token.type, tokenType);\
		EXPECT_EQ(token.data.floatval, value);\
	} while (false)

#define ASSERT_TOKEN_INTEGER(file, tokenType, value) \
	do {\
		token_t token = scan(file, stack);\
		ASSERT_EQ(token.type, tokenType);\
		EXPECT_EQ(token.data.intval, value);\
	} while (false)

#define ASSERT_TOKEN_STRING(file, tokenType, value) \
	do {\
		token_t token = scan(file, stack);\
		ASSERT_EQ(token.type, tokenType);\
		EXPECT_STREQ(token.data.strval->string, value);\
		dynStrFree(token.data.strval);\
	} while (false)

#define ASSERT_TOKEN(file, tokenType) \
	do {\
		token_t token = scan(file, stack);\
		ASSERT_EQ(token.type, tokenType);\
	} while (false)

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
			stackPush(stack, 0);
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

	TEST_F(ScannerTest, scanErrors) {
		token_t token = scan(nullptr, stack);
		ASSERT_EQ(token.type, T_ERROR);
		FILE* file = std::fopen(__FILE__, "r");
		token = scan(file, nullptr);
		ASSERT_EQ(token.type, T_ERROR);
		intStack_t *intStack = stackInit();
		token = scan(file, intStack);
		ASSERT_EQ(token.type, T_ERROR);
		stackFree(intStack);
	}

	TEST_F(ScannerTest, tokenEOF) {
		FILE* file = openFile("eof.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenEOL) {
		FILE* file = openFile("eol.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenErrIndentation) {
		FILE* file = openFile("errIndentation.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN(file, T_KW_IF);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 1);
		ASSERT_TOKEN(file, T_OP_EQ);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 1);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING, "jejda");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN(file, T_DEDENT);
		ASSERT_TOKEN(file, T_KW_ELSE);
        ASSERT_TOKEN(file, T_EOL);
		//ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN(file, T_UNKNOWN);
		//ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenInt) {
		FILE* file = openFile("int.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 1555);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenBinInt) {
		FILE* file = openFile("binInt.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 37);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 37);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 37);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 37);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenOctInt) {
		FILE* file = openFile("octInt.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 100);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 100);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 100);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 100);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenHexInt) {
		FILE* file = openFile("hexInt.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 65534);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 65534);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 65534);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 65534);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenFloatE) {
		FILE* file = openFile("float_e.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN_FLOAT(file, T_FLOAT, 2e+7);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_FLOAT(file, T_FLOAT, 0e3);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_FLOAT(file, T_FLOAT, 0e-3);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_FLOAT(file, T_FLOAT, 3e0);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_FLOAT(file, T_FLOAT, 3e-0);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_FLOAT(file, T_FLOAT, 3e+0);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenFloatDot) {
		FILE* file = openFile("float_dot.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN_FLOAT(file, T_FLOAT, 1555.37);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_FLOAT(file, T_FLOAT, 0.5);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_FLOAT(file, T_FLOAT, 5.0);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 0);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenError) {
		FILE* file = openFile("errToken.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN(file, T_UNKNOWN);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenExample1) {
		FILE* file = openFile("example1.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING, "Zadejte cislo pro vypocet faktorialu: ");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "inputi");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN(file, T_KW_IF);
		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_OP_LESS);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 0);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING_ML, "\nFaktorial nelze spocitat\n");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_DEDENT);

		ASSERT_TOKEN(file, T_KW_ELSE);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN_STRING(file, T_ID, "vysl");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 1);
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN(file, T_KW_WHILE);
		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_OP_GREATER);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 0);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN_STRING(file, T_ID, "vysl");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "vysl");
		ASSERT_TOKEN(file, T_OP_MUL);
		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_OP_SUB);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 1);
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN(file, T_DEDENT);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING, "Vysledek je:");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_ID, "vysl");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_STRING, "\n");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_DEDENT);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenExample2) {
		FILE* file = openFile("example2.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN(file, T_KW_DEF);
		ASSERT_TOKEN_STRING(file, T_ID, "factorial");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_ID, "n");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN(file, T_KW_IF);
		ASSERT_TOKEN_STRING(file, T_ID, "n");
		ASSERT_TOKEN(file, T_OP_LESS);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 2);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN_STRING(file, T_ID, "result");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 1);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_DEDENT);
		ASSERT_TOKEN(file, T_KW_ELSE);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN_STRING(file, T_ID, "decremented_n");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "n");
		ASSERT_TOKEN(file, T_OP_SUB);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 1);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "temp_result");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "factorial");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_ID, "decremented_n");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "result");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "n");
		ASSERT_TOKEN(file, T_OP_MUL);
		ASSERT_TOKEN_STRING(file, T_ID, "temp_result");
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_DEDENT);
		ASSERT_TOKEN(file, T_KW_RETURN);
		ASSERT_TOKEN_STRING(file, T_ID, "result");
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_DEDENT);

		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING, "Zadejte cislo pro vypocet faktorialu: ");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "inputi");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_KW_IF);
		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_OP_LESS);
		ASSERT_TOKEN_FLOAT(file, T_FLOAT, 0.0);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING, "Faktorial nelze spocitat");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_DEDENT);
		ASSERT_TOKEN(file, T_KW_ELSE);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN_STRING(file, T_ID, "vysl");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "factorial");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING, "Vysledek je:");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_ID, "vysl");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_DEDENT);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenExample3) {
		FILE* file = openFile("example3.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN_STRING(file, T_STRING_ML, " Program 3: Prace s retezci a vestavenymi funkcemi ");
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "s1");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_STRING, "Toto je nejaky text");
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "s2");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "s1");
		ASSERT_TOKEN(file, T_OP_ADD);
		ASSERT_TOKEN_STRING(file, T_STRING, ", ktery jeste trochu obohatime");
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_ID, "s1");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_STRING, "\n");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_ID, "s2");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "s1len");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "len");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_ID, "s1");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "s1len");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "s1len");
		ASSERT_TOKEN(file, T_OP_SUB);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 4);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "s1");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "substr");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_ID, "s2");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_ID, "s1len");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 4);
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "s1len");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "s1len");
		ASSERT_TOKEN(file, T_OP_ADD);
		ASSERT_TOKEN_INTEGER(file, T_NUMBER, 1);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING, "4 znaky od ");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_ID, "s1len");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_STRING, ". znaku v \"");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_ID, "s2");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_STRING, "\":");
		ASSERT_TOKEN(file, T_COMMA);
		ASSERT_TOKEN_STRING(file, T_ID, "s1");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING, "Zadejte serazenou posloupnost vsech malych pismen a-h, ");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING, "pricemz se pismena nesmeji v posloupnosti opakovat: ");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "s1");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "inputs");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);

		ASSERT_TOKEN(file, T_KW_IF);
		ASSERT_TOKEN_STRING(file, T_ID, "s1");
		ASSERT_TOKEN(file, T_OP_NOT_EQ);
		ASSERT_TOKEN(file, T_KW_NONE);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN(file, T_KW_WHILE);
		ASSERT_TOKEN_STRING(file, T_ID, "s1");
		ASSERT_TOKEN(file, T_OP_NOT_EQ);
		ASSERT_TOKEN_STRING(file, T_STRING, "abcdefgh");
		ASSERT_TOKEN(file,T_COLON);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_STRING, "Spatne zadana posloupnost, zkuste znovu: ");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "s1");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_ID, "inputs");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_DEDENT);
		ASSERT_TOKEN(file, T_DEDENT);
		ASSERT_TOKEN(file, T_KW_ELSE);
		ASSERT_TOKEN(file, T_COLON);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_INDENT);
		ASSERT_TOKEN(file, T_KW_PASS);
		ASSERT_TOKEN(file, T_DEDENT);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, escapeSeq) {
		FILE* file = openFile("escapeSeq.ifj19");
		ASSERT_NE(file, nullptr);
		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_ASSIGN);
		ASSERT_TOKEN_STRING(file, T_STRING, "\'\r\n\t12Nn\\\"\\Z");
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN_STRING(file, T_ID, "print");
		ASSERT_TOKEN(file, T_LPAR);
		ASSERT_TOKEN_STRING(file, T_ID, "a");
		ASSERT_TOKEN(file, T_RPAR);
		ASSERT_TOKEN(file, T_EOL);
		ASSERT_TOKEN(file, T_EOF);
	}

	TEST_F(ScannerTest, tokenToString) {
		EXPECT_STREQ(tokenToString(T_EOF), "EOF");
		EXPECT_STREQ(tokenToString(T_NUMBER), "NUMBER");
		EXPECT_STREQ(tokenToString(T_ERROR), "ERROR");
		EXPECT_STREQ(tokenToString((enum token_type) (T_ERROR + 1)), "");
	}

}
