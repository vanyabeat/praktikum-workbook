#pragma once
#include <cassert>
#include <cstdlib>
#include <memory>
#include <new>
#include <utility>

#include "raw_memory.h"

template <typename T> class Vector
{
  public:
	Vector() = default;

	explicit Vector(size_t size) : data_(size), capacity_(size), size_(size) //
	{
		std::uninitialized_value_construct_n(data_.GetAddress(), size);
	}

	Vector(const Vector& other) : data_(other.size_), capacity_(other.size_), size_(other.size_) //
	{
		std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
	}

	void Reserve(size_t new_capacity)
	{
		if (new_capacity <= data_.Capacity())
		{
			return;
		}
		RawMemory<T> new_data(new_capacity);
		// Конструируем элементы в new_data, копируя их из data_
		if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
		{
			std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
		}
		else
		{
			std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
		}
		// Разрушаем элементы в data_
		std::destroy_n(data_.GetAddress(), size_);
		// Избавляемся от старой сырой памяти, обменивая её на новую
		data_.Swap(new_data);
		// При выходе из метода старая память будет возвращена в кучу
		capacity_ = new_capacity;
	}

	~Vector()
	{
		std::destroy_n(data_.GetAddress(), size_);
	}

	size_t Size() const noexcept
	{
		return size_;
	}

	size_t Capacity() const noexcept
	{
		return capacity_;
	}

	const T& operator[](size_t index) const noexcept
	{
		return const_cast<Vector&>(*this)[index];
	}

	T& operator[](size_t index) noexcept
	{
		assert(index < size_);
		return data_[index];
	}

  private:
	RawMemory<T> data_;
	size_t capacity_ = 0;
	size_t size_ = 0;
};