#include <istream>
#include <ostream>

class StreamUntier {
public:
	StreamUntier(std::istream &stream) : stream_(stream) {
		tied_before_ = stream_.tie(nullptr);
	}
	~StreamUntier() {
		stream_.tie(tied_before_);
	}

private:
	std::ostream *tied_before_;
	std::istream &stream_;
};