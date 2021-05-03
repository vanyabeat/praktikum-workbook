#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>

#include "raw_memory.h"

template <typename T> class Vector
{
  public:
	Vector() = default;

	explicit Vector(size_t size) : data_(size), capacity_(size), size_(size) //
	{
		size_t i = 0;
		try
		{
			for (; i != size; ++i)
			{
				new (data_ + i) T();
			}
		}
		catch (...)
		{
			destroy_n_(data_.GetAddress(), i);
			throw;
		}
	}

	Vector(const Vector& other) : data_(other.size_), capacity_(other.size_), size_(other.size_) //
	{
		size_t i = 0;
		try
		{
			for (; i != other.size_; ++i)
			{
				copy_construct_(data_ + i, other.data_[i]);
			}
		}
		catch (...)
		{
			destroy_n_(data_.GetAddress(), i);
			throw;
		}
	}

	void Reserve(size_t new_capacity)
	{
		if (new_capacity <= capacity_)
		{
			return;
		}
		RawMemory<T> new_data;
		try
		{
			RawMemory<T> other(new_capacity);
			new_data.Swap(other);
		}
		catch (...)
		{
			throw;
		}
		size_t i = 0;
		try
		{
			for (; i != size_; ++i)
			{
				copy_construct_(new_data + i, data_[i]);
			}
			data_.Swap(new_data);
		}
		catch (...)
		{
			destroy_n_(data_.GetAddress(), i);
			throw;
		}
		destroy_n_(new_data.GetAddress(), i);
		capacity_ = new_capacity;
	}

	~Vector()
	{
		destroy_n_(data_.GetAddress(), size_);
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

  private:
	// Вызывает деструкторы n объектов массива по адресу buf
	static void destroy_n_(T* buf, size_t n) noexcept
	{
		for (size_t i = 0; i != n; ++i)
		{
			destroy_(buf + i);
		}
	}

	// Создаёт копию объекта elem в сырой памяти по адресу buf
	static void copy_construct_(T* buf, const T& elem)
	{
		new (buf) T(elem);
	}

	// Вызывает деструктор объекта по адресу buf
	static void destroy_(T* buf) noexcept
	{
		buf->~T();
	}
};