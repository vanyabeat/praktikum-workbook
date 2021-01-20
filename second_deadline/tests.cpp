#include "string_processing.h"
#include "gtest/gtest.h"
#include <string_view>

TEST(StringView, test)
{
	using namespace std;
	std::string str = "hello world"s;
	auto vec = SplitIntoWords(str);
	std::vector<std::string> expected = {"hello", "world"};
	ASSERT_EQ(expected, vec);
}

TEST(StringView, test1)
{
	using namespace std;
	std::string str = "hello world"s;
	auto vec = SplitIntoWordsView(str);
	std::vector<std::string_view> expected = {"hello"s, "world"s};
	ASSERT_EQ(expected, vec);
}

TEST(StringView, test2)
{
	using namespace std;
	std::string str = "hello world"s;
	auto vec = SplitIntoWordsView(str);
	std::vector<std::string_view> expected = {"hello"s, "world"s};
	ASSERT_EQ(expected, vec);
}

TEST(StringView, test3)
{
	using namespace std;
	std::string str = "hello     world"s;
	auto vec = SplitIntoWordsView(str);
	std::vector<std::string_view> expected = {"hello"s, {}, {}, {}, {}, "world"s};
	ASSERT_EQ(expected, vec);
}