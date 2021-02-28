#include "control_reader.h"
#include "regex"
#include "view_data.h"
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

	std::shared_ptr<Handbook::Control::Request> a = Handbook::Control::ParseRequestString(parsable_string);
	ASSERT_EQ(a->getRequestType(), Handbook::Control::RequestType::IsStop);
	ASSERT_EQ(a->getName(), "Tolstopaltsevo"s);

	ASSERT_DOUBLE_EQ(static_cast<Handbook::Control::Stop*>(a.get())->coordinates.lat, 55.611087);
	ASSERT_DOUBLE_EQ(static_cast<Handbook::Control::Stop*>(a.get())->coordinates.lng, 37.208290);
}

TEST(Stop, Test1)
{
	using namespace std;
	std::string parsable_string = "Stop Tolstopaltsevo: -55.611087, 37.20829, 3900m to Marushkino"s;

	std::shared_ptr<Handbook::Control::Request> a = Handbook::Control::ParseRequestString(parsable_string);
	ASSERT_EQ(a->getRequestType(), Handbook::Control::RequestType::IsStop);
	ASSERT_EQ(a->getName(), "Tolstopaltsevo"s);

	ASSERT_DOUBLE_EQ(static_cast<Handbook::Control::Stop*>(a.get())->coordinates.lat, -55.611087);
	ASSERT_DOUBLE_EQ(static_cast<Handbook::Control::Stop*>(a.get())->coordinates.lng, 37.208290);
}

TEST(Stop, Test2)
{
	using namespace std;
	std::string parsable_string = "Stop Tolstopaltsevo: -55.611087, -37.20829, 3900m to Marushkino"s;

	std::shared_ptr<Handbook::Control::Request> a = Handbook::Control::ParseRequestString(parsable_string);
	ASSERT_EQ(a->getRequestType(), Handbook::Control::RequestType::IsStop);
	ASSERT_EQ(a->getName(), "Tolstopaltsevo"s);

	ASSERT_DOUBLE_EQ(static_cast<Handbook::Control::Stop*>(a.get())->coordinates.lat, -55.611087);
	ASSERT_DOUBLE_EQ(static_cast<Handbook::Control::Stop*>(a.get())->coordinates.lng, -37.208290);
}

TEST(Stop, Test3)
{
	using namespace std;
	std::string parsable_string = "Stop Tolstopaltsevo: 55.611087, 37.20829"s;

	std::shared_ptr<Handbook::Control::Request> a = Handbook::Control::ParseRequestString(parsable_string);
	ASSERT_EQ(a->getRequestType(), Handbook::Control::RequestType::IsStop);
	ASSERT_EQ(a->getName(), "Tolstopaltsevo"s);

	ASSERT_DOUBLE_EQ(static_cast<Handbook::Control::Stop*>(a.get())->coordinates.lat, 55.611087);
	ASSERT_DOUBLE_EQ(static_cast<Handbook::Control::Stop*>(a.get())->coordinates.lng, 37.208290);
}

TEST(Bus, Test1)
{
	using namespace std;
	std::string parsable_string =
		"Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s;

	std::shared_ptr<Handbook::Control::Request> a = Handbook::Control::ParseRequestString(parsable_string);
	ASSERT_EQ(a->getRequestType(), Handbook::Control::RequestType::IsBus);
	ASSERT_EQ(a->getName(), "256"s);
	auto stops = static_cast<Handbook::Control::Bus*>(a.get())->getStops();
	ASSERT_EQ(stops,
			  (std::vector<std::string>{"Biryulyovo Zapadnoye", "Biryusinka", "Universam", "Biryulyovo Tovarnaya",
										"Biryulyovo Passazhirskaya", "Biryulyovo Zapadnoye"}));
}

TEST(Bus, Test2)
{
	using namespace std;
	std::string parsable_string = "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"s;

	std::shared_ptr<Handbook::Control::Request> a = Handbook::Control::ParseRequestString(parsable_string);
	ASSERT_EQ(a->getRequestType(), Handbook::Control::RequestType::IsBus);
	ASSERT_EQ(a->getName(), "750"s);
	auto stops = static_cast<Handbook::Control::Bus*>(a.get())->getStops();
	auto expected =
		std::vector<std::string>{"Tolstopaltsevo", "Marushkino", "Rasskazovka", "Marushkino", "Tolstopaltsevo"};
	ASSERT_EQ(stops, expected);
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
	Handbook::Data::TransportCatalogue transport_catalogue;
	for (const auto& r : reqs)
	{
		std::shared_ptr<Handbook::Control::Request> req = Handbook::Control::ParseRequestString(r);
		AddRequestToCatalogue(req.get(), transport_catalogue);
	}
	std::vector<std::string> stats = {"Bus 256"s,	  "Bus 750"s,		  "Bus 751"s,
									  "Stop Samara"s, "Stop Prazhskaya"s, "Stop Biryulyovo Zapadnoye"s};
	//	std::vector<std::string> stats = {"Bus 750"s};

	ASSERT_EQ("Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature",
			  Handbook::Views::GetData(stats[0], transport_catalogue));
	ASSERT_EQ("Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature",
			  Handbook::Views::GetData(stats[1], transport_catalogue));
	ASSERT_EQ("Bus 751: not found", Handbook::Views::GetData(stats[2], transport_catalogue));
	ASSERT_EQ("Stop Samara: not found", Handbook::Views::GetData(stats[3], transport_catalogue));
	ASSERT_EQ("Stop Prazhskaya: no buses", Handbook::Views::GetData(stats[4], transport_catalogue));
	ASSERT_EQ("Stop Biryulyovo Zapadnoye: buses 256 828", Handbook::Views::GetData(stats[5], transport_catalogue));
}

TEST(Catalogue, Test2)
{
	using namespace std;
	std::vector<std::string> reqs = {"Stop Tolstopaltsevo: 55.611087, 37.20829, 100m to Marushkino"s,
									 "Stop Marushkino: 55.595884, 37.209755"s,
									 "Bus 256: Marushkino > Tolstopaltsevo > Marushkino"s};
	auto requests = std::vector<std::shared_ptr<Handbook::Control::Request>>(reqs.size());
	Handbook::Data::TransportCatalogue transport_catalogue;
	for (const auto& r : reqs)
	{
		std::shared_ptr<Handbook::Control::Request> req = Handbook::Control::ParseRequestString(r);
		AddRequestToCatalogue(req.get(), transport_catalogue);
	}
	std::vector<std::string> stats = {"Bus 256"};
	//	std::vector<std::string> stats = {"Bus 750"s};

	ASSERT_EQ("Bus 256: 3 stops on route, 2 unique stops, 200 route length, 0.0590668 curvature",
			  Handbook::Views::GetData(stats[0], transport_catalogue));
}

TEST(Catalogue, Test3)
{
	using namespace std;
	std::vector<std::string> reqs = {"Stop Tolstopaltsevo: 55.611087, 37.20829, 100m to Marushkino"s,
									 "Stop Marushkino: 55.595884, 37.209755"s,
									 "Bus 256: Marushkino > Tolstopaltsevo > Marushkino"s};
	auto requests = std::vector<std::shared_ptr<Handbook::Control::Request>>(reqs.size());
	Handbook::Data::TransportCatalogue transport_catalogue;
	for (const auto& r : reqs)
	{
		std::shared_ptr<Handbook::Control::Request> req = Handbook::Control::ParseRequestString(r);

		AddRequestToCatalogue(req.get(), transport_catalogue);
	}
	std::vector<std::string> stats = {"Bus 256"};
	//	std::vector<std::string> stats = {"Bus 750"s};

	ASSERT_EQ("Bus 256: 3 stops on route, 2 unique stops, 200 route length, 0.0590668 curvature",
			  Handbook::Views::GetData(stats[0], transport_catalogue));
}

TEST(Catalogue, Test4)
{
	using namespace std;

	Handbook::Data::TransportCatalogue transport_catalogue;
	transport_catalogue.AddStop("Tolstopaltsevo"s, Handbook::Utilities::Coordinates{55.611087, 37.20829}, {{"Marushkino"s, 100}});
	transport_catalogue.AddStop("Marushkino"s, Handbook::Utilities::Coordinates{55.595884, 37.209755});
	transport_catalogue.AddBus("256"s, {"Marushkino", "Tolstopaltsevo", "Marushkino"});

	std::vector<std::string> stats = {"Bus 256"};
	//	std::vector<std::string> stats = {"Bus 750"s};

	ASSERT_EQ("Bus 256: 3 stops on route, 2 unique stops, 200 route length, 0.0590668 curvature",
			  Handbook::Views::GetData(stats[0], transport_catalogue));
}

TEST(Coordinates, Test1)
{
	using namespace std;

	Handbook::Utilities::Coordinates one{55.611087, 37.20829};

	Handbook::Utilities::Coordinates two{55.611087, 37.20829};

	ASSERT_EQ(one, two);
	two.lat = 0.0;
	ASSERT_NE(one, two);
}