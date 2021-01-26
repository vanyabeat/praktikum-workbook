#pragma once

#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <iterator>

class ProxyObject {
public:
	ProxyObject(size_t capacity) : capacity_(capacity) {}

	size_t GetCapacity() {	/// метод не меняет объект, пожтому должен быть константныйм
		return capacity_;
	}

private:
	size_t capacity_;
};

template<typename Type>
class SimpleVector {
public:
public:	/// повторныый public: можно подчистить
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

	SimpleVector(SimpleVector<Type> &&rvl) {
		swap(rvl);
	}

	SimpleVector(std::initializer_list<Type> init)
		: size_(init.size()),
		  capacity_(size_),
		  data(size_) {
		std::copy(init.begin(), init.end(), begin());
	}

	SimpleVector(ProxyObject obj) {
		capacity_ = obj.GetCapacity();	/// этого не достаточно, выделланя память в data должна равная capacity_ и переместите инициализацию в список инициализации
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
/// предлагаю пересмотреть реализацию для проверки и изменения размера у вас есть методы Resize, получается тут дублирование кода
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
///	2. можно копирующий метод реализовать через перемещаемый с копией во временной переменной: f(const T & a) { T copy_a = a; return f(std::move(copy_a)); }
///	3. более правильный испольщзовать std::forward, но его пока что не советую использовать, вы его еще не проходили
/// 3. повозможности пльзуйтесь уже имеющимеся методами по изменения размерности (Reserve, Resize)
	// Вставляет значение value в позицию pos.
	// Возвращает итератор на вставленное значение
	// Если перед вставкой значения вектор был заполнен полностью,
	// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
	Iterator Insert(ConstIterator pos, const Type &value) {
		size_t npos = pos - cbegin();				/// далее не меняется должна быть const
		if (capacity_ == 0) {					/// нет смысла выделять данный случай, по сути это тоже Resive
			SimpleVector<Type> tmp(1);
			tmp.data[0] = value;
			swap(tmp);
		} else {
			SimpleVector<Type> tmp(capacity_);
			if (size_ == capacity_) {
				tmp.Resize(2 * capacity_);
			}
			tmp.size_ = size_ + 1;
			std::copy(cbegin(), pos, tmp.begin());		/// значения в старом контейнере уже будут не нужны, правильней использовать std::move
			tmp.data[npos] = value;
			std::copy(pos, cend(), tmp.begin() + npos + 1);
			swap(tmp);
		}
		return begin() + npos;
	}

	Iterator Insert(ConstIterator pos, Type&& value) {
		size_t n = pos - cbegin();
		if (capacity_ == 0) {
			ArrayPtr<Type> temp(1);
			temp[0] = std::move(value);
			data.swap(temp);
			capacity_ = 1;
		} else {
			if (size_ == capacity_) {
				capacity_ *= 2;
			}
			ArrayPtr<Type> temp(capacity_);
			std::move(begin(), begin() + n, temp.Get());
			std::move_backward(begin() + n, begin() + size_ , temp.Get() + size_ + 1);
			temp[n] = std::move(value);
			data.swap(temp);
		}
		size_++;
		return begin() + n;
	}

	// Удаляет элемент вектора в указанной позиции
	Iterator Erase(ConstIterator pos) {	/// желательно добавить обработку (условеи или assert) варианта pos == end()
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
	Type &At(size_t index) {					/// не замечание, рекомендация, лучше перевернуть условие
		if (index < size_) {					/// if (...)
			return data[index];				///	throw ;
		} else {						/// далее основной код
			throw std::out_of_range("Invalid index");	/// в таком виде код более читаемый
		}
	}

	// Возвращает константную ссылку на элемент с индексом index
	// Выбрасывает исключение std::out_of_range, если index >= size
	const Type &At(size_t index) const {				///    ---//---
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
			ArrayPtr<Type> tmp(new_capacity);	/// с этого места повтор кода с Reserve, воспользуйтесь этим методом
			std::copy(begin(), end(), tmp.Get());
			data.swap(tmp);
			std::fill(end(), begin() + (new_capacity - capacity_), Type{}); /// нет смысла обнулять новые ячейки, а только до new_size
			capacity_ = new_capacity;
		} else if (new_size > size_) {
			std::fill(end(), begin() + new_size, Type{});
		}

		size_ = new_size;
	}

	void Reserve(size_t new_capacity) {
		if (new_capacity > capacity_) {
			ArrayPtr<Type> tmp(new_capacity);
			std::copy(begin(), end(), tmp.Get());	/// в старом объекте значения ячеек уже будут не нужны, лучше использовать std::move
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
	ArrayPtr<Type> data;	/// следуйте порядку наименования, в проете поля классов должны заканчиваться подчеркиванием (data_)
};

ProxyObject Reserve(size_t capacity_to_reserve) {
	return ProxyObject(capacity_to_reserve);
}