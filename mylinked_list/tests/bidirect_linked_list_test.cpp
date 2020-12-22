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

TEST_F(BidirectLinkedListTests, init)
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

TEST_F(BidirectLinkedListTests, push_front)
{
	using namespace std;
	// Шпион, следящий за своим удалением

	// Проверка вставки в начало
	{
		BidirectionalList<int> l;
		ASSERT_TRUE(l.empty());
		ASSERT_EQ(l.size(), 0u);

		l.push_front(0);
		l.push_front(1);
		ASSERT_EQ(l.size(), 2);
		ASSERT_TRUE(!l.empty());

		l.clear();
		ASSERT_EQ(l.size(), 0);
		ASSERT_TRUE(l.empty());
	}
}

TEST_F(BidirectLinkedListTests, PushBack)
{
	using namespace std;
	// Шпион, следящий за своим удалением

	// Проверка вставки в начало
	{
		BidirectionalList<int> l;
		ASSERT_TRUE(l.empty());
		ASSERT_EQ(l.size(), 0u);

		l.push_back(1);
		l.push_back(1);
		l.push_back(3);
		ASSERT_EQ(l.size(), 3);
		ASSERT_TRUE(!l.empty());

		l.clear();
		ASSERT_EQ(l.size(), 0);
		ASSERT_TRUE(l.empty());
	}
}

TEST_F(BidirectLinkedListTests, DeletionSpy)
{
	struct DeletionSpy
	{
		DeletionSpy() = default;
		explicit DeletionSpy(int& instance_counter) noexcept : instance_counter_ptr_(&instance_counter) //
		{
			OnAddInstance();
		}
		DeletionSpy(const DeletionSpy& other) noexcept : instance_counter_ptr_(other.instance_counter_ptr_) //
		{
			OnAddInstance();
		}
		DeletionSpy& operator=(const DeletionSpy& rhs) noexcept
		{
			if (this != &rhs)
			{
				auto rhs_copy(rhs);
				std::swap(instance_counter_ptr_, rhs_copy.instance_counter_ptr_);
			}
			return *this;
		}
		~DeletionSpy()
		{
			OnDeleteInstance();
		}

	  private:
		void OnAddInstance() noexcept
		{
			if (instance_counter_ptr_)
			{
				++(*instance_counter_ptr_);
			}
		}
		void OnDeleteInstance() noexcept
		{
			if (instance_counter_ptr_)
			{
				ASSERT_NE(*instance_counter_ptr_, 0);
				--(*instance_counter_ptr_);
			}
		}

		int* instance_counter_ptr_ = nullptr;
	};
	// Проверка фактического удаления элементов
	{
		int item0_counter = 0;
		int item1_counter = 0;
		int item2_counter = 0;
		{
			BidirectionalList<DeletionSpy> list;
			list.push_front(DeletionSpy{item0_counter});
			list.push_front(DeletionSpy{item1_counter});
			list.push_front(DeletionSpy{item2_counter});

			ASSERT_EQ(item0_counter, 1);
			ASSERT_EQ(item1_counter, 1);
			ASSERT_EQ(item2_counter, 1);
			list.clear();
			ASSERT_EQ(item0_counter, 0);
			ASSERT_EQ(item1_counter, 0);
			ASSERT_EQ(item2_counter, 0);

			list.push_front(DeletionSpy{item0_counter});
			list.push_front(DeletionSpy{item1_counter});
			list.push_front(DeletionSpy{item2_counter});
			ASSERT_EQ(item0_counter, 1);
			ASSERT_EQ(item1_counter, 1);
			ASSERT_EQ(item2_counter, 1);
		}
		ASSERT_EQ(item0_counter, 0);
		ASSERT_EQ(item1_counter, 0);
		ASSERT_EQ(item2_counter, 0);
	}
}