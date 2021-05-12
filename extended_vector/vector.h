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

	explicit Vector(size_t size) : data_(size), size_(size) //
	{
		std::uninitialized_value_construct_n(data_.GetAddress(), size);
	}

	Vector(const Vector& other) : data_(other.size_), size_(other.size_) //
	{
		std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
	}

	Vector(Vector&& other_) noexcept : data_(std::move(other_.data_)), size_(std::move(other_.size_))
	{
		other_.size_ = 0;
	}

	Vector& operator=(const Vector& other)
	{
		if (this != &other)
		{
			if (other.size_ > data_.Capacity())
			{
				/* Применить copy-and-swap */
				Vector copy(other);
				Swap(copy);
			}
			else
			{
				if (!(size_ < other.size_))
				{
					std::copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
					std::destroy_n(data_.GetAddress() + other.size_, size_ - other.size_);
					size_ = other.size_;
				}
				else
				{
					std::copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
					std::uninitialized_copy_n(other.data_.GetAddress() + size_, other.size_ - size_,
											  data_.GetAddress() + size_);
					size_ = other.size_;
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
			size_ = right_.size_;

			right_.size_ = 0;
		}
		return *this;
	}

	/// Принял буду размещать!
#pragma region "Iterators"
	using iterator = T*;
	using const_iterator = const T*;

	iterator begin() noexcept
	{
		return data_.GetAddress();
	}

	iterator end() noexcept
	{
		return data_ + size_;
	}

	const_iterator begin() const noexcept
	{
		return data_.GetAddress();
	}

	const_iterator end() const noexcept
	{
		return data_ + size_;
	}

	const_iterator cbegin() const noexcept
	{
		return data_.GetAddress();
	}

	const_iterator cend() const noexcept
	{
		return data_ + size_;
	}
#pragma endregion

	void Reserve(size_t new_capacity)
	{
		if (new_capacity <= data_.Capacity())
		{
			return;
		}
		RawMemory<T> new_data(new_capacity);
		// Конструируем элементы в new_data, копируя их из data_
		/// могу шибаться, но мне кажется при резервировании в старом векторе знечения уже будут не нужны, возможно
		/// достаточно делать всегда перемещение UPD(делал не проходит тренажер)
		if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
		{
			std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
		}
		else
		{
			std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
		}
		// Разрушаем элементы в data_
		/// также могу ошибаться, но если было перемещение, от что будет разрушать destroy_n?
		std::destroy_n(data_.GetAddress(), size_);
		// Избавляемся от старой сырой памяти, обменивая её на новую
		data_.Swap(new_data);
		// При выходе из метода старая память будет возвращена в кучу
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
			if (new_size > Capacity())
			{
				Reserve(new_size);
			}
			std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
			size_ = new_size;
		}
	}

	template <typename Type> void PushBack(Type&& value)
	{
		EmplaceBack(std::forward<Type>(value));
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
		return data_.Capacity();
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
		swap(first.size_, second.size_);
	}

	template <typename... Args> T& EmplaceBack(Args&&... args)
	{
		if (size_ == Capacity())
		{
			// в лекциях объясняли почему лучше так :(
			auto new_cap = size_ == 0 ? 1 : size_ * 2;
			RawMemory<T> new_data(new_cap);
			new (new_data + size_) T(std::forward<Args>(args)...);

			if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
			{
				std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
			}
			else
			{
				std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
			}
			std::destroy_n(data_.GetAddress(), size_);

			data_.Swap(new_data);
		}
		else
		{
			new (data_ + size_) T(std::forward<Args>(args)...);
		}
		++size_;

		return data_[size_ - 1];
	}

	/// рекомендую упростить метод, разбить его на отдельные смысловые части в приватные методы и по возможности для
	/// изменения вместимости использльзовать Reserve (будет немного менее эффективно) возможно присутствую проблемы из
	/// методов выше, т.к. код сдублирован и при решении дублирования, они могут решиться
	template <typename... Args> iterator Emplace(const_iterator pos, Args&&... args)
	{
		iterator res_pos = begin();
		if (pos == cend())
		{
			res_pos = &EmplaceBack(std::forward<Args>(args)...);
		}
		else if (size_ == data_.Capacity())
		{
			auto new_cap = size_ == 0 ? 1 : size_ * 2;
			RawMemory<T> new_data(new_cap);
			const size_t dest_pos = (pos - begin());
			new (new_data + dest_pos) T(std::forward<Args>(args)...);
			if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
			{
				std::uninitialized_move_n(data_.GetAddress(), dest_pos, new_data.GetAddress());
			}
			else
			{
				try
				{
					std::uninitialized_copy_n(data_.GetAddress(), dest_pos, new_data.GetAddress());
				}
				catch (...)
				{
					std::destroy_n(new_data + dest_pos, 1);
					throw;
				}
			}

			if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
			{
				std::uninitialized_move_n(data_.GetAddress() + dest_pos, size_ - dest_pos,
										  new_data.GetAddress() + dest_pos + 1);
			}
			else
			{
				try
				{
					std::uninitialized_copy_n(data_.GetAddress() + dest_pos, size_ - dest_pos,
											  new_data.GetAddress() + dest_pos + 1);
				}
				catch (...)
				{
					std::destroy_n(new_data.GetAddress(), dest_pos + 1);
					throw;
				}
			}
			std::destroy_n(data_.GetAddress(), size_);
			data_.Swap(new_data);
			res_pos = begin() + dest_pos;
			++size_;
		}
		else
		{
			T tmp(std::forward<Args>(args)...);
			new (data_ + size_) T(std::move(data_[size_ - 1]));
			res_pos = begin() + (pos - begin());
			std::move_backward(res_pos, end() - 1, end());
			*res_pos = std::move(tmp);
			++size_;
		}

		return res_pos;
	}

	iterator Erase(const_iterator pos)
	{
		iterator res_it = begin() + (pos - begin());
		std::move(res_it + 1, end(), res_it);
		std::destroy_n(end() - 1, 1);
		--size_;

		return res_it;
	}

	template <typename Type> iterator Insert(const_iterator pos, Type&& value)
	{
		return Emplace(pos, std::forward<Type>(value));
	}

  private:
	RawMemory<T> data_;
	size_t size_ = 0;
};