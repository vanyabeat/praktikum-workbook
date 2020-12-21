#include "../src/array_ptr.h"
#include <gtest/gtest.h>

TEST(ArrayTest, Arrayptr)
{
	ArrayPtr<int> numbers(10);
	const auto& const_numbers = numbers;

	numbers[2] = 42;
	ASSERT_EQ(const_numbers[2], 42);
	ASSERT_EQ(&const_numbers[2], &numbers[2]);

	ASSERT_EQ(numbers.Get(), &numbers[0]);

	ArrayPtr<int> numbers_2(5);
	numbers_2[2] = 43;

	numbers.swap(numbers_2);

	ASSERT_EQ(numbers_2[2], 42);
	ASSERT_EQ(numbers[2], 43);
}