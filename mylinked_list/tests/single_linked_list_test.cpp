#include "../src/single_linked_list.h"
#include "helper.h"
#include <gtest/gtest.h>
#include <iostream>
#include <set>
#include <algorithm>

class SingleLinkedListTests : public ::testing::Test
{
  protected:
	void SetUp()
	{
	}

	void TearDown()
	{
	}

  public:
	template <typename T> std::vector<T> CreateUniqMergedVector(std::vector<T>& a, std::vector<T>& b)
	{
		std::vector<T> vec(a);
		vec.insert(vec.end(), b.begin(), b.end());
		std::set<T> res(vec.begin(), vec.end());
		return std::vector<T>(res.begin(), res.end());
	}

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
				assert(*instance_counter_ptr_ != 0);
				--(*instance_counter_ptr_);
			}
		}

		int* instance_counter_ptr_ = nullptr;
	};

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
};

TEST_F(SingleLinkedListTests, Initialize)
{
	using namespace std;
	{
		const SingleLinkedList<int> empty_int_list;
		ASSERT_EQ(empty_int_list.GetSize(), 0u);
		ASSERT_TRUE(empty_int_list.IsEmpty());
	}

	{
		const SingleLinkedList<string> empty_string_list;
		ASSERT_EQ(empty_string_list.GetSize(), 0u);
		ASSERT_TRUE(empty_string_list.IsEmpty());
	}
}

TEST_F(SingleLinkedListTests, Begin)
{
	using namespace std;
	// Шпион, следящий за своим удалением

	// Проверка вставки в начало
	{
		SingleLinkedList<int> l;
		ASSERT_TRUE(l.IsEmpty());
		ASSERT_EQ(l.GetSize(), 0u);

		l.PushFront(0);
		l.PushFront(1);
		ASSERT_EQ(l.GetSize(), 2);
		ASSERT_TRUE(!l.IsEmpty());

		l.Clear();
		ASSERT_EQ(l.GetSize(), 0);
		ASSERT_TRUE(l.IsEmpty());
	}
}

TEST_F(SingleLinkedListTests, Spy)
{
	// Проверка фактического удаления элементов
	{
		int item0_counter = 0;
		int item1_counter = 0;
		int item2_counter = 0;
		{
			SingleLinkedList<DeletionSpy> list;
			list.PushFront(DeletionSpy{item0_counter});
			list.PushFront(DeletionSpy{item1_counter});
			list.PushFront(DeletionSpy{item2_counter});

			ASSERT_EQ(item0_counter, 1);
			ASSERT_EQ(item1_counter, 1);
			ASSERT_EQ(item2_counter, 1);
			list.Clear();
			ASSERT_EQ(item0_counter, 0);
			ASSERT_EQ(item1_counter, 0);
			ASSERT_EQ(item2_counter, 0);

			list.PushFront(DeletionSpy{item0_counter});
			list.PushFront(DeletionSpy{item1_counter});
			list.PushFront(DeletionSpy{item2_counter});
			ASSERT_EQ(item0_counter, 1);
			ASSERT_EQ(item1_counter, 1);
			ASSERT_EQ(item2_counter, 1);
		}
		ASSERT_EQ(item0_counter, 0);
		ASSERT_EQ(item1_counter, 0);
		ASSERT_EQ(item2_counter, 0);
	}
}

TEST_F(SingleLinkedListTests, Throw)
{
	// Проверка фактического удаления элементов
	{
		bool exception_was_thrown = false;
		// Последовательно уменьшаем счётчик копирований до нуля, пока не будет выброшено исключение
		for (int max_copy_counter = 5; max_copy_counter >= 0; --max_copy_counter)
		{
			// Создаём непустой список
			SingleLinkedList<ThrowOnCopy> list;
			list.PushFront(ThrowOnCopy{});
			try
			{
				int copy_counter = max_copy_counter;
				list.PushFront(ThrowOnCopy(copy_counter));
				// Если метод не выбросил исключение, список должен перейти в новое состояние
				ASSERT_EQ(list.GetSize(), 2);
			}
			catch (const std::bad_alloc&)
			{
				exception_was_thrown = true;
				// После выбрасывания исключения состояние списка должно остаться прежним
				ASSERT_EQ(list.GetSize(), 1);
				break;
			}
		}
		ASSERT_TRUE(exception_was_thrown);
	}
}

TEST_F(SingleLinkedListTests, IteratorsEmpty)
{

	// Итерирование по пустому списку
	{
		SingleLinkedList<int> list;
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

TEST_F(SingleLinkedListTests, IteratorsNonEmpty)
{
	// Итерирование по непустому списку
	{
		SingleLinkedList<int> list;
		const auto& const_list = list;

		list.PushFront(1);
		ASSERT_EQ(list.GetSize(), 1u);
		ASSERT_TRUE(!list.IsEmpty());

		ASSERT_TRUE(const_list.begin() != const_list.end());
		ASSERT_TRUE(const_list.cbegin() != const_list.cend());
		ASSERT_TRUE(list.begin() != list.end());

		ASSERT_TRUE(const_list.begin() == const_list.cbegin());

		ASSERT_TRUE(*list.cbegin() == 1);
		*list.begin() = -1;
		ASSERT_TRUE(*list.cbegin() == -1);

		const auto old_begin = list.cbegin();
		list.PushFront(2);
		ASSERT_EQ(list.GetSize(), 2);

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

TEST_F(SingleLinkedListTests, IteratorsOperator)
{
	// Проверка оператора ->
	{
		using namespace std;
		SingleLinkedList<std::string> string_list;

		string_list.PushFront("one"s);
		ASSERT_EQ(string_list.cbegin()->length(), 3u);
		string_list.begin()->push_back('!');
		ASSERT_EQ(*string_list.begin(), std::string("one!"));
	}
}

TEST_F(SingleLinkedListTests, IteratorsPolyForm)
{
	// Проверка оператора ->
	// Преобразование итераторов
	{
		SingleLinkedList<int> list;
		list.PushFront(1);
		// Конструирование ConstItrator из Iterator
		SingleLinkedList<int>::ConstIterator const_it(list.begin());
		ASSERT_EQ(const_it, list.cbegin());
		ASSERT_EQ(*const_it, *list.cbegin());

		SingleLinkedList<int>::ConstIterator const_it1;
		// Присваивание ConstIterator-у значения Iterator
		const_it1 = list.begin();
		ASSERT_EQ(const_it1, const_it);
	}
}

TEST_F(SingleLinkedListTests, Equals)
{
	// Проверка списков на равенство и неравенство
	SingleLinkedList<int> list_1;
	list_1.PushFront(1);
	list_1.PushFront(2);

	SingleLinkedList<int> list_2;
	list_2.PushFront(1);
	list_2.PushFront(2);
	list_2.PushFront(3);

	SingleLinkedList<int> list_1_copy;
	list_1_copy.PushFront(1);
	list_1_copy.PushFront(2);

	SingleLinkedList<int> empty_list;
	SingleLinkedList<int> another_empty_list;

	// Список равен самому себе
	ASSERT_TRUE(list_1 == list_1);
	ASSERT_TRUE(empty_list == empty_list);

	// Списки с одинаковым содержимым равны, а с разным - не равны
	ASSERT_TRUE(list_1 == list_1_copy);
	ASSERT_TRUE(list_1 != list_2);
	ASSERT_TRUE(list_2 != list_1);
	ASSERT_TRUE(empty_list == another_empty_list);
}

TEST_F(SingleLinkedListTests, Compare)
{
	// Лексикографическое сравнение списков
	{

		ASSERT_TRUE((SingleLinkedList<int>{1, 2, 3} < SingleLinkedList<int>{1, 2, 3, 1}));
		ASSERT_FALSE((SingleLinkedList<int>{3, 2, 1} < SingleLinkedList<int>{1, 2, 3, 1}));
		ASSERT_FALSE((SingleLinkedList<int>{1, 2, 3} > SingleLinkedList<int>{1, 2, 3, 1}));
		ASSERT_TRUE((SingleLinkedList<int>{1, 2, 3} <= SingleLinkedList<int>{1, 2, 3}));
		ASSERT_TRUE((SingleLinkedList<int>{1, 2, 4} > SingleLinkedList<int>{1, 2, 3}));
		ASSERT_TRUE((SingleLinkedList<int>{1, 2, 3} >= SingleLinkedList<int>{1, 2, 3}));
	}
}

TEST_F(SingleLinkedListTests, Swap)
{
	// Обмен содержимого списков

	SingleLinkedList<int> first;
	first.PushFront(1);
	first.PushFront(2);

	SingleLinkedList<int> second;
	second.PushFront(10);
	second.PushFront(11);
	second.PushFront(15);

	const auto old_first_begin = first.begin();
	const auto old_second_begin = second.begin();
	const auto old_first_size = first.GetSize();
	const auto old_second_size = second.GetSize();

	first.swap(second);

	ASSERT_EQ(second.begin(), old_first_begin);
	ASSERT_EQ(first.begin(), old_second_begin);
	ASSERT_EQ(second.GetSize(), old_first_size);
	ASSERT_EQ(first.GetSize(), old_second_size);

	// Обмен при помощи функции swap
	{
		using std::swap;

		// В отсутствие пользовательской перегрузки будет вызвана функция std::swap, которая
		// выполнит обмен через создание временной копии
		swap(first, second);

		// Убеждаемся, что используется не std::swap, а пользовательская перегрузка

		// Если бы обмен был выполнен с созданием временной копии,
		// то итератор first.begin() не будет равен ранее сохранённому значению,
		// так как копия будет хранить свои узлы по иным адресам
		ASSERT_EQ(first.begin(), old_first_begin);
		ASSERT_EQ(second.begin(), old_second_begin);
		ASSERT_EQ(first.GetSize(), old_first_size);
		ASSERT_EQ(second.GetSize(), old_second_size);
	}
}

TEST_F(SingleLinkedListTests, Initializer_lists)
{
	// Инициализация списка при помощи std::initializer_list
	{
		SingleLinkedList<int> list{1, 2, 3, 4, 5};
		assert(list.GetSize() == 5);
		assert(!list.IsEmpty());
		assert(std::equal(list.begin(), list.end(), std::begin({1, 2, 3, 4, 5})));
	}
}

TEST_F(SingleLinkedListTests, Copy)
{
	// Копирование списков
	{
		const SingleLinkedList<int> empty_list{};
		// Копирование пустого списка
		{
			auto list_copy(empty_list);
			ASSERT_TRUE(list_copy.IsEmpty());
		}

		SingleLinkedList<int> non_empty_list{1, 2, 3, 4};
		// Копирование непустого списка
		{
			auto list_copy(non_empty_list);

			ASSERT_TRUE(non_empty_list.begin() != list_copy.begin());
			ASSERT_EQ(list_copy, non_empty_list);
		}
	}
}

TEST_F(SingleLinkedListTests, Assign)
{
	// Присваивание списков
	{
		const SingleLinkedList<int> source_list{1, 2, 3, 4};

		SingleLinkedList<int> receiver{5, 4, 3, 2, 1};
		receiver = source_list;
		ASSERT_TRUE(receiver.begin() != source_list.begin());
		ASSERT_EQ(receiver, source_list);
	}

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

	// Безопасное присваивание списков
	{
		SingleLinkedList<ThrowOnCopy> src_list;
		src_list.PushFront(ThrowOnCopy{});
		src_list.PushFront(ThrowOnCopy{});
		auto thrower = src_list.begin();
		src_list.PushFront(ThrowOnCopy{});

		int copy_counter = 0; // при первом же копировании будет выброшего исключение
		thrower->countdown_ptr = &copy_counter;

		SingleLinkedList<ThrowOnCopy> dst_list;
		dst_list.PushFront(ThrowOnCopy{});
		int dst_counter = 10;
		dst_list.begin()->countdown_ptr = &dst_counter;
		dst_list.PushFront(ThrowOnCopy{});

		try
		{
			dst_list = src_list;
			// Ожидается исключение при присваивании
			assert(false);
		}
		catch (const std::bad_alloc&)
		{
			// Проверяем, что состояние списка-приёмника не изменилось
			// при выбрасывании исключений
			ASSERT_EQ(dst_list.GetSize(), 2);
			auto it = dst_list.begin();
			ASSERT_NE(it, dst_list.end());
			assert(it->countdown_ptr == nullptr);
			++it;
			ASSERT_NE(it, dst_list.end());
			ASSERT_EQ(it->countdown_ptr, &dst_counter);
			ASSERT_EQ(dst_counter, 10);
		}
		catch (...)
		{
			// Других типов исключений не ожидается
			assert(false);
		}
	}
}

TEST_F(SingleLinkedListTests, PopFront)
{
	struct DeletionSpy
	{
		~DeletionSpy()
		{
			if (deletion_counter_ptr)
			{
				++(*deletion_counter_ptr);
			}
		}
		int* deletion_counter_ptr = nullptr;
	};
	SingleLinkedList<int> numbers{3, 14, 15, 92, 6};
	numbers.PopFront();
	ASSERT_EQ(numbers, (SingleLinkedList<int>{14, 15, 92, 6}));

	SingleLinkedList<DeletionSpy> list;
	list.PushFront(DeletionSpy{});
	int deletion_counter = 0;
	list.begin()->deletion_counter_ptr = &deletion_counter;
	ASSERT_EQ(deletion_counter, 0);
	list.PopFront();
	ASSERT_EQ(deletion_counter, 1);
}

TEST_F(SingleLinkedListTests, BeforeBegin)
// Доступ к позиции, предшествующей begin
{
	SingleLinkedList<int> empty_list;
	const auto& const_empty_list = empty_list;
	ASSERT_EQ(empty_list.before_begin(), empty_list.cbefore_begin());
	ASSERT_EQ(++empty_list.before_begin(), empty_list.begin());
	ASSERT_EQ(++empty_list.cbefore_begin(), const_empty_list.begin());

	SingleLinkedList<int> numbers{1, 2, 3, 4};
	const auto& const_numbers = numbers;
	ASSERT_EQ(numbers.before_begin(), numbers.cbefore_begin());
	ASSERT_EQ(++numbers.before_begin(), numbers.begin());
	ASSERT_EQ(++numbers.cbefore_begin(), const_numbers.begin());
}

// Вставка элемента после указанной позиции
// Вставка в пустой список
TEST_F(SingleLinkedListTests, InsertEmpty)
{
	SingleLinkedList<int> lst;
	const auto inserted_item_pos = lst.InsertAfter(lst.before_begin(), 123);
	ASSERT_EQ(lst, SingleLinkedList<int>{123});
	ASSERT_EQ(inserted_item_pos, lst.begin());
	ASSERT_EQ(*inserted_item_pos, 123);
}

TEST_F(SingleLinkedListTests, NotEmpty)
// Вставка в непустой список
{
	SingleLinkedList<int> lst{1, 2, 3};
	auto inserted_item_pos = lst.InsertAfter(lst.before_begin(), 123);

	ASSERT_EQ(inserted_item_pos, lst.begin());
	ASSERT_NE(inserted_item_pos, lst.end());
	ASSERT_EQ(*inserted_item_pos, 123);
	ASSERT_EQ(lst, (SingleLinkedList<int>{123, 1, 2, 3}));

	inserted_item_pos = lst.InsertAfter(lst.begin(), 555);
	ASSERT_EQ(++SingleLinkedList<int>::Iterator(lst.begin()), inserted_item_pos);
	ASSERT_EQ(*inserted_item_pos, 555);
	ASSERT_EQ(lst, (SingleLinkedList<int>{123, 555, 1, 2, 3}));
}

TEST_F(SingleLinkedListTests, ThrowOnCopy)
{
	// Вспомогательный класс, бросающий исключение после создания N-копии

	// Проверка обеспечения строгой гарантии безопасности исключений
	{
		bool exception_was_thrown = false;
		for (int max_copy_counter = 10; max_copy_counter >= 0; --max_copy_counter)
		{
			SingleLinkedList<ThrowOnCopy> list{ThrowOnCopy{}, ThrowOnCopy{}, ThrowOnCopy{}};
			try
			{
				int copy_counter = max_copy_counter;
				list.InsertAfter(list.cbegin(), ThrowOnCopy(copy_counter));
				ASSERT_EQ(list.GetSize(), 4u);
			}
			catch (const std::bad_alloc&)
			{
				exception_was_thrown = true;
				ASSERT_EQ(list.GetSize(), 3u);
				break;
			}
		}
		ASSERT_TRUE(exception_was_thrown);
	}
}

TEST_F(SingleLinkedListTests, Deletion)

{
	struct DeletionSpy
	{
		~DeletionSpy()
		{
			if (deletion_counter_ptr)
			{
				++(*deletion_counter_ptr);
			}
		}
		int* deletion_counter_ptr = nullptr;
	};
	{
		SingleLinkedList<int> lst{1, 2, 3, 4};
		const auto& const_lst = lst;
		const auto item_after_erased = lst.EraseAfter(const_lst.cbefore_begin());
		ASSERT_EQ(lst, (SingleLinkedList<int>{2, 3, 4}));
		ASSERT_EQ(item_after_erased, lst.begin());
	}
	{
		SingleLinkedList<int> lst{1, 2, 3, 4};
		const auto item_after_erased = lst.EraseAfter(lst.cbegin());
		ASSERT_EQ(lst, (SingleLinkedList<int>{1, 3, 4}));
		ASSERT_EQ(item_after_erased, (++lst.begin()));
	}
	{
		SingleLinkedList<int> lst{1, 2, 3, 4};
		const auto item_after_erased = lst.EraseAfter(++(++lst.cbegin()));
		ASSERT_EQ(lst, (SingleLinkedList<int>{1, 2, 3}));
		ASSERT_EQ(item_after_erased, lst.end());
	}
	{
		SingleLinkedList<DeletionSpy> list{DeletionSpy{}, DeletionSpy{}, DeletionSpy{}};
		auto after_begin = ++list.begin();
		int deletion_counter = 0;
		after_begin->deletion_counter_ptr = &deletion_counter;
		ASSERT_EQ(deletion_counter, 0u);
		list.EraseAfter(list.cbegin());
		ASSERT_EQ(deletion_counter, 1u);
	}
}
