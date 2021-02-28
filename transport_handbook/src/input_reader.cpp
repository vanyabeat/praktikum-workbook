#include "input_reader.h"
#include <regex>

std::string& ltrim(std::string& str)
{
	auto it2 =
		std::find_if(str.begin(), str.end(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
	str.erase(str.begin(), it2);
	return str;
}

std::string& rtrim(std::string& str)
{
	auto it1 =
		std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
	str.erase(it1.base(), str.end());
	return str;
}

std::string& trim(std::string& str)
{
	return ltrim(rtrim(str));
}

std::vector<std::string> SplitIntoWords(const std::string& text, std::vector<char> vec = {'>', '-'})
{
	std::vector<std::string> words;
	std::string word;
	auto functor_predicate = [&vec](char c) {
		for (auto t : vec)
		{
			if (c == t)
			{
				return true;
			}
		}
		return false;
	};

	for (const char c : text)
	{
		if (functor_predicate(c))
		{
			words.push_back(trim(word));
			word = "";
		}
		else
		{
			word += c;
		}
	}
	words.push_back(trim(word));

	return words;
}

std::vector<std::pair<std::string, size_t>> ParseDistances(const std::vector<std::string>& vec)
{
	using namespace std;
	std::vector<std::pair<std::string, size_t>> result;
	auto m = "m"s;
	auto to = "to"s;
	for (const auto& i : vec)
	{
		size_t m_pos = i.find(m);
		size_t to_pos = i.find(to);
		result.push_back({std::string(i.begin() + to_pos + to.size() + 1, i.end()),
						  std::stoi(std::string(i.begin(), i.begin() + m_pos))});
	}
	return result;
}

std::tuple<std::string, Coordinates, std::vector<std::pair<std::string, size_t>>> ParseStop(const std::string& r_str)
{
	using namespace std;
	if (r_str.find(" to "s) == r_str.npos)
	{
		std::regex str_expr("Stop (.*): ([0-9.-]*), ([0-9.-]*)");
		std::smatch base_match;
		std::regex_match(r_str, base_match, str_expr);

		return std::make_tuple(std::string(base_match[1]),
							   Coordinates{std::stod(base_match[2]), std::stod(base_match[3])},
							   std::vector<std::pair<std::string, size_t>>{});
	}
	else
	{
		std::regex str_expr("Stop (.*): ([0-9.-]*), ([0-9.-]*), (.*)");
		std::smatch base_match;
		std::regex_match(r_str, base_match, str_expr);
		std::vector<std::string> vec = SplitIntoWords(base_match[4], {','});

		return std::make_tuple(std::string(base_match[1]),
							   Coordinates{std::stod(base_match[2]), std::stod(base_match[3])}, ParseDistances(vec));
	}
}

Coordinates ParseCoordsSubstring(const std::string& r_str)
{
	// latitude, longitude
	using namespace std;
	size_t comma = r_str.find(","s);
	Coordinates result{std::stod(std::string(r_str.begin(), r_str.begin() + comma)),
					   std::stod(std::string(r_str.begin() + comma + 2, r_str.end()))};
	return result;
}

std::vector<std::string> FullPath(const std::string& r_str)
{
	using namespace std;
	bool full_cycled = (r_str.find(">"s) != r_str.npos);
	std::vector<std::string> result;
	if (full_cycled)
	{
		result = SplitIntoWords(r_str);
	}
	else
	{
		auto tmp = SplitIntoWords(r_str);
		result.insert(result.end(), tmp.begin(), tmp.end());
		tmp.pop_back();
		std::reverse(tmp.begin(), tmp.end());
		result.insert(result.end(), tmp.begin(), tmp.end());
	}
	return result;
}

Request* ParseRequestString(const std::string& r_str)
{
	using namespace std;
	// я пока не осилил смарт поинтеры и наверное паттерн абстрактная фабрика ?
	Request* result;
	// если это не Stop, то это Bus )
	size_t found_stop = r_str.find("Stop "s);
	size_t find_semicolon;
	std::string name;
	if (found_stop != r_str.npos)
	{
		result = new Stop();
		result->setRequestType(RequestType::IsStop);

		auto stop = ParseStop(r_str);

		name = std::get<0>(stop);
		static_cast<Stop*>(result)->coordinates = std::get<1>(stop);
		static_cast<Stop*>(result)->setDistanceToOtherStop(std::get<2>(stop));
	}
	else
	{
		result = new Bus();
		find_semicolon = r_str.find(":"s);
		name = std::string(r_str.begin() + "Bus "s.size(), r_str.begin() + find_semicolon);
		result->setRequestType(RequestType::IsBus);
		static_cast<Bus*>(result)->setStops(FullPath(std::string(r_str.begin() + find_semicolon + 2, r_str.end())));
	}
	result->setName(std::move(name));
	return result;
}

std::string ReadLine()
{
	std::string s;
	getline(std::cin, s);
	return s;
}

int ReadLineWithNumber()
{
	int result;
	std::cin >> result;
	ReadLine();
	return result;
}

RequestType Bus::getRequestType() const
{
	return type_;
}

void Bus::setRequestType(RequestType requestType)
{
	type_ = requestType;
}

const std::string& Bus::getName() const
{
	return name_;
}

void Bus::setName(const std::string& name)
{
	name_ = name;
}

const std::vector<std::string>& Bus::getStops() const
{
	return stops_;
}

void Bus::setStops(const std::vector<std::string>& stops)
{
	Bus::stops_ = stops;
}

RequestType Stop::getRequestType() const
{
	return type_;
}

void Stop::setRequestType(RequestType requestType)
{
	type_ = requestType;
}

const std::string& Stop::getName() const
{
	return name_;
}

void Stop::setName(const std::string& name)
{
	name_ = name;
}

const std::vector<std::pair<std::string, size_t>>& Stop::getDistanceToOtherStop() const
{
	return distance_to_other_stop;
}

void Stop::setDistanceToOtherStop(const std::vector<std::pair<std::string, size_t>>& distanceToOtherStop)
{
	distance_to_other_stop = distanceToOtherStop;
}

Requests::~Requests()
{
	for (auto r : requests)
	{
		delete r;
	}
}
