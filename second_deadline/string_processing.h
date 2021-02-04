#pragma once
#include <set>
#include <string>
#include <vector>

std::vector<std::string_view> SplitIntoWords(std::string_view text);

template <typename StringContainer> std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings)
{
	std::set<std::string> non_empty_strings;
	for (const auto& str : strings)
	{
		if (str.size() > 0)
		{
			non_empty_strings.insert(std::string(str));
		}
	}
	return non_empty_strings;
}