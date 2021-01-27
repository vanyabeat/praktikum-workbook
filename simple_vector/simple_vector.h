#pragma once

#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <iterator>

class ProxyObject {
public:
	ProxyObject(size_t capacity) : capacity_(capacity) {}

	size_t GetCapacity() const {
		return capacity_;
	}

private:
	size_t capacity_;
};

template<typename Type>
class SimpleVector {
public:
	using Iterator = Type *;
	using ConstIterator = const Type *;

#pragma region Constructors
	SimpleVector() noexcept = default;

	SimpleVector(size_t size, const Type &value = Type{})
		: size_(size), capacity_(size), data_(size_) {
		std::fill(begin(), end(), value);
	}

	SimpleVector(const SimpleVector<Type> &other) : size_(other.GetSize()),
													capacity_(other.GetCapacity()),
													data_(size_) {
		std::copy(other.begin(), other.end(), begin());
	}

	SimpleVector(SimpleVector<Type> &&rvl) {
		swap(rvl);
	}

	SimpleVector(std::initializer_list<Type> init)
		: size_(init.size()),
		  capacity_(size_),
		  data_(size_) {
		std::copy(init.begin(), init.end(), begin());
	}

	SimpleVector(ProxyObject obj) : capacity_(obj.GetCapacity()), data_(obj.GetCapacity()) {
	}
#pragma endregion
#pragma region Swaps
	void swap(SimpleVector &other) noexcept {
		std::swap(size_, other.size_);
		std::swap(capacity_, other.capacity_);
		data_.swap(other.data_);
	}

	friend void swap(SimpleVector<Type> &lhs, SimpleVector<Type> &rhs) {
		lhs.swap(rhs);
	}
#pragma endregion
#pragma region Operators
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
		return data_[index];
	}

	// Возвращает константную ссылку на элемент с индексом index
	const Type &operator[](size_t index) const noexcept {
		//        assert(index >= 0 && index < size_);
		return data_[index];
	}
#pragma endregion
#pragma PushsAndInserts
	void PushBack(const Type &item) {
		Type copy_item = item;
		PushBack(std::move(copy_item));
	}

	void PushBack(Type &&item) {
		Insert(end(), std::move(item));
	}

	void PopBack() noexcept {
		if (!IsEmpty()) {
			--size_;
		}
	}


	/// предлагая пересмотреть реализации копирующего и перемещаящего Insert
	/// 1. код у них должен быть полностью идентичным, отличается он только перемещением или копированием всего одного нового элемента.
	///    поэтому замечания пишу только для первого метода, хотя вариант реализации у перемещаемого Insert более интересна
	/// 2. раз код оплностью должен быть идентичным, то значит есть проблема повтора кода и ее нужно решать, могу предлодить 3 варианта
	///	1. общую часть можно выделить в отдельный приватный метод
	/// 3. повозможности пльзуйтесь уже имеющимеся методами по изменения размерности (Reserve, Resize)

	Iterator Insert(ConstIterator pos, const Type &value) {
		Type copy_value = value;
		return Insert(pos, std::move(copy_value));// красивое и лаконичное решение
	}

	Iterator Insert(ConstIterator pos, Type &&value) {
		size_t n = pos - cbegin();
		if (capacity_ == 0) {
			Reserve(1);
		}
		if (size_ == capacity_) {
			capacity_ *= 2;
		}
		ArrayPtr<Type> temp(capacity_);
		std::move(begin(), begin() + n, temp.Get());
		std::move_backward(begin() + n, begin() + size_, temp.Get() + size_ + 1);
		temp[n] = std::move(value);
		data_.swap(temp);
		size_++;
		return begin() + n;
	}

	// Удаляет элемент вектора в указанной позиции
	Iterator Erase(ConstIterator pos) {
		assert(pos != end());
		size_t index = pos - cbegin();
		if (!IsEmpty()) {
			std::move(begin() + index + 1, end(), begin() + index);
			--size_;
		}
		return begin() + index;
	}
#pragma endregion

	// Возвращает количество элементов в массиве
	size_t GetSize() const noexcept {
		return size_;
	}

	// Возвращает вместимость массива
	size_t GetCapacity() const noexcept {
		return capacity_;
	}

	bool IsEmpty() const noexcept {
		return (size_ == 0);
	}

	Type &At(size_t index) {
		if (index > size_) {
			throw std::out_of_range("Invalid index");
		} else {
			return data_[index];
		}
	}

	// Возвращает константную ссылку на элемент с индексом index
	// Выбрасывает исключение std::out_of_range, если index >= size
	const Type &At(size_t index) const {
		if (index > size_) {
			throw std::out_of_range("Invalid index");
		} else {
			return data_[index];
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
			Reserve(new_capacity);
		}
		std::fill(end(), begin() + new_size, Type{});
		size_ = new_size;
	}

	void Reserve(size_t new_capacity) {
		if (new_capacity > capacity_) {
			ArrayPtr<Type> tmp(new_capacity);
			std::move(begin(), end(), tmp.Get());
			data_.swap(tmp);
			capacity_ = new_capacity;
		}
	}
#pragma region Iterators
	Iterator begin() noexcept {
		return data_.Get();
	}

	Iterator end() noexcept {
		return data_.Get() + size_;
	}

	ConstIterator begin() const noexcept {
		return data_.Get();
	}

	ConstIterator end() const noexcept {
		return data_.Get() + size_;
	}

	ConstIterator cbegin() const noexcept {
		return begin();
	}

	ConstIterator cend() const noexcept {
		return end();
	}
#pragma endregion
private:
	size_t size_ = 0;
	size_t capacity_ = 0;
	ArrayPtr<Type> data_;
};

ProxyObject Reserve(size_t capacity_to_reserve) {
	return ProxyObject(capacity_to_reserve);
}