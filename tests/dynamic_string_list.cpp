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
#include "dynamic_string.h"
#include "dynamic_string_list.h"
}

namespace Tests {

	class DynamicStringListTest : public ::testing::Test {
	protected:
		void SetUp() override {
			list = dynStrListInit();
		}

		void TearDown() override {
			dynStrListFree(list);
		}

		dynStr_t *createDynStr(const std::string &str) {
			dynStr_t *string = dynStrInit();
			dynStrAppendString(string, str.c_str());
			return string;
		}

		dynStrList_t *list;
	};

	TEST_F(DynamicStringListTest, Init) {
		ASSERT_EQ(list->head, nullptr);
		ASSERT_EQ(list->tail, nullptr);
	}

	TEST_F(DynamicStringListTest, PushFrontNull) {
		ASSERT_FALSE(dynStrListPushFront(nullptr, nullptr));
		ASSERT_FALSE(dynStrListPushFront(list, nullptr));
		ASSERT_EQ(list->head, nullptr);
		ASSERT_EQ(list->tail, nullptr);
	}

	TEST_F(DynamicStringListTest, PushFront) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		ASSERT_EQ(list->head, list->tail);
		ASSERT_EQ(list->head->next, nullptr);
		ASSERT_EQ(list->head->prev, nullptr);
		ASSERT_STREQ(dynStrGetString(list->head->string), "B");
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		ASSERT_NE(list->head, list->tail);
		ASSERT_EQ(list->head->next, list->tail);
		ASSERT_EQ(list->tail->prev, list->head);
		ASSERT_STREQ(dynStrGetString(list->head->string), "A");
		ASSERT_STREQ(dynStrGetString(list->tail->string), "B");
	}

	TEST_F(DynamicStringListTest, PushBackNull) {
		ASSERT_FALSE(dynStrListPushBack(nullptr, nullptr));
		ASSERT_FALSE(dynStrListPushBack(list, nullptr));
		ASSERT_EQ(list->head, nullptr);
		ASSERT_EQ(list->tail, nullptr);
	}

	TEST_F(DynamicStringListTest, PushBack) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushBack(list, string));
		ASSERT_EQ(list->head, list->tail);
		ASSERT_EQ(list->head->next, nullptr);
		ASSERT_EQ(list->head->prev, nullptr);
		ASSERT_STREQ(dynStrGetString(list->head->string), "B");
		string = createDynStr("C");
		ASSERT_TRUE(dynStrListPushBack(list, string));
		ASSERT_NE(list->head, list->tail);
		ASSERT_EQ(list->head->next, list->tail);
		ASSERT_EQ(list->tail->prev, list->head);
		ASSERT_STREQ(dynStrGetString(list->head->string), "B");
		ASSERT_STREQ(dynStrGetString(list->tail->string), "C");
	}

	TEST_F(DynamicStringListTest, PopFrontEmpty) {
		dynStrListPopFront(list);
		ASSERT_EQ(list->head, nullptr);
		ASSERT_EQ(list->tail, nullptr);
	}

	TEST_F(DynamicStringListTest, PopFront) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		dynStrListPopFront(list);
		ASSERT_EQ(list->head, list->tail);
		ASSERT_STREQ(dynStrGetString(list->head->string), "B");
	}

	TEST_F(DynamicStringListTest, PopBackEmpty) {
		dynStrListPopBack(list);
		ASSERT_EQ(list->head, nullptr);
		ASSERT_EQ(list->tail, nullptr);
	}

	TEST_F(DynamicStringListTest, PopBack) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		dynStrListPopBack(list);
		ASSERT_EQ(list->head, list->tail);
		ASSERT_STREQ(dynStrGetString(list->head->string), "A");
	}

	TEST_F(DynamicStringListTest, IsEmpty) {
		ASSERT_TRUE(dynStrListIsEmpty(nullptr));
		ASSERT_TRUE(dynStrListIsEmpty(list));
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushBack(list, string));
		ASSERT_FALSE(dynStrListIsEmpty(list));
	}

	TEST_F(DynamicStringListTest, Size) {
		ASSERT_EQ(dynStrListSize(list), 0);
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		ASSERT_EQ(dynStrListSize(list), 1);
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		ASSERT_EQ(dynStrListSize(list), 2);
	}

	TEST_F(DynamicStringListTest, FrontNull) {
		ASSERT_EQ(dynStrListFront(nullptr), nullptr);
	}

	TEST_F(DynamicStringListTest, FrontEmpty) {
		ASSERT_EQ(dynStrListFront(list), nullptr);
	}

	TEST_F(DynamicStringListTest, Front) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		dynStrListEl_t *element = dynStrListFront(list);
		ASSERT_EQ(element, list->head);
		ASSERT_STREQ(dynStrGetString(element->string), "A");
	}

	TEST_F(DynamicStringListTest, PrevNull) {
		ASSERT_EQ(dynStrListElPrev(nullptr), nullptr);
	}

	TEST_F(DynamicStringListTest, PrevFront) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		dynStrListEl_t *element = dynStrListElPrev(list->head);
		ASSERT_EQ(element, nullptr);
	}

	TEST_F(DynamicStringListTest, Prev) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		dynStrListEl_t *element = dynStrListElPrev(list->tail);
		ASSERT_EQ(element, list->head);
		ASSERT_STREQ(dynStrGetString(element->string), "A");
	}

	TEST_F(DynamicStringListTest, NextNull) {
		ASSERT_EQ(dynStrListElNext(nullptr), nullptr);
	}

	TEST_F(DynamicStringListTest, NextBack) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		dynStrListEl_t *element = dynStrListElNext(list->tail);
		ASSERT_EQ(element, nullptr);
	}

	TEST_F(DynamicStringListTest, Next) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		dynStrListEl_t *element = dynStrListElNext(list->head);
		ASSERT_EQ(element, list->tail);
		ASSERT_STREQ(dynStrGetString(element->string), "B");
	}

	TEST_F(DynamicStringListTest, BackNull) {
		ASSERT_EQ(dynStrListBack(nullptr), nullptr);
	}

	TEST_F(DynamicStringListTest, BackEmpty) {
		ASSERT_EQ(dynStrListBack(list), nullptr);
	}

	TEST_F(DynamicStringListTest, Back) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		dynStrListEl_t *element = dynStrListBack(list);
		ASSERT_EQ(element, list->tail);
		ASSERT_STREQ(dynStrGetString(element->string), "B");
	}

	TEST_F(DynamicStringListTest, RemoveNull) {
		dynStrListRemove(nullptr, nullptr);
		dynStrListRemove(list, nullptr);
		ASSERT_EQ(list->head, nullptr);
		ASSERT_EQ(list->tail, nullptr);
	}

	TEST_F(DynamicStringListTest, Remove1) {
		dynStr_t *string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		dynStrListEl_t *element = dynStrListBack(list);
		dynStrListRemove(list, element);
		ASSERT_EQ(list->head, nullptr);
		ASSERT_EQ(list->tail, nullptr);
	}

	TEST_F(DynamicStringListTest, Remove2) {
		dynStr_t *string = createDynStr("B");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		string = createDynStr("A");
		ASSERT_TRUE(dynStrListPushFront(list, string));
		dynStrListEl_t *element = dynStrListBack(list);
		dynStrListRemove(list, element);
		ASSERT_EQ(list->head, list->tail);
		ASSERT_STREQ(dynStrGetString(list->head->string), "A");
	}

	TEST_F(DynamicStringListTest, InitElementNull) {
		dynStrListEl_t *element = dynStrListElInit(nullptr);
		ASSERT_EQ(element, nullptr);
	}

	TEST_F(DynamicStringListTest, InitElement) {
		dynStr_t *string = createDynStr("A");
		dynStrListEl_t *element = dynStrListElInit(string);
		ASSERT_NE(element, nullptr);
		ASSERT_EQ(element->prev, nullptr);
		ASSERT_EQ(element->next, nullptr);
		ASSERT_STREQ(dynStrGetString(element->string), "A");
		dynStrListElFree(element);
	}

	TEST_F(DynamicStringListTest, ElementGet) {
		dynStr_t *string = createDynStr("A");
		dynStrListEl_t *element = dynStrListElInit(string);
		ASSERT_STREQ(dynStrGetString(dynStrListElGet(element)), "A");
		dynStrListElFree(element);
	}

}
