#pragma once

#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <iterator>

class ProxyObject {
public:
	ProxyObject(size_t capacity) : capacity_(capacity) {}

	size_t GetCapacity() {
		return capacity_;
	}

private:
	size_t capacity_;
};

template<typename Type>
class SimpleVector {
public:
public:
	using Iterator = Type *;
	using ConstIterator = const Type *;

	//#pragma region Constructors
	SimpleVector() noexcept = default;

	SimpleVector(size_t size, const Type &value = Type{})
		: size_(size), capacity_(size), data(size_) {
		std::fill(begin(), end(), value);
	}

	SimpleVector(const SimpleVector<Type> &other) : size_(other.GetSize()),
													capacity_(other.GetCapacity()),
													data(size_) {
		std::copy(other.begin(), other.end(), begin());
	}

	SimpleVector(std::initializer_list<Type> init)
		: size_(init.size()),
		  capacity_(size_),
		  data(size_) {
		std::copy(init.begin(), init.end(), begin());
	}

	SimpleVector(ProxyObject obj) {
		capacity_ = obj.GetCapacity();
	}
	//#pragma endregion
	//#pragma region Swaps
	void swap(SimpleVector &other) noexcept {
		std::swap(size_, other.size_);
		std::swap(capacity_, other.capacity_);
		data.swap(other.data);
	}

	friend void swap(SimpleVector<Type> &lhs, SimpleVector<Type> &rhs) {
		lhs.swap(rhs);
	}
//#pragma endregion
//#pragma region Operators
	SimpleVector &operator=(const SimpleVector<Type> &other) {
		if (this != &other) {
			if (other.IsEmpty()) {
				Clear();
			} else {
				auto copy(other);
				swap(copy);
			}
		}
		return *this;
	}

	friend bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	friend bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {

		return !(lhs == rhs);
	}

	friend bool operator<(const SimpleVector &lhs, const SimpleVector &rhs) {

		return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
	}


	friend bool operator>(const SimpleVector &lhs, const SimpleVector &rhs) {

		return (rhs < lhs);
	}

	friend bool operator<=(const SimpleVector &lhs, const SimpleVector &rhs) {
		return !(rhs < lhs);
	}

	friend bool operator>=(const SimpleVector &lhs, const SimpleVector &rhs) {
		return !(lhs < rhs);
	}

	// Возвращает ссылку на элемент с индексом index
	Type &operator[](size_t index) noexcept {
		//        assert(index >= 0 && index < size_);
		return data[index];
	}

	// Возвращает константную ссылку на элемент с индексом index
	const Type &operator[](size_t index) const noexcept {
		//        assert(index >= 0 && index < size_);
		return data[index];
	}
	//#pragma endregion

	// Добавляет элемент в конец вектора
	// При нехватке места увеличивает вдвое вместимость вектора
	void PushBack(const Type &item) {
		if (capacity_ == 0) {
			SimpleVector<Type> tmp(1);
			tmp.data[0] = item;
			swap(tmp);
		} else {
			if (size_ < capacity_) {
				data[size_] = item;
				++size_;
			} else {
				Resize(2 * capacity_);
				size_ = size_ / 2 + 1;
				data[GetSize() - 1] = item;
			}
		}
	}

	void PopBack() noexcept {
		if (!IsEmpty()) {
			--size_;
		}
	}

	// Вставляет значение value в позицию pos.
	// Возвращает итератор на вставленное значение
	// Если перед вставкой значения вектор был заполнен полностью,
	// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
	Iterator Insert(ConstIterator pos, const Type &value) {
		size_t npos = pos - cbegin();
		if (capacity_ == 0) {
			SimpleVector<Type> tmp(1);
			tmp.data[0] = value;
			swap(tmp);
		} else {
			SimpleVector<Type> tmp(capacity_);
			if (size_ == capacity_) {
				tmp.Resize(2 * capacity_);
			}
			tmp.size_ = size_ + 1;
			std::copy(cbegin(), pos, tmp.begin());
			tmp.data[npos] = value;
			std::copy(pos, cend(), tmp.begin() + npos + 1);
			swap(tmp);
		}
		return begin() + npos;
	}

	// Удаляет элемент вектора в указанной позиции
	Iterator Erase(ConstIterator pos) {
		size_t index = pos - cbegin();
		if (!IsEmpty()) {
			std::move(begin() + index + 1, end(), begin() + index);
			--size_;
		}
		return begin() + index;
	}

	// Возвращает количество элементов в массиве
	size_t GetSize() const noexcept {
		// Напишите тело самостоятельно
		return size_;
	}

	// Возвращает вместимость массива
	size_t GetCapacity() const noexcept {
		// Напишите тело самостоятельно
		return capacity_;
	}

	// Сообщает, пустой ли массив
	bool IsEmpty() const noexcept {
		return (size_ == 0);
	}


	// Возвращает константную ссылку на элемент с индексом index
	// Выбрасывает исключение std::out_of_range, если index >= size
	Type &At(size_t index) {
		if (index < size_) {
			return data[index];
		} else {
			throw std::out_of_range("Invalid index");
		}
	}

	// Возвращает константную ссылку на элемент с индексом index
	// Выбрасывает исключение std::out_of_range, если index >= size
	const Type &At(size_t index) const {
		if (index < size_) {
			return data[index];
		} else {
			throw std::out_of_range("Invalid index");
		}
	}

	// Обнуляет размер массива, не изменяя его вместимость
	void Clear() noexcept {
		size_ = 0;
	}

	// Изменяет размер массива.
	// При увеличении размера новые элементы получают значение по умолчанию для типа Type
	void Resize(size_t new_size) {
		if (new_size > capacity_) {
			size_t new_capacity = std::max(new_size, capacity_ * 2);
			ArrayPtr<Type> tmp(new_capacity);
			std::copy(begin(), end(), tmp.Get());
			data.swap(tmp);
			std::fill(end(), begin() + (new_capacity - capacity_), Type{});
			capacity_ = new_capacity;
		} else if (new_size > size_) {
			std::fill(end(), begin() + new_size, Type{});
		}

		size_ = new_size;
	}

	void Reserve(size_t new_capacity) {
		if (new_capacity > capacity_) {
			ArrayPtr<Type> tmp(new_capacity);
			std::copy(begin(), end(), tmp.Get());
			data.swap(tmp);
			capacity_ = new_capacity;
		}
	}
	//#pragma region Iterators
	Iterator begin() noexcept {
		return data.Get();
	}

	Iterator end() noexcept {
		return data.Get() + size_;
	}

	ConstIterator begin() const noexcept {
		return data.Get();
	}

	ConstIterator end() const noexcept {
		return data.Get() + size_;
	}

	ConstIterator cbegin() const noexcept {
		return begin();
	}

	ConstIterator cend() const noexcept {
		return end();
	}
	//#pragma endregion
private:
	size_t size_ = 0;
	size_t capacity_ = 0;
	ArrayPtr<Type> data;
};

ProxyObject Reserve(size_t capacity_to_reserve) {
	return ProxyObject(capacity_to_reserve);
}