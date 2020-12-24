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
		const LinkedList<int> empty_int_list;
		ASSERT_EQ(empty_int_list.size(), 0u);
		ASSERT_TRUE(empty_int_list.empty());
	}

	{
		const LinkedList<string> empty_string_list;
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
		LinkedList<int> l;
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
		LinkedList<int> l;
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
			LinkedList<DeletionSpy> list;
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

TEST_F(BidirectLinkedListTests, Throw)
{
	// Вспомогательный класс, бросающий исключение после создания N-копии
	struct ThrowOnCopy
	{
		ThrowOnCopy() = default;
		explicit ThrowOnCopy(int& copy_counter) noexcept : countdown_ptr(&copy_counter)
		{
		}
		ThrowOnCopy(const ThrowOnCopy& other) : countdown_ptr(other.countdown_ptr) //
		{
			if (countdown_ptr)
			{
				if (*countdown_ptr == 0)
				{
					throw std::bad_alloc();
				}
				else
				{
					--(*countdown_ptr);
				}
			}
		}
		// Присваивание элементов этого типа не требуется
		ThrowOnCopy& operator=(const ThrowOnCopy& rhs) = delete;
		// Адрес счётчика обратного отсчёта. Если не равен nullptr, то уменьшается при каждом копировании.
		// Как только обнулится, конструктор копирования выбросит исключение
		int* countdown_ptr = nullptr;
	};
	// Проверка фактического удаления элементов
	{
		bool exception_was_thrown = false;
		// Последовательно уменьшаем счётчик копирований до нуля, пока не будет выброшено исключение
		for (int max_copy_counter = 5; max_copy_counter >= 0; --max_copy_counter)
		{
			// Создаём непустой список
			LinkedList<ThrowOnCopy> list;
			list.push_front(ThrowOnCopy{});
			try
			{
				int copy_counter = max_copy_counter;
				list.push_front(ThrowOnCopy(copy_counter));
				// Если метод не выбросил исключение, список должен перейти в новое состояние
				ASSERT_EQ(list.size(), 2);
			}
			catch (const std::bad_alloc&)
			{
				exception_was_thrown = true;
				// После выбрасывания исключения состояние списка должно остаться прежним
				ASSERT_EQ(list.size(), 1);
				break;
			}
		}
		ASSERT_TRUE(exception_was_thrown);
	}
}

TEST_F(BidirectLinkedListTests, IteratorsEmpty)
{

	// Итерирование по пустому списку
	{
		LinkedList<int> list;
		// Константная ссылка для доступа к константным версиям begin()/end()
		const auto& const_list = list;

		// Итераторы begine и end у пустого диапазона равны друг другу
		ASSERT_EQ(list.begin(), list.end());
		ASSERT_EQ(const_list.begin(), const_list.end());
		ASSERT_EQ(list.cbegin(), list.cend());
		ASSERT_EQ(list.cbegin(), const_list.begin());
		ASSERT_EQ(list.cend(), const_list.end());
	}
}

TEST_F(BidirectLinkedListTests, IteratorsNonEmpty)
{
	// Итерирование по непустому списку
	{
		LinkedList<int> list;
		const auto& const_list = list;

		list.push_front(1);
		ASSERT_EQ(list.size(), 1u);
		ASSERT_TRUE(!list.empty());

		ASSERT_TRUE(const_list.begin() != const_list.end());
		ASSERT_TRUE(const_list.cbegin() != const_list.cend());
		ASSERT_TRUE(list.begin() != list.end());

		ASSERT_TRUE(const_list.begin() == const_list.cbegin());

		ASSERT_TRUE(*list.cbegin() == 1);
		*list.begin() = -1;
		ASSERT_TRUE(*list.cbegin() == -1);

		const auto old_begin = list.cbegin();
		list.push_front(2);
		ASSERT_EQ(list.size(), 2);

		const auto new_begin = list.cbegin();
		ASSERT_NE(new_begin, old_begin);
		// Проверка прединкремента
		{
			auto new_begin_copy(new_begin);
			ASSERT_EQ((++(new_begin_copy)), old_begin);
		}
		// Проверка постинкремента
		{
			auto new_begin_copy(new_begin);
			ASSERT_EQ(((new_begin_copy)++), new_begin);
			ASSERT_EQ(new_begin_copy, old_begin);
		}
		// Итератор, указывающий на позицию после последнего элемента равен итератору end()
		{
			auto old_begin_copy(old_begin);
			ASSERT_EQ((++old_begin_copy), list.end());
		}
	}
}

TEST_F(BidirectLinkedListTests, IteratorsDecrement)
{
	// Итерирование по непустому списку
	{
		LinkedList<int> list;
		const auto& const_list = list;

		list.push_back(100500);
		list.push_back(1);
		list.push_back(2);
		list.push_back(3);
		ASSERT_EQ(list.size(), 4u);
		ASSERT_TRUE(!list.empty());

		ASSERT_TRUE(const_list.begin() != const_list.end());
		ASSERT_TRUE(const_list.cbegin() != const_list.cend());
		ASSERT_TRUE(list.begin() != list.end());

		ASSERT_TRUE(const_list.begin() == const_list.cbegin());

		ASSERT_EQ(*list.cbegin(), 100500);
		*list.begin() = -1;
		ASSERT_TRUE(*list.cbegin() == -1);
		auto end_list = list.end();
		auto back = --end_list;
		ASSERT_EQ(*(end_list), 3);
	}
}

TEST_F(BidirectLinkedListTests, IteratorsDecrement2)
{
	// Итерирование по непустому списку
	{
		LinkedList<int> list;
		const auto& const_list = list;

		list.push_back(100500);
		list.push_back(1);
		list.push_back(2);
		list.push_back(3);
		list.push_front(100501);
		ASSERT_EQ(*(--(--list.end())), 2);
		ASSERT_EQ(*(list.begin()), 100501);
	}
}

TEST_F(BidirectLinkedListTests, back)
{
	{
		using namespace std;
		LinkedList<int> list;
		list.push_back(1);
		list.push_back(2);
		list.push_back(3);
		list.push_back(4);
		std::stringstream out;
		out << list;
		ASSERT_EQ(out.str(), "{1, 2, 3, 4}"s);
	}
	{
		using namespace std;
		LinkedList<int> list;
		list.push_back(1);
		list.push_back(2);
		list.push_front(-2);
		list.push_front(-1);
		std::stringstream out;
		out << list;
		ASSERT_EQ(out.str(), "{-1, -2, 1, 2}"s);
	}
}

TEST_F(BidirectLinkedListTests, arithmetic)
{
	{
		using namespace std;
		LinkedList<int> list;
		list.push_back(1);
		list.push_back(2);
		list.push_back(3);
		list.push_back(4);
		LinkedList<int>::iterator it = list.begin();
		ASSERT_EQ(*((it + 1) + 1), 3);
		ASSERT_EQ(*((it + 3) - 1), 3);
	}
	{
		LinkedList<int> list;
		list.push_back(1);
		list.push_back(2);
		list.push_back(3);
		list.push_back(4);
		LinkedList<int>::iterator it = list.end();
		ASSERT_EQ(*(it - list.size()), 1);
	}
	{
		using namespace std;
		LinkedList<int> list;
		list.push_back(1);
		list.push_back(2);
		list.push_back(3);
		list.push_back(4);
		list.push_back(5);
		LinkedList<int>::iterator it = list.begin();
		ASSERT_EQ(*(it + 1), 2);
	}
}

TEST_F(BidirectLinkedListTests, distance)
{
	LinkedList<int> list;
	list.push_back(1);
	list.push_back(2);
	list.push_back(3);
	list.push_back(4);
	list.push_back(5);
	ASSERT_EQ(*list.begin(), 1);
	ASSERT_EQ(*(list.begin() + 1), 2);
	ASSERT_EQ(*(list.begin() + 2), 3);
	ASSERT_EQ((list.end() - list.begin()), 5);
	ASSERT_EQ(std::distance(list.begin(), list.end()), 5);
}

TEST_F(BidirectLinkedListTests, operators)
{
	LinkedList<int> list;
	list.push_back(1);
	list.push_back(2);
	list.push_back(3);
	list.push_back(4);
	list.push_back(5);
	ASSERT_EQ(list[0], 1);
	ASSERT_EQ(list[1], 2);
	ASSERT_EQ(list[2], 3);
	list[1] = 100500;
}

TEST_F(BidirectLinkedListTests, initializer_list)
{
	LinkedList<int> list{1, 2, 3, 4, 5, 6};
	ASSERT_EQ(list[0], 1);
	ASSERT_EQ(list[list.size() - 1], 6);
}

TEST_F(BidirectLinkedListTests, initializer_list_strings)
{
	using namespace std;
	LinkedList<std::string> list{"first1"s, "first2"s, "first3"s, "first4"s, "first5"s, "first6"s};
	ASSERT_EQ(*list.begin(), "first1"s);
}

TEST_F(BidirectLinkedListTests, IteratorsOperator)
{
	// Проверка оператора ->
	{
		using namespace std;
		LinkedList<std::string> string_list;

		string_list.push_front("one"s);
		ASSERT_EQ(string_list.cbegin()->length(), 3u);
		string_list.begin()->push_back('!');
		ASSERT_EQ(*string_list.begin(), std::string("one!"));
	}
}

TEST_F(BidirectLinkedListTests, Equals)
{
	LinkedList<int> list1{1, 2, 3, 4, 5, 6};
	LinkedList<int> list1_1{1, 2, 3, 4, 5, 6};
	LinkedList<int> list3{1, 2, 3, 3, 5, 6};
	LinkedList<int> list2{666, 667, 668};
	ASSERT_FALSE(list1 == list2);
	ASSERT_FALSE(list1 == list3);
	ASSERT_TRUE(list1 == list1_1);
	//	std::cout << list2 << list1 << std::endl;
}

TEST_F(BidirectLinkedListTests, IteratorsPolyForm)
{
	// Проверка оператора ->
	// Преобразование итераторов
	{
		LinkedList<int> list;
		list.push_front(1);
		// Конструирование ConstItrator из Iterator
		LinkedList<int>::ConstIterator const_it(list.begin());
		ASSERT_EQ(const_it, list.cbegin());
		ASSERT_EQ(*const_it, *list.cbegin());

		LinkedList<int>::ConstIterator const_it1;
		// Присваивание ConstIterator-у значения Iterator
		const_it1 = list.begin();
		ASSERT_EQ(const_it1, const_it);
	}
}

TEST_F(BidirectLinkedListTests, Equals2)
{
	// Проверка списков на равенство и неравенство
	LinkedList<int> list_1;
	list_1.push_front(1);
	list_1.push_front(2);

	LinkedList<int> list_2;
	list_2.push_front(1);
	list_2.push_front(2);
	list_2.push_front(3);

	LinkedList<int> list_1_copy;
	list_1_copy.push_front(1);
	list_1_copy.push_front(2);

	LinkedList<int> empty_list;
	LinkedList<int> another_empty_list;

	// Список равен самому себе
	ASSERT_TRUE(list_1 == list_1);
	ASSERT_TRUE(empty_list == empty_list);

	// Списки с одинаковым содержимым равны, а с разным - не равны
	ASSERT_TRUE(list_1 == list_1_copy);
	ASSERT_TRUE(list_1 != list_2);
	ASSERT_TRUE(list_2 != list_1);
	ASSERT_TRUE(empty_list == another_empty_list);
}

TEST_F(BidirectLinkedListTests, Swap)
{

	LinkedList<int> list_1{1, 2, 3};
	LinkedList<int> list_2{-1, -2};
	list_1.swap(list_2);
	std::cout << list_1;
}
