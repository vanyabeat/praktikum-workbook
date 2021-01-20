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


std::vector<std::string_view> SplitIntoWordsView(std::string_view str)
{
	using namespace std;
	std::vector<std::string_view> result;
	size_t first = 0;

	while (first != std::string_view::npos)
	{
		first = str.find_first_of(" "s, first);

		if (first != str.size())
			result.emplace_back(str.substr(0, first));

		str.remove_prefix(first + 1);
	}

	return result;
}