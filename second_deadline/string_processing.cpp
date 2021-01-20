#include "string_processing.h"

std::vector<std::string> SplitIntoWords(const std::string &text) {
	std::vector<std::string> words;
	std::string word;
	for (const char c : text) {
		if (c == ' ') {
			words.push_back(word);
			word = "";
		} else {
			word += c;
		}
	}
	words.push_back(word);

	return words;
}


std::vector<std::string_view> SplitIntoWordsView(std::string_view str) {
	std::vector<std::string_view> result;
	const int64_t end = std::string_view::npos;
	while (true) {
		int64_t space = str.find(' ');
		result.push_back(str.substr(0, space));
		if (space == end) {
			break;
		} else {
			str.remove_prefix(space + 1);
		}

	}
	return result;
}