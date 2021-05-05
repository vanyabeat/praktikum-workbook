#include "vector.h"
#include <cassert>
#include <gtest/gtest.h>
#include <iostream>
#include <numeric>
#include <string>

using namespace std;

int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

namespace
{

struct Obj
{
	Obj()
	{
		++num_default_constructed;
	}

	Obj(const Obj& /*other*/)
	{
		++num_copied;
	}

	Obj(Obj&& /*other*/) noexcept
	{
		++num_moved;
	}

	Obj& operator=(const Obj& other) = default;
	Obj& operator=(Obj&& other) = default;

	~Obj()
	{
		++num_destroyed;
	}

	static int GetAliveObjectCount()
	{
		return num_default_constructed + num_copied + num_moved - num_destroyed;
	}

	static void ResetCounters()
	{
		num_default_constructed = 0;
		num_copied = 0;
		num_moved = 0;
		num_destroyed = 0;
	}

	static inline int num_default_constructed = 0;
	static inline int num_copied = 0;
	static inline int num_moved = 0;
	static inline int num_destroyed = 0;
};

} // namespace

TEST(Vector, Test1)
{
	Obj::ResetCounters();
	const size_t SIZE = 100500;
	const size_t INDEX = 10;
	const int MAGIC = 42;
	{
		Vector<int> v;
		ASSERT_EQ(v.Capacity(), 0);
		ASSERT_EQ(v.Size(), 0);

		v.Reserve(SIZE);
		ASSERT_EQ(v.Capacity(), SIZE);
		ASSERT_EQ(v.Size(), 0);
	}
	{
		Vector<int> v(SIZE);
		const auto& cv(v);
		ASSERT_EQ(v.Capacity(), SIZE);
		ASSERT_EQ(v.Size(), SIZE);
		ASSERT_EQ(v[0], 0);
		ASSERT_EQ(&v[0], &cv[0]);
		v[INDEX] = MAGIC;
		ASSERT_EQ(v[INDEX], MAGIC);
		ASSERT_EQ(&v[100] - &v[0], 100);

		v.Reserve(SIZE * 2);
		ASSERT_EQ(v.Size(), SIZE);
		ASSERT_EQ(v.Capacity(), SIZE * 2);
		ASSERT_EQ(v[INDEX], MAGIC);
	}
	{
		Vector<int> v(SIZE);
		v[INDEX] = MAGIC;
		const auto v_copy(v);
		ASSERT_NE(&v[INDEX], &v_copy[INDEX]);
		ASSERT_EQ(v[INDEX], v_copy[INDEX]);
	}
	{
		Vector<Obj> v;
		v.Reserve(SIZE);
		ASSERT_EQ(Obj::GetAliveObjectCount(), 0);
	}
	{
		Vector<Obj> v(SIZE);
		ASSERT_EQ(Obj::GetAliveObjectCount(), SIZE);
		v.Reserve(SIZE * 2);
		ASSERT_EQ(Obj::GetAliveObjectCount(), SIZE);
	}
	ASSERT_EQ(Obj::GetAliveObjectCount(), 0);
}

namespace
{

struct Obj2
{
	Obj2()
	{
		if (default_construction_throw_countdown > 0)
		{
			if (--default_construction_throw_countdown == 0)
			{
				throw std::runtime_error("Oops");
			}
		}
		++num_default_constructed;
	}

	Obj2(const Obj2& other)
	{
		if (other.throw_on_copy)
		{
			throw std::runtime_error("Oops");
		}
		++num_copied;
	}

	Obj2(Obj&& /*other*/) noexcept
	{
		++num_moved;
	}

	Obj2& operator=(const Obj2& other) = default;
	Obj2& operator=(Obj2&& other) = default;

	~Obj2()
	{
		++num_destroyed;
	}

	static int GetAliveObjectCount()
	{
		return num_default_constructed + num_copied + num_moved - num_destroyed;
	}

	static void ResetCounters()
	{
		default_construction_throw_countdown = 0;
		num_default_constructed = 0;
		num_copied = 0;
		num_moved = 0;
		num_destroyed = 0;
	}

	bool throw_on_copy = false;

	static inline int default_construction_throw_countdown = 0;
	static inline int num_default_constructed = 0;
	static inline int num_copied = 0;
	static inline int num_moved = 0;
	static inline int num_destroyed = 0;
};

} // namespace

TEST(Vector, Test2)
{

	const size_t SIZE = 100;
	Obj2::ResetCounters();
	{
		Obj2::default_construction_throw_countdown = SIZE / 2;
		try
		{
			Vector<Obj2> v(SIZE);
			ASSERT_TRUE(false && "Exception is expected");
		}
		catch (const std::runtime_error&)
		{
		}
		catch (...)
		{
			// Unexpected error
			ASSERT_TRUE(false && "Unexpected exception");
		}
		ASSERT_EQ(Obj2::num_default_constructed, SIZE / 2 - 1);
		ASSERT_EQ(Obj2::GetAliveObjectCount(), 0);
	}
	Obj2::ResetCounters();
	{
		Vector<Obj2> v(SIZE);
		try
		{
			v[SIZE / 2].throw_on_copy = true;
			Vector<Obj2> v_copy(v);
			ASSERT_TRUE(false && "Exception is expected");
		}
		catch (const std::runtime_error&)
		{
			ASSERT_EQ(Obj2::num_copied, SIZE / 2);
		}
		catch (...)
		{
			// Unexpected error
			ASSERT_TRUE(false && "Unexpected exception");
		}
		ASSERT_EQ(Obj2::GetAliveObjectCount(), SIZE);
	}
	Obj2::ResetCounters();
	{
		Vector<Obj2> v(SIZE);
		try
		{
			v[SIZE - 1].throw_on_copy = true;
			v.Reserve(SIZE * 2);
			ASSERT_TRUE(false && "Exception is expected");
		}
		catch (const std::runtime_error&)
		{
			ASSERT_EQ(Obj2::num_copied, SIZE - 1);
		}
		catch (...)
		{
			// Unexpected error
			ASSERT_TRUE(false && "Unexpected exception");
		}
		ASSERT_EQ(v.Capacity(), SIZE);
		ASSERT_EQ(v.Size(), SIZE);
		ASSERT_EQ(Obj2::GetAliveObjectCount(), SIZE);
	}
}

struct Obj3
{
	Obj3()
	{
		if (default_construction_throw_countdown > 0)
		{
			if (--default_construction_throw_countdown == 0)
			{
				throw std::runtime_error("Oops");
			}
		}
		++num_default_constructed;
	}

	Obj3(const Obj3& other)
	{
		if (other.throw_on_copy)
		{
			throw std::runtime_error("Oops");
		}
		++num_copied;
	}

	Obj3(Obj3&& /*other*/) noexcept
	{
		++num_moved;
	}

	Obj3& operator=(const Obj3& other) = default;
	Obj3& operator=(Obj3&& other) = default;

	~Obj3()
	{
		++num_destroyed;
	}

	static int GetAliveObjectCount()
	{
		return num_default_constructed + num_copied + num_moved - num_destroyed;
	}

	static void ResetCounters()
	{
		default_construction_throw_countdown = 0;
		num_default_constructed = 0;
		num_copied = 0;
		num_moved = 0;
		num_destroyed = 0;
	}

	bool throw_on_copy = false;

	static inline int default_construction_throw_countdown = 0;
	static inline int num_default_constructed = 0;
	static inline int num_copied = 0;
	static inline int num_moved = 0;
	static inline int num_destroyed = 0;
};

TEST(Vector, Test3_2)
{
	Obj3::ResetCounters();
	const size_t SIZE = 100500;
	const size_t INDEX = 10;
	const int MAGIC = 42;
	{
		Vector<int> v;
		ASSERT_EQ(v.Capacity(), 0);
		ASSERT_EQ(v.Size(), 0);

		v.Reserve(SIZE);
		ASSERT_EQ(v.Capacity(), SIZE);
		ASSERT_EQ(v.Size(), 0);
	}
	{
		Vector<int> v(SIZE);
		const auto& cv(v);
		ASSERT_EQ(v.Capacity(), SIZE);
		ASSERT_EQ(v.Size(), SIZE);
		ASSERT_EQ(v[0], 0);
		ASSERT_EQ(&v[0], &cv[0]);
		v[INDEX] = MAGIC;
		ASSERT_EQ(v[INDEX], MAGIC);
		ASSERT_EQ(&v[100] - &v[0], 100);

		v.Reserve(SIZE * 2);
		ASSERT_EQ(v.Size(), SIZE);
		ASSERT_EQ(v.Capacity(), SIZE * 2);
		ASSERT_EQ(v[INDEX], MAGIC);
	}
	{
		Vector<int> v(SIZE);
		v[INDEX] = MAGIC;
		const auto v_copy(v);
		ASSERT_NE(&v[INDEX], &v_copy[INDEX]);
		ASSERT_EQ(v[INDEX], v_copy[INDEX]);
	}
	{
		Vector<Obj3> v;
		v.Reserve(SIZE);
		ASSERT_EQ(Obj3::GetAliveObjectCount(), 0);
	}
	{
		Vector<Obj3> v(SIZE);
		ASSERT_EQ(Obj3::GetAliveObjectCount(), SIZE);
		const int old_copy_count = Obj3::num_copied;
		const int old_move_count = Obj3::num_moved;
		v.Reserve(SIZE * 2);
		ASSERT_EQ(Obj3::GetAliveObjectCount(), SIZE);
		ASSERT_EQ(Obj3::num_copied, old_copy_count);
		ASSERT_EQ(Obj3::num_moved, old_move_count + static_cast<int>(SIZE));
	}
	ASSERT_EQ(Obj3::GetAliveObjectCount(), 0);
}

TEST(Vector, Test3_1)
{
	const size_t SIZE = 100;
	Obj3::ResetCounters();
	{
		Obj3::default_construction_throw_countdown = SIZE / 2;
		try
		{
			Vector<Obj3> v(SIZE);
			ASSERT_TRUE(false && "Exception is expected");
		}
		catch (const std::runtime_error&)
		{
		}
		catch (...)
		{
			// Unexpected error
			ASSERT_TRUE(false && "Unexpected exception");
		}
		ASSERT_EQ(Obj3::num_default_constructed, SIZE / 2 - 1);
		ASSERT_EQ(Obj3::GetAliveObjectCount(), 0);
	}
	Obj3::ResetCounters();
	{
		Vector<Obj3> v(SIZE);
		try
		{
			v[SIZE / 2].throw_on_copy = true;
			Vector<Obj3> v_copy(v);
			ASSERT_TRUE(false && "Exception is expected");
		}
		catch (const std::runtime_error&)
		{
			ASSERT_EQ(Obj3::num_copied, SIZE / 2);
		}
		catch (...)
		{
			// Unexpected error
			ASSERT_TRUE(false && "Unexpected exception");
		}
		ASSERT_EQ(Obj3::GetAliveObjectCount(), SIZE);
	}
	Obj3::ResetCounters();
	{
		Vector<Obj3> v(SIZE);
		try
		{
			v[SIZE - 1].throw_on_copy = true;
			v.Reserve(SIZE * 2);
		}
		catch (...)
		{
			// Unexpected error
			ASSERT_TRUE(false && "Unexpected exception");
		}
		ASSERT_EQ(v.Capacity(), SIZE * 2);
		ASSERT_EQ(v.Size(), SIZE);
		ASSERT_EQ(Obj3::GetAliveObjectCount(), SIZE);
	}
}

struct Obj4
{
	Obj4()
	{
		if (default_construction_throw_countdown > 0)
		{
			if (--default_construction_throw_countdown == 0)
			{
				throw std::runtime_error("Oops");
			}
		}
		++num_default_constructed;
	}

	explicit Obj4(int id) : id(id) //
	{
		++num_constructed_with_id;
	}

	Obj4(const Obj4& other) : id(other.id) //
	{
		if (other.throw_on_copy)
		{
			throw std::runtime_error("Oops");
		}
		++num_copied;
	}

	Obj4(Obj4&& other) noexcept : id(other.id) //
	{
		++num_moved;
	}

	Obj4& operator=(const Obj4& other) = default;
	Obj4& operator=(Obj4&& other) = default;

	~Obj4()
	{
		++num_destroyed;
		id = 0;
	}

	static int GetAliveObjectCount()
	{
		return num_default_constructed + num_copied + num_moved + num_constructed_with_id - num_destroyed;
	}

	static void ResetCounters()
	{
		default_construction_throw_countdown = 0;
		num_default_constructed = 0;
		num_copied = 0;
		num_moved = 0;
		num_destroyed = 0;
		num_constructed_with_id = 0;
	}

	bool throw_on_copy = false;
	int id = 0;

	static inline int default_construction_throw_countdown = 0;
	static inline int num_default_constructed = 0;
	static inline int num_constructed_with_id = 0;
	static inline int num_copied = 0;
	static inline int num_moved = 0;
	static inline int num_destroyed = 0;
};

TEST(Vector, Test4)
{
	const size_t MEDIUM_SIZE = 100;
	const size_t LARGE_SIZE = 250;
	const int ID = 42;
	{
		Obj4::ResetCounters();
		Vector<int> v(MEDIUM_SIZE);
		{
			auto v_copy(std::move(v));

			ASSERT_EQ(v_copy.Size(), MEDIUM_SIZE);
			ASSERT_EQ(v_copy.Capacity(), MEDIUM_SIZE);
		}
		ASSERT_EQ(Obj4::GetAliveObjectCount() , 0);
	}
	{
		Obj4::ResetCounters();
		{
			Vector<Obj4> v(MEDIUM_SIZE);
			v[MEDIUM_SIZE / 2].id = ID;
			ASSERT_EQ(Obj4::num_default_constructed, MEDIUM_SIZE);
			Vector<Obj4> moved_from_v(std::move(v));
			ASSERT_EQ(moved_from_v.Size(), MEDIUM_SIZE);
			ASSERT_EQ(moved_from_v[MEDIUM_SIZE / 2].id, ID);
		}
		ASSERT_EQ(Obj4::GetAliveObjectCount(), 0);

		ASSERT_EQ(Obj4::num_moved, 0);
		ASSERT_EQ(Obj4::num_copied, 0);
		ASSERT_EQ(Obj4::num_default_constructed, MEDIUM_SIZE);
	}
	{
		Obj4::ResetCounters();
		Vector<Obj4> v_medium(MEDIUM_SIZE);
		v_medium[MEDIUM_SIZE / 2].id = ID;
		Vector<Obj4> v_large(LARGE_SIZE);
		v_large = v_medium;
		ASSERT_EQ(v_large.Size(), MEDIUM_SIZE);
		ASSERT_EQ(v_large.Capacity(), LARGE_SIZE);
		ASSERT_EQ(v_large[MEDIUM_SIZE / 2].id, ID);
		ASSERT_EQ(Obj4::GetAliveObjectCount(), MEDIUM_SIZE + MEDIUM_SIZE);
	}
	{
		Obj4::ResetCounters();
		Vector<Obj4> v(MEDIUM_SIZE);
		{
			Vector<Obj4> v_large(LARGE_SIZE);
			v_large[LARGE_SIZE - 1].id = ID;
			v = v_large;
			ASSERT_EQ(v.Size(), LARGE_SIZE);
			ASSERT_EQ(v_large.Capacity(), LARGE_SIZE);
			ASSERT_EQ(v_large[LARGE_SIZE - 1].id, ID);
			ASSERT_EQ(Obj4::GetAliveObjectCount(), LARGE_SIZE + LARGE_SIZE);
		}
		ASSERT_EQ(Obj4::GetAliveObjectCount() , LARGE_SIZE);
	}
	assert(Obj4::GetAliveObjectCount() == 0);
	{
		Obj4::ResetCounters();
		Vector<Obj4> v(MEDIUM_SIZE);
		v[MEDIUM_SIZE - 1].id = ID;
		Vector<Obj4> v_small(MEDIUM_SIZE / 2);
		v_small.Reserve(MEDIUM_SIZE + 1);
		const size_t num_copies = Obj4::num_copied;
		v_small = v;
		ASSERT_EQ(v_small.Size() , v.Size());
		ASSERT_EQ(v_small.Capacity() , MEDIUM_SIZE + 1);
		v_small[MEDIUM_SIZE - 1].id = ID;
		ASSERT_EQ(Obj4::num_copied - num_copies , MEDIUM_SIZE - (MEDIUM_SIZE / 2));
	}
}