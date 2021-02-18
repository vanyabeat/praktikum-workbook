#include "input_reader.h"
#include "regex"
#include <gtest/gtest.h>
#include <iostream>
#include <transport_catalogue.h>
int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST(Stop, Test)
{
	using namespace std;
	std::string parsable_string = "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"s;

	Request* a = ParseRequestString(parsable_string);
	ASSERT_EQ(a->getRequestType(), RequestType::IsStop);
	ASSERT_EQ(a->getName(), "Tolstopaltsevo"s);

	ASSERT_DOUBLE_EQ(static_cast<Stop*>(a)->coordinates.lat, 55.611087);
	ASSERT_DOUBLE_EQ(static_cast<Stop*>(a)->coordinates.lng, 37.208290);
	delete a;
}

TEST(Stop, Test2)
{
	using namespace std;
	std::string parsable_string = "Stop Tolstopaltsevo: 55.611087, 37.20829"s;

	Request* a = ParseRequestString(parsable_string);
	ASSERT_EQ(a->getRequestType(), RequestType::IsStop);
	ASSERT_EQ(a->getName(), "Tolstopaltsevo"s);

	ASSERT_DOUBLE_EQ(static_cast<Stop*>(a)->coordinates.lat, 55.611087);
	ASSERT_DOUBLE_EQ(static_cast<Stop*>(a)->coordinates.lng, 37.208290);
	delete a;
}

TEST(Bus, Test)
{
	using namespace std;
	std::string parsable_string =
		"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s;

	Request* a = ParseRequestString(parsable_string);
	ASSERT_EQ(a->getRequestType(), RequestType::IsBus);
	ASSERT_EQ(a->getName(), "256"s);
	auto stops = static_cast<Bus*>(a)->getStops();
	ASSERT_EQ(stops,
			  (std::vector<std::string>{"Biryulyovo Zapadnoye", "Biryusinka", "Universam", "Biryulyovo Tovarnaya",
										"Biryulyovo Passazhirskaya", "Biryulyovo Zapadnoye"}));
	delete a;
}

TEST(Bus, Test2)
{
	using namespace std;
	std::string parsable_string = "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"s;

	Request* a = ParseRequestString(parsable_string);
	ASSERT_EQ(a->getRequestType(), RequestType::IsBus);
	ASSERT_EQ(a->getName(), "750"s);
	auto stops = static_cast<Bus*>(a)->getStops();
	auto expected =
		std::vector<std::string>{"Tolstopaltsevo", "Marushkino", "Rasskazovka", "Marushkino", "Tolstopaltsevo"};
	ASSERT_EQ(stops, expected);
	delete a;
}

TEST(Catalogue, Test1)
{
	using namespace std;
	std::vector<std::string> reqs = {
		"Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"s,
		"Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"s,
		"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s,
		"Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka"s,
		"Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino"s,
		"Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam"s,
		"Stop Biryusinka: 55.581065, 37.64839, 750m to Universam"s,
		"Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya"s,
		"Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya"s,
		"Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye"s,
		"Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s,
		"Stop Rossoshanskaya ulitsa: 55.595579, 37.605757"s,
		"Stop Prazhskaya: 55.611678, 37.603831"s};
	auto requests = Requests(reqs.size());
	TransportCatalogue transport_catalogue;
	for (const auto& r : reqs)
	{
		Request* req = ParseRequestString(r);
		requests.requests.push_back(req);
		transport_catalogue.AddRequest(req);
	}
	int a = 5;
}
