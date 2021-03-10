#pragma once

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
namespace json
{

	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;
	using Number = std::variant<int, double>;

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error
	{
	  public:
		using runtime_error::runtime_error;
	};

	class Node
	{
	  public:
		Node() : node_item_(){};
		explicit Node(Array array);
		explicit Node(Dict map);
		explicit Node(bool value);
		explicit Node(int value);
		explicit Node(double value);
		explicit Node(Number value);
		explicit Node(std::string value);
		explicit Node(std::nullptr_t nullv);

		bool IsNull() const;
		bool IsArray() const;
		bool IsDict() const;
		bool IsBool() const;
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		double AsDouble() const;
		bool IsString() const;

		int AsInt() const;

		explicit operator std::string() const;

		friend std::ostream& operator<<(std::ostream& out, const Node& node);

		//		const Array& AsArray() const;
		//		const Dict& AsMap() const;
		//		int AsInt() const;
		//		const std::string& AsString() const;
		friend bool operator==(const Node& l, const Node& r);

	  private:
		//                  0          1      2     3    4     5          6
		std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> node_item_;
	};

	class Document
	{
	  public:
		explicit Document(Node root);

		const Node& GetRoot() const;

	  private:
		Node root_;
	};

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

} // namespace json