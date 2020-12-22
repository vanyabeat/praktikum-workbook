#include "../src/bidirect_linked_list.h"
#include "helper.h"
#include <gtest/gtest.h>
#include <iostream>
#include <set>

class BidirectLinkedListTests : public ::testing::Test
{
  protected:
	void SetUp()
	{
	}

	void TearDown()
	{
	}
};

TEST_F(BidirectLinkedListTests, Initialize)
{
	using namespace std;
	{
		const BidirectionalList<int> empty_int_list;
		ASSERT_EQ(empty_int_list.size(), 0u);
		ASSERT_TRUE(empty_int_list.empty());
	}

	{
		const BidirectionalList<string> empty_string_list;
		ASSERT_EQ(empty_string_list.size(), 0u);
		ASSERT_TRUE(empty_string_list.empty());
	}
}