#pragma once
#include <vector>
#include <iterator>
#include "document.h"		/// не нашел использование структур из этого заголовочного файла

template<typename It>
class IteratorRange {
public:
	explicit IteratorRange(It begin, It end, size_t size) : begin_(begin), end_(end), size_(size) {}

	It begin() const {
		return begin_;
	}

	It end() const {
		return end_;
	}

	auto size() {		/// в данном месте лучше не использовать auto, если тип простой, то правильней его и использовать, метод не меняет поля, поэтому должен быть конкстантным
		return size_;
	}

private:
	It begin_;
	It end_;
	size_t size_{};
};

template<typename Iterator>
class Paginator {
public:
	explicit Paginator(Iterator b, Iterator e, size_t page_size) {
		auto d = distance(b, e);						/// как это работает без "std::"? просто любопытно
		// просто тупо будем определять кратно или не кратно)
		size_ = d / page_size + (d % page_size != 0 ? 1 : 0);

		for (int i = 0; i < size_; ++i) {
			auto b_it = next(b, page_size * i);				/// как это работает без "std::"? просто любопытно
			auto e_it = (i == size_ - 1 ? e : next(b_it, page_size));	/// как это работает без "std::"? просто любопытно
			IteratorRange<Iterator> page(b_it, e_it, std::distance(b_it, e_it));
			pages_.push_back(page);
		}
	}

	auto begin() const {
		return pages_.begin();
	}

	auto end() const {
		return pages_.end();
	}

	size_t size() {									/// метод не меняет поля класса, должен быть константным
		return std::distance(pages_.begin(), pages_.end());			/// зачем каждый раз высчитывать? почему не использовать pages_.size() ?
	}

private:
	std::vector<IteratorRange<Iterator>> pages_;
	size_t size_;									/// поле используется только как локальная пременная в конструкторе
};


template<typename Container>
auto Paginate(const Container &c, size_t page_size) {
	return Paginator(begin(c), end(c), page_size);
}

template<typename Iterator>
std::ostream &operator<<(std::ostream &out, IteratorRange<Iterator> range) {		/// лишнее копирование range
	for (auto document : range) {							/// лишнее копирование document
		out << document;
	}
	return out;
}