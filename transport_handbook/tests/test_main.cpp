#include "control_reader.h"
#include "json.h"
#include "regex"
#include "svg.h"
#include "view_data.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string_view>
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
	transport_catalogue.AddStop("Tolstopaltsevo"s, Handbook::Utilities::Coordinates{55.611087, 37.20829},
								{{"Marushkino"s, 100}});
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

TEST(SVG, Circle)
{
	using namespace std;
	{
		std::stringstream ss;

		ss << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

		svg::Circle c;
		c.SetCenter({20, 20}).SetRadius(10);
		svg::RenderContext ctx(ss, 2, 2);
		c.Render(ctx);

		ss << "</svg>"sv;
		std::string result = R"(<?xml version="1.0" encoding="UTF-8" ?>
<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
  <circle cx="20" cy="20" r="10"/>
</svg>)";
		ASSERT_EQ(ss.str(), result);
	}
	{
		std::stringstream ss;

		ss << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

		svg::Circle c;
		c.SetCenter({20, 20}).SetRadius(10).SetFillColor("red");
		svg::RenderContext ctx(ss, 2, 2);
		c.Render(ctx);

		ss << "</svg>"sv;
		std::string result = R"(<?xml version="1.0" encoding="UTF-8" ?>
<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
  <circle cx="20" cy="20" r="10" fill="red" />
</svg>)";
		ASSERT_EQ(ss.str(), result);
	}
}

TEST(SVG, Polyline)
{
	{
		using namespace std;
		std::stringstream ss;

		ss << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

		svg::Polyline c;
		c.AddPoint({20, 10}).AddPoint({10, 10});
		svg::RenderContext ctx(ss, 2, 2);
		c.Render(ctx);

		ss << "</svg>"sv;
		std::string result = R"(<?xml version="1.0" encoding="UTF-8" ?>
<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
  <polyline points="20,10 10,10" />
</svg>)";
		ASSERT_EQ(ss.str(), result);
	}
	{
		using namespace std;
		std::stringstream ss;

		ss << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

		svg::Polyline c;
		c.AddPoint({20, 10}).AddPoint({10, 10}).SetStrokeLineJoin(svg::StrokeLineJoin::BEVEL);
		svg::RenderContext ctx(ss, 2, 2);
		c.Render(ctx);

		ss << "</svg>"sv;
		std::string result = R"(<?xml version="1.0" encoding="UTF-8" ?>
<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
  <polyline points="20,10 10,10" stroke-linejoin="bevel" />
</svg>)";
		ASSERT_EQ(ss.str(), result);
	}
}

TEST(SVG, Text)
{
	using namespace std;

	{
		std::stringstream ss;
		ss << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

		svg::Text c;
		c.SetPosition({1, 2}).SetFontSize(212);
		svg::RenderContext ctx(ss, 2, 2);
		c.Render(ctx);

		ss << "</svg>"sv;
		std::string expected = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
							   "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"
							   "  <text x=\"1\" y=\"2\" dx=\"0\" dy=\"0\" font-size=\"212\" ></text>\n"
							   "</svg>";
		ASSERT_EQ(expected, ss.str());
	}
	{
		std::stringstream ss;
		ss << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

		svg::Text c;
		c.SetPosition({1, 2}).SetFontSize(212);
		svg::RenderContext ctx(ss, 2, 2);
		c.Render(ctx);

		ss << "</svg>"sv;
		std::string expected = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
							   "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"
							   "  <text x=\"1\" y=\"2\" dx=\"0\" dy=\"0\" font-size=\"212\" ></text>\n"
							   "</svg>";
		ASSERT_EQ(expected, ss.str());
	}
}

svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays)
{
	svg::Polyline polyline;
	for (int i = 0; i <= num_rays; ++i)
	{
		double angle = 2 * M_PI * (i % num_rays) / num_rays;
		polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
		if (i == num_rays)
		{
			break;
		}
		angle += M_PI / num_rays;
		polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
	}
	return polyline;
}

// Выводит приветствие, круг и звезду
void DrawPicture()
{
	using namespace std;
	svg::Document doc;
	doc.Add(svg::Circle().SetCenter({20, 20}).SetRadius(10));
	doc.Add(svg::Text()
				.SetFontFamily("Verdana"s)
				.SetPosition({35, 20})
				.SetOffset({0, 6})
				.SetFontSize(12)
				.SetFontWeight("bold"s)
				.SetData("Hello C++"s));
	doc.Add(CreateStar({20, 50}, 10, 5, 5));
	doc.Render(std::cout);
}

TEST(SVG, Document2)
{
	svg::Document doc;
	doc.Add(svg::Circle().SetCenter({20, 20}).SetRadius(10));
	doc.Add(svg::Text()
				.SetFontFamily("Verdana")
				.SetPosition({35, 20})
				.SetOffset({0, 6})
				.SetFontSize(12)
				.SetFontWeight("bold")
				.SetData("Hello C++"));
	doc.Add(CreateStar({20, 50}, 10, 5, 5));
	doc.Render(std::cout);
}

TEST(SVG, Document)
{
	DrawPicture();
}
template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target)
{
	for (auto it = begin; it != end; ++it)
	{
		(*it)->Draw(target);
	}
}

template <typename Container> void DrawPicture(const Container& container, svg::ObjectContainer& target)
{
	using namespace std;
	DrawPicture(begin(container), end(container), target);
}

TEST(SVG, Shapes)
{
	using namespace svg;
	using namespace shapes;
	using namespace std;

	vector<unique_ptr<svg::Drawable>> picture;

	picture.emplace_back(make_unique<Triangle>(Point{100, 20}, Point{120, 50}, Point{80, 40}));
	// 5-лучевая звезда с центром {50, 20}, длиной лучей 10 и внутренним радиусом 4
	picture.emplace_back(make_unique<Star>(Point{50.0, 20.0}, 10.0, 4.0, 5));
	// Снеговик с "головой" радиусом 10, имеющей центр в точке {30, 20}
	picture.emplace_back(make_unique<Snowman>(Point{30, 20}, 10.0));

	svg::Document doc;
	// Так как документ реализует интерфейс ObjectContainer,
	// его можно передать в DrawPicture в качестве цели для рисования
	DrawPicture(picture, doc);

	// Выводим полученный документ в stdout
	doc.Render(cout);
}

TEST(SVG, Seters)
{
	using namespace svg;
	using namespace shapes;
	using namespace std;

	vector<unique_ptr<svg::Drawable>> picture;
	picture.emplace_back(make_unique<Triangle>(Point{100, 20}, Point{120, 50}, Point{80, 40}));
	picture.emplace_back(make_unique<Star>(Point{50.0, 20.0}, 10.0, 4.0, 5));
	picture.emplace_back(make_unique<Snowman>(Point{30, 20}, 10.0));

	svg::Document doc;
	DrawPicture(picture, doc);

	const Text base_text = //
		Text().SetFontFamily("Verdana"s).SetFontSize(12).SetPosition({10, 100}).SetData("Happy New Year!"s);
	doc.Add(Text{base_text}
				.SetStrokeColor("yellow"s)
				.SetFillColor("yellow"s)
				.SetStrokeLineJoin(StrokeLineJoin::ROUND)
				.SetStrokeLineCap(StrokeLineCap::ROUND)
				.SetStrokeWidth(3));
	doc.Add(Text{base_text}.SetFillColor("red"s));

	doc.Render(cout);
}

TEST(JSON, test1)
{
	using namespace json;
	using namespace std;

	istringstream strm{"[]"s};
	json::Node node = json::Load(strm).GetRoot();

	//	// Как узнать, какой тип хранит node в данный момент?
	//	assert(node.AsInt() == 0);
	//	assert(node.AsArray().empty());
	//	assert(node.AsMap().empty());
	//	assert(node.AsString().empty());
	std::variant<int, double, std::string> v = 1;
	std::cout << v.index() << std::endl;
	v = "asdasd"s;
	std::cout << v.index() << std::endl;
}

TEST(JSON, test2)
{
	using namespace std;
	std::istringstream iss(R"({"key":"value","key2":45,"key3":[null,15.5]})"s);
	json::Node node = json::Load(iss).GetRoot();

	auto a = 6;
}

json::Document LoadJSON(const std::string& s)
{
	std::istringstream strm(s);

	return json::Load(strm);
}

//// Раскомментируйте эти функции по мере того, как реализуете недостающий функционал
///*
std::string Print(const json::Node& node)
{
	std::ostringstream out;
	Print(json::Document{node}, out);
	return out.str();
}

TEST(JSON, test3)
{
	using namespace std;
	json::Node null_node;
	ASSERT_TRUE(null_node.IsNull());

	json::Node null_node1{nullptr};
	ASSERT_TRUE(null_node1.IsNull());

	ASSERT_EQ(Print(null_node), "null"s);

	const json::Node node = LoadJSON("null"s).GetRoot();

	ASSERT_TRUE(node.IsNull());

	ASSERT_EQ(node, null_node);
}

TEST(JSON, numbers)
{
	using namespace json;
	using namespace std;
	Node int_node{42};
	ASSERT_TRUE(int_node.IsInt());
	ASSERT_TRUE(int_node.AsInt() == 42);
	// целые числа являются подмножеством чисел с плавающей запятой
	ASSERT_TRUE(int_node.IsDouble());
	// Когда узел хранит int, можно получить соответствующее ему double-значение
	ASSERT_EQ(int_node.AsDouble(), 42.0);
	ASSERT_TRUE(!int_node.IsPureDouble());

	Node dbl_node{123.45};
	ASSERT_TRUE(dbl_node.IsDouble());
	ASSERT_EQ(dbl_node.AsDouble(), 123.45);
	ASSERT_TRUE(dbl_node.IsPureDouble()); // Значение содержит число с плавающей запятой
	ASSERT_TRUE(!dbl_node.IsInt());

	ASSERT_EQ(Print(int_node), "42"s);
	ASSERT_EQ(Print(dbl_node), "123.45"s);

	ASSERT_EQ(LoadJSON("42"s).GetRoot(), int_node);
	ASSERT_EQ(LoadJSON("123.45"s).GetRoot(), dbl_node);
	ASSERT_EQ(LoadJSON("0.25"s).GetRoot().AsDouble(), 0.25);
	ASSERT_EQ(LoadJSON("3e5"s).GetRoot().AsDouble(), 3e5);
	ASSERT_EQ(LoadJSON("1.2e-5"s).GetRoot().AsDouble(), 1.2e-5);
	ASSERT_EQ(LoadJSON("1.2e+5"s).GetRoot().AsDouble(), 1.2e5);
	ASSERT_EQ(LoadJSON("-123456"s).GetRoot().AsInt(), -123456);
}

TEST(JSON, strings)
{
	using namespace json;
	using namespace std;

	Node str_node{"Hello, \"\"everybody\""s};
	//
	ASSERT_TRUE(str_node.IsString());
	ASSERT_EQ(str_node.AsString(), "Hello, \"\"everybody\""s);

	ASSERT_TRUE(!str_node.IsInt());
	ASSERT_TRUE(!str_node.IsDouble());

	//	std::cout << "\"Hello, \\\"everybody\\\"\""s << std::endl;
	//	std::cout << Print(str_node) << std::endl;

	ASSERT_EQ(Print(str_node), "\"Hello, \\\"\\\"everybody\\\"\""s);

	ASSERT_EQ(LoadJSON(Print(str_node)).GetRoot(), str_node);
}

TEST(JSON, bools)
{
	using namespace json;
	using namespace std;
	Node true_node{true};
	ASSERT_TRUE(true_node.IsBool());
	ASSERT_TRUE(true_node.AsBool());

	Node false_node{false};
	ASSERT_TRUE(false_node.IsBool());
	ASSERT_TRUE(!false_node.AsBool());

	ASSERT_EQ(Print(true_node), "true"s);
	ASSERT_EQ(Print(false_node), "false"s);

	ASSERT_EQ(LoadJSON("true"s).GetRoot(), true_node);
	ASSERT_EQ(LoadJSON("false"s).GetRoot(), false_node);
}

TEST(JSON, array)
{
	using namespace json;
	using namespace std;
	Node arr_node{Array{1, 1.23, "Hello"s}};
	ASSERT_TRUE(arr_node.IsArray());
	const Array& arr = arr_node.AsArray();
	ASSERT_EQ(arr.size(), 3);
	ASSERT_EQ(arr.at(0).AsInt(), 1);

	auto b = LoadJSON("[1, 1.23, \"Hello\"]"s);
	auto c = 3;
	std::cout << Print(arr_node);
	ASSERT_EQ(LoadJSON("[1, 1.23, \"Hello\"]"s).GetRoot(), arr_node);
	ASSERT_EQ(LoadJSON(Print(arr_node)).GetRoot(), arr_node);
}

TEST(JSON, map)
{
	using namespace json;
	using namespace std;
	Node dict_node{Dict{{"key1"s, "value1"s}, {"key2"s, 42}}};
	ASSERT_TRUE(dict_node.IsMap());
	const Dict& dict = dict_node.AsMap();
	ASSERT_EQ(dict.size(), 2);
	ASSERT_EQ(dict.at("key1"s).AsString(), "value1"s);
	ASSERT_EQ(dict.at("key2"s).AsInt(), 42);

	auto b = LoadJSON("{ \"key1\": \"value1\", \"key2\": 42 }"s);
	ASSERT_EQ(LoadJSON("{ \"key1\": \"value1\", \"key2\": 42 }"s).GetRoot(), dict_node);
	std::cout << Print(dict_node);
	ASSERT_EQ(LoadJSON(Print(dict_node)).GetRoot(), dict_node);
}

void MustFailToLoad(const std::string& s)
{
	using namespace std;
	try
	{
		LoadJSON(s);
		std::cerr << "ParsingError exception is expected on '"sv << s << "'"sv << std::endl;
		assert(false);
	}
	catch (const json::ParsingError&)
	{
		// ok
	}
	catch (const std::exception& e)
	{
		std::cerr << "exception thrown: "sv << e.what() << std::endl;
		assert(false);
	}
	catch (...)
	{
		std::cerr << "Unexpected error"sv << std::endl;
		assert(false);
	}
}

template <typename Fn> void MustThrowLogicError(Fn fn)
{
	using namespace std;
	try
	{
		fn();
		std::cerr << "logic_error is expected"sv << std::endl;
		assert(false);
	}
	catch (const std::logic_error&)
	{
		// ok
	}
	catch (const std::exception& e)
	{
		std::cerr << "exception thrown: "sv << e.what() << std::endl;
		assert(false);
	}
	catch (...)
	{
		std::cerr << "Unexpected error"sv << std::endl;
		assert(false);
	}
}

TEST(JSON, inserts)
{
	using namespace json;
	using namespace std;
	MustFailToLoad("["s);
	MustFailToLoad("]"s);

	MustFailToLoad("{"s);
	MustFailToLoad("}"s);

	MustFailToLoad("\"hello"s); // незакрытая кавычка

	MustFailToLoad("tru"s);
	MustFailToLoad("fals"s);
	MustFailToLoad("nul"s);

	Node dbl_node{3.5};
	MustThrowLogicError([&dbl_node] { dbl_node.AsInt(); });
	MustThrowLogicError([&dbl_node] { dbl_node.AsString(); });
	MustThrowLogicError([&dbl_node] { dbl_node.AsArray(); });

	Node array_node{Array{}};
	MustThrowLogicError([&array_node] { array_node.AsMap(); });
	MustThrowLogicError([&array_node] { array_node.AsDouble(); });
	MustThrowLogicError([&array_node] { array_node.AsBool(); });
}

TEST(JSON, benchmark)
{
	using namespace json;
	using namespace std;
	const auto start = std::chrono::steady_clock::now();
	Array arr;
	arr.reserve(1'000);
	for (int i = 0; i < 1'000; ++i)
	{
		arr.emplace_back(Dict{
			{"int"s, 42},
			{"double"s, 42.1},
			{"null"s, nullptr},
			{"string"s, "hello"s},
			{"array"s, Array{1, 2, 3}},
			{"bool"s, true},
			{"map"s, Dict{{"key"s, "value"s}}},
		});
	}
	std::stringstream strm;
	json::Print(Document{arr}, strm);
	const auto doc = json::Load(strm);
	assert(doc.GetRoot() == arr);
	const auto duration = std::chrono::steady_clock::now() - start;
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << "ms"sv << std::endl;
}

TEST(JSON, ADDITIONAL){
	using namespace json;
	using namespace std;
	ASSERT_EQ(Node(42) , Node(42.));
}