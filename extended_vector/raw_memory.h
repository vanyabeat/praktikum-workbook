#pragma once
#include <cstdlib>
#include <memory>

template <typename T> class RawMemory
{
  public:
	RawMemory() = default;

	explicit RawMemory(size_t capacity) : buffer_(Allocate(capacity)), capacity_(capacity)
	{
	}
	RawMemory(const RawMemory&) = delete;
	RawMemory& operator=(const RawMemory& rhs) = delete;
	RawMemory(RawMemory&& other) noexcept
	{
		buffer_ = other.buffer_;
		other.buffer_ = nullptr;

		capacity_ = other.capacity_;
		other.capacity_ = 0;
	}

	RawMemory& operator=(RawMemory&& rhs) noexcept
	{
		buffer_ = rhs.buffer_;
		rhs.buffer_ = nullptr;

		capacity_ = rhs.capacity_;
		rhs.capacity_ = 0;

		return *this;
	}
	~RawMemory()
	{
		Deallocate(buffer_);
	}

	T* operator+(size_t offset) noexcept
	{
		// Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
		assert(offset <= capacity_);
		return buffer_ + offset;
	}

	const T* operator+(size_t offset) const noexcept
	{
		return const_cast<RawMemory&>(*this) + offset;
	}

	const T& operator[](size_t index) const noexcept
	{
		return const_cast<RawMemory&>(*this)[index];
	}

	T& operator[](size_t index) noexcept
	{
		assert(index < capacity_);
		return buffer_[index];
	}

	void Swap(RawMemory& other) noexcept
	{
		std::swap(buffer_, other.buffer_);
		std::swap(capacity_, other.capacity_);
	}

	const T* GetAddress() const noexcept
	{
		return buffer_;
	}

	T* GetAddress() noexcept
	{
		return buffer_;
	}

	size_t Capacity() const
	{
		return capacity_;
	}

  private:
	// Выделяет сырую память под n элементов и возвращает указатель на неё
	static T* Allocate(size_t n)
	{
		return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
	}

	// Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
	static void Deallocate(T* buf) noexcept
	{
		operator delete(buf);
	}

	T* buffer_ = nullptr;
	size_t capacity_ = 0;
};