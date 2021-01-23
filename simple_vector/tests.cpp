#include "simple_vector.h"
#include <cassert>
#include <gtest/gtest.h>
#include <iostream>
#include <numeric>
#include <string>


using namespace std;

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(SimpleVectorTest, one) {
	SimpleVector<int> v;
	ASSERT_EQ(v.GetSize(), 0u);
	ASSERT_TRUE(v.IsEmpty());
	ASSERT_EQ(v.GetCapacity(), 0u);
}


TEST(SimpleVectorTest, Init) {
	// Инициализация вектора указанного размера
	SimpleVector<int> v(5);
	ASSERT_EQ(v.GetSize(), 5u);
	ASSERT_EQ(v.GetCapacity(), 5u);
	ASSERT_TRUE(!v.IsEmpty());
	for (size_t i = 0; i < v.GetSize(); ++i) {
		ASSERT_EQ(v[i], 0);
	}
}

TEST(SimpleVectorTest, Init2) {
	SimpleVector<int> v(3, 42);
	ASSERT_EQ(v.GetSize(), 3);
	ASSERT_EQ(v.GetCapacity(), 3);
	for (size_t i = 0; i < v.GetSize(); ++i) {
		ASSERT_EQ(v[i], 42);
	}
}

TEST(SimpleVectorTest, InitList) {
	SimpleVector<int> v{1, 2, 3};
	ASSERT_EQ(v.GetSize(), 3);
	ASSERT_EQ(v.GetCapacity(), 3);
	ASSERT_EQ(v[2], 3);
}

TEST(SimpleVectorTest, At) {
	SimpleVector<int> v(3);
	ASSERT_EQ(&v.At(2), &v[2]);
	try {
		v.At(3);
	} catch (std::out_of_range const &err) {
		EXPECT_EQ(err.what(), std::string("Invalid index"));
	}
}

TEST(SimpleVectorTest, Clear) {

	SimpleVector<int> v(10);
	const size_t old_capacity = v.GetCapacity();
	v.Clear();
	ASSERT_EQ(v.GetSize(), 0);
	ASSERT_EQ(v.GetCapacity(), old_capacity);
}

TEST(SimpleVectorTest, Resize) {
	// Изменение размера
	{
		SimpleVector<int> v(3);
		v[2] = 17;
		v.Resize(7);
		ASSERT_EQ(v.GetSize(), 7);
		ASSERT_TRUE(v.GetCapacity() >= v.GetSize());
		ASSERT_EQ(v[2], 17);
		ASSERT_EQ(v[3], 0);
	}
}

TEST(SimpleVectorTest, Resize1) {
	{
		SimpleVector<int> v(3);
		v[0] = 42;
		v[1] = 55;
		const size_t old_capacity = v.GetCapacity();
		v.Resize(2);
		ASSERT_EQ(v.GetSize(), 2);
		ASSERT_EQ(v.GetCapacity(), old_capacity);
		ASSERT_EQ(v[0], 42);
		ASSERT_EQ(v[1], 55);
	}
}

TEST(SimpleVectorTest, Resize2) {
	{
		const size_t old_size = 3;
		SimpleVector<int> v(3);
		v.Resize(old_size + 5);
		v[3] = 42;
		v.Resize(old_size);
		ASSERT_EQ(v[2], 0);
		v.Resize(old_size + 3);
		ASSERT_EQ(v[3], 0);
	}
}

TEST(SimpleVectorTest, Constructors) {
	{

		SimpleVector<int> v(5);
		v[0] = 1;
		v[1] = 2;
		v[2] = 3;
		v[3] = 4;
		v[4] = 5;
		SimpleVector<int> c(v);
		ASSERT_EQ(v[2], v[2]);
	}
	{

		SimpleVector<int> v(5);
		v[0] = 1;
		v[1] = 2;
		v[2] = 3;
		v[3] = 4;
		v[4] = 5;
		SimpleVector<int> c(v);
		ASSERT_EQ(v[2], v[2]);
	}
	{

		SimpleVector<int> v(1);
		v.PushBack(1);
		ASSERT_EQ(v[0], 0);
		ASSERT_EQ(*(v.begin() + 1), 1);
	}
}

TEST(SimpleVectorTest, Push) {
	SimpleVector<int> v(1);
	v.PushBack(42);
	ASSERT_EQ(v.GetSize(), 2);
	ASSERT_TRUE(v.GetCapacity() >= v.GetSize());
	ASSERT_EQ(v[0], 0);
	ASSERT_EQ(v[1], 42);
}

TEST(SimpleVectorTest, CopyConstruct)
// Конструктор копирования
{
	SimpleVector<int> numbers{1, 2};
	auto numbers_copy(numbers);
	ASSERT_TRUE(&numbers_copy[0] != &numbers[0]);
	ASSERT_EQ(numbers_copy.GetSize(), numbers.GetSize());
	for (size_t i = 0; i < numbers.GetSize(); ++i) {
		ASSERT_EQ(numbers_copy[i], numbers[i]);
		ASSERT_TRUE(&numbers_copy[i] != &numbers[i]);
	}
}

TEST(SimpleVector, PopBack) {
	// PopBack
	{
		SimpleVector<int> v{0, 1, 2, 3};
		const size_t old_capacity = v.GetCapacity();
		const auto old_begin = v.begin();
		v.PopBack();
		ASSERT_EQ(v.GetCapacity(), old_capacity);
		ASSERT_EQ(v.begin(), old_begin);
		ASSERT_EQ(v, (SimpleVector<int>{0, 1, 2}));
	}
}

TEST(SimpleVectorTest, Capacity) {

	SimpleVector<int> v(2);
	v.Resize(1);
	const size_t old_capacity = v.GetCapacity();
	v.PushBack(123);
	ASSERT_EQ(v.GetSize(), 2);
	ASSERT_EQ(v.GetCapacity(), old_capacity);
}

TEST(SimpleVectorTest, Iterate) {
	// Итерирование по SimpleVector
	{
		// Пустой вектор
		{
			SimpleVector<int> v;
			ASSERT_EQ(v.begin(), nullptr);
			ASSERT_EQ(v.end(), nullptr);
		}

		// Непустой вектор
		{
			SimpleVector<int> v(10, 42);
			ASSERT_TRUE(v.begin());
			ASSERT_EQ(*v.begin(), 42);
			ASSERT_EQ(v.end(), v.begin() + v.GetSize());
		}
	}
}

TEST(SimpleVectorTest, Compare) {
	{
		ASSERT_TRUE((SimpleVector{1, 2, 3} == SimpleVector{1, 2, 3}));
		ASSERT_TRUE((SimpleVector{1, 2, 3} != SimpleVector{1, 2, 2}));

		ASSERT_TRUE((SimpleVector{1, 2, 3} < SimpleVector{1, 2, 3, 1}));
		ASSERT_TRUE((SimpleVector{1, 2, 3} > SimpleVector{1, 2, 2, 1}));

		ASSERT_TRUE((SimpleVector{1, 2, 3} >= SimpleVector{1, 2, 3}));
		ASSERT_TRUE((SimpleVector{1, 2, 4} >= SimpleVector{1, 2, 3}));
		ASSERT_TRUE((SimpleVector{1, 2, 3} <= SimpleVector{1, 2, 3}));
		ASSERT_TRUE((SimpleVector{1, 2, 3} <= SimpleVector{1, 2, 4}));
	}
}

TEST(SimpleVectorTest, Pushback2) {
	SimpleVector<int> v2;
	v2.PushBack(0);
	v2.PushBack(1);
	v2.PushBack(2);
	ASSERT_EQ(v2, (SimpleVector<int>{0, 1, 2}));
}

TEST(SimpleVectorTest, Swap) {
	// Обмен значений векторов
	{
		SimpleVector<int> v1{42, 666};
		SimpleVector<int> v2;
		v2.PushBack(0);
		v2.PushBack(1);
		v2.PushBack(2);
		const int *const begin1 = &v1[0];
		const int *const begin2 = &v2[0];

		const size_t capacity1 = v1.GetCapacity();
		const size_t capacity2 = v2.GetCapacity();

		const size_t size1 = v1.GetSize();
		const size_t size2 = v2.GetSize();

		v1.swap(v2);
		ASSERT_TRUE(&v2[0] == begin1);
		ASSERT_TRUE(&v1[0] == begin2);
		ASSERT_TRUE(v1.GetSize() == size2);
		ASSERT_TRUE(v2.GetSize() == size1);
		ASSERT_TRUE(v1.GetCapacity() == capacity2);
		ASSERT_TRUE(v2.GetCapacity() == capacity1);
	}
}

TEST(SimpleVectorTest, Test1) {
	{
		SimpleVector<int> src_vector{1, 2, 3, 4};
		SimpleVector<int> dst_vector{1, 2, 3, 4, 5, 6};
		dst_vector = src_vector;
		ASSERT_EQ(dst_vector, src_vector);
	}
}

TEST(SimpleVectorTest, Insert) {
	// Вставка элементов
	{
		SimpleVector<int> v{1, 2, 3, 4};
		v.Insert(v.begin() + 2, 42);
		ASSERT_EQ(v, (SimpleVector<int>{1, 2, 42, 3, 4}));
	}
}

TEST(SimpleVectorTest, Insert2) {
	// Вставка элементов

	SimpleVector<int> v;
	v.Insert(v.begin(), 42);
	ASSERT_EQ(v, (SimpleVector<int>{42}));
}

TEST(SimpleVectorTest, Erase) {
	{
		SimpleVector<int> v{1, 2, 3, 4};
		v.Erase(v.cbegin() + 2);

		ASSERT_EQ(v, (SimpleVector<int>{1, 2, 4}));
	}
}

TEST(SimpleVectorTests, Reserve) {

	{
		cout << "TestReserveConstructor"s << endl;
		SimpleVector<int> v(Reserve(5));
		ASSERT_EQ(v.GetCapacity(), 5);
		ASSERT_TRUE(v.IsEmpty());
		cout << "Done!"s << endl;
	}

	{
		cout << "TestReserveMethod"s << endl;
		SimpleVector<int> v;
		// зарезервируем 5 мест в векторе
		v.Reserve(5);
		ASSERT_EQ(v.GetCapacity(), 5);
		ASSERT_TRUE(v.IsEmpty());

		// попытаемся уменьшить capacity до 1
		v.Reserve(1);
		// capacity должно остаться прежним
		ASSERT_EQ(v.GetCapacity(), 5);
		// поместим 10 элементов в вектор
		for (int i = 0; i < 10; ++i) {
			v.PushBack(i);
		}
		ASSERT_EQ(v.GetSize(), 10);
		// увеличим capacity до 100
		v.Reserve(100);
		// проверим, что размер не поменялся
		ASSERT_EQ(v.GetSize(), 10);
		ASSERT_EQ(v.GetCapacity(), 100);
		// проверим, что элементы на месте
		for (int i = 0; i < 10; ++i) {
			ASSERT_EQ(v[i], i);
		}
		cout << "Done!"s << endl;
	}
}

class X {
public:
	X()
		: X(5) {
	}
	X(size_t num)
		: x_(num) {
	}
	X(const X &other) = delete;
	X &operator=(const X &other) = delete;
	X(X &&other) {
		x_ = exchange(other.x_, 0);
	}
	X &operator=(X &&other) {
		x_ = exchange(other.x_, 0);
		return *this;
	}
	size_t GetX() const {
		return x_;
	}

private:
	size_t x_;
};

SimpleVector<int> GenerateVector(size_t size) {
	SimpleVector<int> v(size);
	iota(v.begin(), v.end(), 1);
	return v;
}

TEST(SimpleVectorTest, temporary) {
	const size_t size = 1000000;
	cout << "Test with temporary object, copy elision" << endl;
	SimpleVector<int> moved_vector(GenerateVector(size));
	ASSERT_EQ(moved_vector.GetSize(), size);
	cout << "Done!" << endl
		 << endl;
}

TEST(SimpleVectorTest, moveconstructor) {
	const size_t size = 1000000;
	cout << "Test with named object, move constructor" << endl;
	SimpleVector<int> vector_to_move(GenerateVector(size));
	ASSERT_EQ(vector_to_move.GetSize(), size);

	SimpleVector<int> moved_vector(move(vector_to_move));
	ASSERT_EQ(moved_vector.GetSize(), size);
	ASSERT_EQ(vector_to_move.GetSize(), 0);
	cout << "Done!" << endl
		 << endl;
}