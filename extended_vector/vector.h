#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>

template <typename T> class Vector
{
  public:
	Vector() = default;

	explicit Vector(size_t size) : data_(alloc_(size)), capacity_(size), size_(size) //
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
			// В переменной i содержится количество созданных элементов.
			// Теперь их надо разрушить
			destroy_n_(data_, i);
			// Освобождаем память, выделенную через Allocate
			dealloc_(data_);
			// Перевыбрасываем пойманное исключение, чтобы сообщить об ошибке создания объекта
			throw;
		}
	}

	Vector(const Vector& other) : data_(alloc_(other.size_)), capacity_(other.size_), size_(other.size_) //
	{
		for (size_t i = 0; i != other.size_; ++i)
		{
			copy_construct_(data_ + i, other.data_[i]);
		}
	}

	void Reserve(size_t new_capacity)
	{
		if (new_capacity <= capacity_)
		{
			return;
		}
		T* new_data = alloc_(new_capacity);
		for (size_t i = 0; i != size_; ++i)
		{
			copy_construct_(new_data + i, data_[i]);
		}
		destroy_n_(data_, size_);
		dealloc_(data_);

		data_ = new_data;
		capacity_ = new_capacity;
	}

	~Vector()
	{
		destroy_n_(data_, size_);
		dealloc_(data_);
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
	T* data_ = nullptr;
	size_t capacity_ = 0;
	size_t size_ = 0;

  private:
	// Выделяет сырую память под n элементов и возвращает указатель на неё
	static T* alloc_(size_t n)
	{
		return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
	}

	// Освобождает сырую память, выделенную ранее по адресу buf при помощи alloc_
	static void dealloc_(T* buf) noexcept
	{
		operator delete(buf);
	}

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