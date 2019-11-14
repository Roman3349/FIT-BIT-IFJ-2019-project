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
#include "../src/stack.h"
}

namespace Tests {

	class StackTest : public ::testing::Test {
	protected:
		void SetUp() override {
			stack = stackInit();
		}

		void TearDown() override {
			stackFree(stack);
		}

		intStack_t *stack;
	};

	TEST_F(StackTest, Init) {
		ASSERT_EQ(stack->head, nullptr);
	}

	TEST_F(StackTest, IsEmpty) {
		ASSERT_TRUE(stackIsEmpty(stack));
		stackPush(stack, 0);
		ASSERT_FALSE(stackIsEmpty(stack));
	}

	TEST_F(StackTest, Push) {
		stackPush(stack, 0);
		ASSERT_EQ(stack->head->value, 0);
		ASSERT_EQ(stack->head->next, nullptr);
		intStack_item_t* item = stack->head;
		stackPush(stack, 1);
		ASSERT_EQ(stack->head->value, 1);
		ASSERT_EQ(stack->head->next, item);
	}

	TEST_F(StackTest, Pop) {
		stackPush(stack, 0);
		intStack_item_t* item = stack->head;
		stackPush(stack, 1);
		intStack_item_t *i;
		int value;
		ASSERT_TRUE(stackPop(stack, &value));
		ASSERT_EQ(value, 1);
		ASSERT_EQ(stack->head, item);
		ASSERT_TRUE(stackPop(stack, &value));
		ASSERT_EQ(value, 0);
		ASSERT_EQ(stack->head, nullptr);
		ASSERT_FALSE(stackPop(stack, &value));
	}

	TEST_F(StackTest, Top) {
		int value;
		ASSERT_FALSE(stackTop(stack, &value));
		stackPush(stack, 0);
		ASSERT_TRUE(stackTop(stack, &value));
		ASSERT_EQ(value, 0);
	}
}
