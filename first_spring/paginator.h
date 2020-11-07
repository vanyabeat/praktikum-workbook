#pragma once
#include <iterator>
#include "document.h"

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

	auto size() {
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
		auto d = distance(b, e);
		// просто тупо будем определять кратно или не кратно)
		size_ = d / page_size + (d % page_size != 0 ? 1 : 0);

		for (int i = 0; i < size_; ++i) {
			auto b_it = next(b, page_size * i);
			auto e_it = (i == size_ - 1 ? e : next(b_it, page_size));
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

	size_t size() {
		return std::distance(pages_.begin(), pages_.end());
	}

private:
	std::vector<IteratorRange<Iterator>> pages_;
	size_t size_;
};


template<typename Container>
auto Paginate(const Container &c, size_t page_size) {
	return Paginator(begin(c), end(c), page_size);
}

template<typename Iterator>
std::ostream &operator<<(std::ostream &out, IteratorRange<Iterator> range) {
	for (auto document : range) {
		out << document;
	}
	return out;
}