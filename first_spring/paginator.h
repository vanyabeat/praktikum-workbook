#pragma once
#include <iterator>
#include <vector>

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

	size_t size() const {
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
		auto d = std::distance(b, e);
		// просто тупо будем определять кратно или не кратно)
		size_t size_ = d / page_size + (d % page_size != 0 ? 1 : 0);

		for (int i = 0; i < size_; ++i) {
			auto b_it = std::next(b, page_size * i);// Сам не понял как очепятка работает !?
			auto e_it = (i == size_ - 1 ? e : std::next(b_it, page_size));
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

	size_t size() const {
		return pages_.size();
	}

private:
	std::vector<IteratorRange<Iterator>> pages_;
};


template<typename Container>
auto Paginate(const Container &c, size_t page_size) {
	return Paginator(begin(c), end(c), page_size);
}

template<typename Iterator>
std::ostream &operator<<(std::ostream &out, IteratorRange<Iterator> &range) {
	for (auto &document : range) {	/// ссылка должна быть константной
		out << document;
	}
	return out;
}