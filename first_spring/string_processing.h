#pragma once
#include <string>
#include <vector>
#include <set>
std::vector<std::string> SplitIntoWords(const std::string &text);

template<typename StringContainer>
static std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer &strings) {
	std::set<std::string> non_empty_strings;
	for (const auto &str : strings) {
		if (!str.empty()) {
			non_empty_strings.insert(str);
		}
	}
	return non_empty_strings;
}