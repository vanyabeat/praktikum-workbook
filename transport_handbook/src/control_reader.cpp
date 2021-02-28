#include "control_reader.h"
#include <regex>

static std::string& ltrim(std::string& str)
{
	auto it2 =
		std::find_if(str.begin(), str.end(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
	str.erase(str.begin(), it2);
	return str;
}

static std::string& rtrim(std::string& str)
{
	auto it1 =
		std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
	str.erase(it1.base(), str.end());
	return str;
}

static std::string& trim(std::string& str)
{
	return ltrim(rtrim(str));
}

static std::vector<std::string> SplitIntoWords(const std::string& text, std::vector<char> vec = {'>', '-'})
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

static std::vector<std::pair<std::string, size_t>> ParseDistances(const std::vector<std::string>& vec)
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

static std::tuple<std::string, Handbook::Utilities::Coordinates, std::vector<std::pair<std::string, size_t>>> ParseStop(
	const std::string& r_str)
{
	using namespace std;
	if (r_str.find(" to "s) == r_str.npos)
	{
		std::regex str_expr("Stop (.*): ([0-9.-]*), ([0-9.-]*)");
		std::smatch base_match;
		std::regex_match(r_str, base_match, str_expr);

		return std::make_tuple(std::string(base_match[1]),
							   Handbook::Utilities::Coordinates{std::stod(base_match[2]), std::stod(base_match[3])},
							   std::vector<std::pair<std::string, size_t>>{});
	}
	else
	{
		std::regex str_expr("Stop (.*): ([0-9.-]*), ([0-9.-]*), (.*)");
		std::smatch base_match;
		std::regex_match(r_str, base_match, str_expr);
		std::vector<std::string> vec = SplitIntoWords(base_match[4], {','});

		return std::make_tuple(std::string(base_match[1]),
							   Handbook::Utilities::Coordinates{std::stod(base_match[2]), std::stod(base_match[3])}, ParseDistances(vec));
	}
}

static Handbook::Utilities::Coordinates ParseCoordsSubstring(const std::string& r_str)
{
	// latitude, longitude
	using namespace std;
	size_t comma = r_str.find(","s);
	Handbook::Utilities::Coordinates result{std::stod(std::string(r_str.begin(), r_str.begin() + comma)),
					   std::stod(std::string(r_str.begin() + comma + 2, r_str.end()))};
	return result;
}

static std::vector<std::string> FullPath(const std::string& r_str)
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

std::shared_ptr<Handbook::Control::Request> Handbook::Control::ParseRequestString(const std::string& r_str)
{
	using namespace std;
	// я пока не осилил смарт поинтеры и наверное паттерн абстрактная фабрика ?
	std::shared_ptr<Handbook::Control::Request> result = nullptr;
	// если это не Stop, то это Bus )
	size_t found_stop = r_str.find("Stop "s);
	size_t find_semicolon;
	std::string name;
	if (found_stop != r_str.npos)
	{
		result = std::shared_ptr<Handbook::Control::Request>(new Handbook::Control::Stop());
		result->setRequestType(Handbook::Control::RequestType::IsStop);

		auto stop = ParseStop(r_str);

		name = std::get<0>(stop);
		static_cast<Handbook::Control::Stop*>(result.get())->coordinates = std::get<1>(stop);
		static_cast<Handbook::Control::Stop*>(result.get())->setDistanceToOtherStop(std::get<2>(stop));
	}
	else
	{
		result = std::shared_ptr<Handbook::Control::Request>(new Handbook::Control::Bus());
		find_semicolon = r_str.find(":"s);
		name = std::string(r_str.begin() + "Bus "s.size(), r_str.begin() + find_semicolon);
		result->setRequestType(Handbook::Control::RequestType::IsBus);
		static_cast<Handbook::Control::Bus*>(result.get())
			->setStops(FullPath(std::string(r_str.begin() + find_semicolon + 2, r_str.end())));
	}
	result->setName(std::move(name));
	return result;
}

std::string Handbook::Control::ReadLine(std::istream& istream)
{
	std::string s;
	getline(std::cin, s);
	return s;
}

int Handbook::Control::ReadLineWithNumber(std::istream& istream)
{
	int result;
	istream >> result;
	ReadLine(istream);
	return result;
}

Handbook::Control::RequestType Handbook::Control::Bus::getRequestType() const
{
	return type_;
}

void Handbook::Control::Bus::setRequestType(RequestType requestType)
{
	type_ = requestType;
}

const std::string& Handbook::Control::Bus::getName() const
{
	return name_;
}

void Handbook::Control::Bus::setName(const std::string& name)
{
	name_ = name;
}

const std::vector<std::string>& Handbook::Control::Bus::getStops() const
{
	return stops_;
}

void Handbook::Control::Bus::setStops(const std::vector<std::string>& stops)
{
	Bus::stops_ = stops;
}

Handbook::Control::RequestType Handbook::Control::Stop::getRequestType() const
{
	return type_;
}

void Handbook::Control::Stop::setRequestType(RequestType requestType)
{
	type_ = requestType;
}

const std::string& Handbook::Control::Stop::getName() const
{
	return name_;
}

void Handbook::Control::Stop::setName(const std::string& name)
{
	name_ = name;
}

const std::vector<std::pair<std::string, size_t>>& Handbook::Control::Stop::getDistanceToOtherStop() const
{
	return distance_to_other_stop;
}

void Handbook::Control::Stop::setDistanceToOtherStop(
	const std::vector<std::pair<std::string, size_t>>& distanceToOtherStop)
{
	distance_to_other_stop = distanceToOtherStop;
}

void Handbook::Control::AddRequestToCatalogue(Handbook::Control::Request* request,
											  Handbook::Data::TransportCatalogue& transport_catalogue)
{
	switch (request->getRequestType())
	{
	case Handbook::Control::RequestType::IsBus: {
		Handbook::Control::Bus* bus = static_cast<Handbook::Control::Bus*>(request);
		transport_catalogue.AddBus(bus->getName(), bus->getStops());
		break;
	}
	case Handbook::Control::RequestType::IsStop: {
	}
		Handbook::Control::Stop* stop = static_cast<Handbook::Control::Stop*>(request);
		transport_catalogue.AddStop(stop->getName(), stop->coordinates, stop->getDistanceToOtherStop());
		break;
	}
}