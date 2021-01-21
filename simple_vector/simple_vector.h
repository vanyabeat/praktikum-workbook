#pragma once

#include <cassert>
#include <initializer_list>
#include <iterator>


template<typename Type>
class SimpleVector {
public:
	using Iterator = Type *;
	using ConstIterator = const Type *;

	SimpleVector() noexcept = default;

	// Создаёт вектор из size элементов, инициализированных значением по умолчанию
	explicit SimpleVector(size_t size) {
		if (size) {
			size_ = size;
			capacity_ = size_;
			data = new Type[capacity_];
			std::fill(data, data + size, Type());
		}
	}

	SimpleVector(const SimpleVector<Type> &other) {
		size_ = other.size_;
		capacity_ = size_;
		data = new Type[capacity_];
		std::copy(other.begin(), other.end(), data);
	}

	// Обменивает содержимое списков за время O(1)
	void swap(SimpleVector<Type> &other) noexcept {
		std::swap(data, other.data);
		std::swap(size_, other.size_);
		std::swap(capacity_, other.capacity_);
	}

	friend void swap(SimpleVector<Type> &lhs, SimpleVector<Type> &rhs) {
		lhs.swap(rhs);
	}

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
		// если он пустой
		if (capacity_ == 0) {
			SimpleVector<Type> tmp(1);
			tmp.data[0] = value;
			swap(tmp);
		}
		// иначе
		else {
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


	// Создаёт вектор из size элементов, инициализированных значением value
	SimpleVector(size_t size, const Type &value) {
		if (size) {
			size_ = size;
			capacity_ = size_;
			data = new Type[capacity_];
			std::fill(data, data + size, value);
		}
	}

	// Создаёт вектор из std::initializer_list
	SimpleVector(std::initializer_list<Type> init) {
		if (init.size()) {
			size_ = init.size();
			capacity_ = size_;
			data = new Type[init.size()];
			std::copy(init.begin(), init.end(), data);
		}
	}

	~SimpleVector() {
		delete[] data;
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
		std::fill(data, data + capacity_, Type{});
	}

	// Изменяет размер массива.
	// При увеличении размера новые элементы получают значение по умолчанию для типа Type
	void Resize(size_t new_size) {
		if (new_size > capacity_) {
			size_t new_capacity = std::max(new_size, capacity_ * 2);
			//            ArrayPtr <Type> tmp(new_capacity);
			Type *new_data = new Type[new_capacity];
			std::fill(new_data, new_data + new_capacity, Type());
			std::copy(begin(), end(), new_data);
			capacity_ = new_capacity;

			Type *old_data = data;
			data = new_data;
			delete[] old_data;
			old_data = nullptr;
		} else if (new_size > size_) {
			std::fill(end(), begin() + new_size, Type{});
		}
		size_ = new_size;
	}

	// Возвращает итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	Iterator begin() noexcept {
		return data;
	}

	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	Iterator end() noexcept {
		if (size_) {
			return data + size_;
		} else {
			return nullptr;
		}
	}

	// Возвращает константный итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator begin() const noexcept {
		return data;
	}

	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator end() const noexcept {
		if (size_) {
			return data + size_;
		} else {
			return nullptr;
		}
	}

	// Возвращает константный итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator cbegin() const noexcept {
		return data;
	}

	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator cend() const noexcept {
		if (size_) {
			return data + size_;
		} else {
			return nullptr;
		}
	}

private:
	Type *data = nullptr;
	size_t size_ = 0;
	size_t capacity_ = 0;
};