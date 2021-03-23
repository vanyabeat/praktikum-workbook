#include "json_builder.h"
#include <gtest/gtest.h>

TEST(Json, UseTest)
{

	using namespace json;
	using namespace std;
	//	ASSERT_EQ(Node(42), Node(42.));
	json::Document document(Dict{
		{"int"s, 42},
		{"double"s, 42.1},
		{"null"s, nullptr},
		{"string"s, "hello"s},
		{"array"s, Array{1, 2, 3}},
		{"bool"s, true},
		{"map"s, Dict{{"key"s, "value"s}}},
	});
	json::Print(document, std::cout);
}

TEST(TestEmpty, JustAString)
{
	using namespace std;

	auto expected_doc = json::Document{json::Builder{}.Value("just a string").Build()};

	json::Document actual_doc("just a string");
	//	std::cout << "EXPECTED" << std::endl;
	//	json::Print(expected_doc, cout);
	//	std::cout << std::endl;
	//	std::cout << "ACTUAL" << std::endl;
	//	json::Print(actual_doc, cout);
	//	std::cout << std::endl;
	ASSERT_EQ(expected_doc, actual_doc);
}

TEST(TestEmpty, Exception)
{
	using namespace std;

	bool expt = false;
	try
	{
		auto expected_doc = json::Document{json::Builder{}.Value("just a string").Value("just a string").Build()};
	}
	catch (std::exception& e)
	{
		ASSERT_EQ(e.what(), "[Value] ended node"s);
		expt = true;
	}
	ASSERT_TRUE(expt);
}

TEST(TestEmpty, JustAString2)
{
	using namespace std;

	auto expected_doc = json::Document{json::Builder{}.Value("just a string").Build()};

	json::Document actual_doc("just a string");
	std::cout << "EXPECTED" << std::endl;
	json::Print(expected_doc, cout);
	std::cout << std::endl;
	std::cout << "ACTUAL" << std::endl;
	json::Print(actual_doc, cout);
	std::cout << std::endl;
	ASSERT_EQ(expected_doc, actual_doc);

	json::Print(
		json::Document{// Форматирование не имеет формального значения:
					   // это просто цепочка вызовов методов
					   json::Builder{}
						   .StartDict()
						   .Key("key1"s)
						   .Value(123)
						   .Key("key2"s)
						   .Value("value2"s)
						   .Key("key3"s)
						   .StartArray()
						   .Value(456)
						   .StartDict()
						   .EndDict()
						   .StartDict()
						   .Key(""s)
						   .Value(nullptr)
						   .EndDict()
						   .Value(""s)
						   .EndArray()
						   .EndDict()
						   .Build()},
		cout);
}

TEST(TestArray, ArrayEmpty)
{
	using namespace std;

	auto expected_doc = json::Document{json::Builder{}.StartArray().EndArray().Build()};

	json::Document actual_doc(json::Array{});
	ASSERT_EQ(expected_doc, actual_doc);
}

TEST(TestArray, ArrayEmptyNested)
{
	using namespace std;

	auto expected_doc =
		json::Document{json::Builder{}.StartArray().Value(1).StartArray().Value(1).EndArray().EndArray().Build()};

	json::Document actual_doc(json::Array{1, json::Array{1}});
	std::cout << "EXPECTED" << std::endl;
	json::Print(expected_doc, cout);
	std::cout << std::endl;
	std::cout << "ACTUAL" << std::endl;
	json::Print(actual_doc, cout);
	std::cout << std::endl;
	ASSERT_EQ(expected_doc, actual_doc);
}

TEST(TestArray, ArrayEmptyNestedExeption)
{
	using namespace std;
	bool expt = false;
	try
	{
		auto expected_doc =
			json::Document{json::Builder{}.StartArray().Value(1).StartArray().Value(1).EndArray().Build()};
	}

	catch (std::exception& e)
	{
		ASSERT_EQ(e.what(), "[Build] building of ended node"s);
		expt = true;
	}
	ASSERT_TRUE(expt);
}

TEST(TestDict, DictEmpty)
{
	using namespace std;

	auto expected_doc = json::Document{json::Builder{}.StartDict().EndDict().Build()};

	json::Document actual_doc(json::Dict{});
	std::cout << "EXPECTED" << std::endl;
	json::Print(expected_doc, cout);
	std::cout << std::endl;
	std::cout << "ACTUAL" << std::endl;
	json::Print(actual_doc, cout);
	std::cout << std::endl;
	ASSERT_EQ(expected_doc, actual_doc);
}

TEST(TestDict, DictNested)
{
	using namespace std;

	auto expected_doc =
		json::Document{json::Builder{}.StartDict().Key("key").StartArray().EndArray().EndDict().Build()};

	json::Document actual_doc(json::Dict{{"key", json::Array{}}});
	std::cout << "EXPECTED" << std::endl;
	json::Print(expected_doc, cout);
	std::cout << std::endl;
	std::cout << "ACTUAL" << std::endl;
	json::Print(actual_doc, cout);
	std::cout << std::endl;
	ASSERT_EQ(expected_doc, actual_doc);
}

//TEST(KeyValue, Exception)
//{
//	using namespace std;
//
//	using namespace std;
//	bool expt = false;
//	try
//	{
//		json::Builder{}.Value("s"s).Key("1"s).Build();
//	}
//
//	catch (std::exception& e)
//	{
//		ASSERT_EQ(e.what(), "[Key] ended node key"s);
//		expt = true;
//	}
//	ASSERT_TRUE(expt);
//}
//
//TEST(StartDict, Exception)
//{
//	using namespace std;
//
//	using namespace std;
//	bool expt = false;
//	try
//	{
//		json::Builder{}.Value("s"s).StartDict().Build();
//	}
//
//	catch (std::exception& e)
//	{
//		ASSERT_EQ(e.what(), "[Dict] ready node start dict"s);
//		expt = true;
//	}
//	ASSERT_TRUE(expt);
//}
//
//TEST(StartArray, Exception)
//{
//	using namespace std;
//
//	using namespace std;
//	bool expt = false;
//	try
//	{
//		json::Builder{}.Value("s"s).StartArray().Build();
//	}
//
//	catch (std::exception& e)
//	{
//		ASSERT_EQ(e.what(), "[Array] start array error"s);
//		expt = true;
//	}
//	ASSERT_TRUE(expt);
//}
//
//TEST(StartDict, Exception2)
//{
//	using namespace std;
//
//	using namespace std;
//	bool expt = false;
//	try
//	{
//		json::Builder{}.StartDict().Key("1"s).Key("2"s).EndDict();
//	}
//
//	catch (std::exception& e)
//	{
//		ASSERT_EQ(e.what(), "[Key] repeat sequence"s);
//		expt = true;
//	}
//	ASSERT_TRUE(expt);
//}
//
//TEST(StartArray, Exception2)
//{
//	using namespace std;
//
//	using namespace std;
//	bool expt = false;
//	try
//	{
//		json::Builder{}.StartArray().StartDict().Key("1"s).EndDict().Key("1"s).EndArray();
//	}
//
//	catch (std::exception& e)
//	{
//		ASSERT_EQ(e.what(), "[Dict] dict value expected"s);
//		expt = true;
//	}
//	ASSERT_TRUE(expt);
//}
//
//TEST(StartDict, Exception3)
//{
//	using namespace std;
//	bool expt = false;
//	try
//	{
//		json::Builder{}.StartDict().Value("2"s).EndDict();
//	}
//
//	catch (std::exception& e)
//	{
//		ASSERT_EQ(e.what(), "[Value] dict value without key"s);
//		expt = true;
//	}
//	ASSERT_TRUE(expt);
//}
//
//TEST(StartDict, Exception4)
//{
//	using namespace std;
//	bool expt = false;
//	try
//	{
//		json::Builder{}.StartDict().StartDict().Key("1"s).Value("2"s).EndDict().EndDict();
//	}
//
//	catch (std::exception& e)
//	{
//		ASSERT_EQ(e.what(), "[Dict] start dict in another dict"s);
//		expt = true;
//	}
//	ASSERT_TRUE(expt);
//}
//
//TEST(StartArray, Exception4)
//{
//	using namespace std;
//	bool expt = false;
//	try
//	{
//		json::Builder{}.StartDict().StartArray().Value("2"s).EndArray().EndDict();
//	}
//
//	catch (std::exception& e)
//	{
//		ASSERT_EQ(e.what(), "[Array] start array error"s);
//		expt = true;
//	}
//	ASSERT_TRUE(expt);
//}
//
//TEST(StartDict, Exception5)
//{
//	using namespace std;
//	bool expt = false;
//	try
//	{
//		json::Builder{}.StartDict().EndArray();
//	}
//
//	catch (std::exception& e)
//	{
//		ASSERT_EQ(e.what(), "[Array] non-array node end array"s);
//		expt = true;
//	}
//	ASSERT_TRUE(expt);
//}
//
//TEST(StartDict, Exception6)
//{
//	using namespace std;
//	bool expt = false;
//	try
//	{
//		json::Builder{}.StartArray().EndDict();
//	}
//
//	catch (std::exception& e)
//	{
//		ASSERT_EQ(e.what(), "[Dict] it is not a dict"s);
//		expt = true;
//	}
//	ASSERT_TRUE(expt);
//}


