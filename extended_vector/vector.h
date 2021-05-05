#pragma once
#include "raw_memory.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <new>
#include <utility>

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

	Vector(Vector&& other_) noexcept
		: data_(std::move(other_.data_)), capacity_(std::move(other_.capacity_)), size_(std::move(other_.size_))
	{
		other_.capacity_ = 0;
		other_.size_ = 0;
	}

	Vector& operator=(const Vector& right_)
	{
		if (this != &right_)
		{
			if (right_.size_ > data_.Capacity())
			{
				/* Применить copy-and-swap */
				Vector copy(right_);
				Swap(copy);
			}
			else
			{
				if (!(size_ < right_.size_))
				{
					std::copy_n(right_.data_.GetAddress(), right_.size_, data_.GetAddress());
					std::destroy_n(data_.GetAddress() + right_.size_, size_ - right_.size_);
					size_ = right_.size_;
				}
				else
				{
					;
					for (auto [it_r, it_this] = std::tuple{right_.data_.GetAddress(), data_.GetAddress()};
						 it_r != right_.data_.GetAddress() + size_; ++it_r, ++it_this)
					{
						*it_this = *it_r;
					}
					std::uninitialized_copy_n(right_.data_.GetAddress() + size_, right_.size_ - size_,
											  data_.GetAddress() + size_);
					size_ = right_.size_;
				}
			}
		}
		return *this;
	}

	Vector& operator=(Vector&& right_) noexcept
	{
		if (this != &right_)
		{
			data_ = std::move(right_.data_);
			capacity_ = std::move(right_.capacity_);
			size_ = std::move(right_.size_);
			right_.capacity_ = 0;
			right_.size_ = 0;
		}
		return *this;
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

	void Resize(size_t new_size)
	{
		if (new_size < size_)
		{
			std::destroy_n(data_.GetAddress() + new_size, size_ - new_size);
			size_ = new_size;
		}
		else
		{
			if (new_size > capacity_)
			{
				Reserve(new_size);
			}
			std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
			size_ = new_size;
		}
	}

	template <typename C> void PushBack(C value)
	{
		if (size_ == 0 && Capacity() == 0)
		{
			Reserve(1);
		}
		constexpr bool is_nothrow_move = std::is_nothrow_move_constructible_v<C>;
		constexpr bool is_move = std::is_move_constructible_v<C>;
		constexpr bool is_nothrow_copy = std::is_nothrow_copy_constructible_v<C>;
		constexpr bool is_copy = std::is_copy_constructible_v<C>;

		if (size_ == Capacity())
		{
			RawMemory<T> tmp(Capacity() * 2);
			if constexpr (is_nothrow_move && !is_nothrow_copy)
			{
				new (tmp.GetAddress() + size_) T(std::move(value));
			}
			else if constexpr (is_move && !is_copy)
			{
				try
				{
					new (tmp.GetAddress() + size_) T(std::move(value));
				}
				catch (...)
				{
					throw;
				}
			}
			else if constexpr (is_nothrow_copy && !is_nothrow_move)
			{
				new (tmp.GetAddress() + size_) T(value);
			}
			else if constexpr (is_copy && !is_move)
			{
				try
				{
					new (tmp.GetAddress() + size_) T(value);
				}
				catch (...)
				{
					throw;
				}
			}

			std::uninitialized_move_n(data_.GetAddress(), size_, tmp.GetAddress());
			std::destroy_n(data_.GetAddress(), size_);
			std::swap(data_, tmp);
			capacity_ = capacity_ * 2;
		}
		else
		{
			if constexpr (is_nothrow_move && !is_nothrow_copy)
			{
				new (data_ + size_) T(std::move(value));
			}
			else if constexpr (is_move && !is_copy)
			{
				try
				{
					new (data_ + size_) T(std::move(value));
				}
				catch (...)
				{
					throw;
				}
			}
			else if constexpr (is_nothrow_copy && !is_nothrow_move)
			{
				new (data_ + size_) T(value);
			}
			else if constexpr (is_copy && !is_move)
			{
				try
				{
					new (data_ + size_) T(value);
				}
				catch (...)
				{
					throw;
				}
			}
		}

		++size_;
	}

	void PopBack() noexcept
	{
		std::destroy_at(data_.GetAddress() + size_);
		--size_;
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

	void Swap(Vector& other) noexcept
	{
		using std::swap;
		swap(*this, other);
	}

	friend void swap(Vector<T>& first, Vector<T>& second) noexcept
	{
		using std::swap;
		swap(first.data_, second.data_);
		swap(first.capacity_, second.capacity_);
		swap(first.size_, second.size_);
	}

  private:
	RawMemory<T> data_;
	size_t capacity_ = 0;
	size_t size_ = 0;
};