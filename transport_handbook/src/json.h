#pragma once

#include <iomanip>
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
		Node(Array array);
		Node(Dict map);
		Node(bool value);
		Node(int value);
		Node(double value);
		Node(Number value);
		Node(std::string value);
		Node(std::nullptr_t nullv);

		bool IsNull() const;
		bool IsArray() const;

		bool IsMap() const;
		bool IsBool() const;
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsString() const;

		const std::string& AsString() const;
		double AsDouble() const;
		int AsInt() const;
		bool AsBool() const;
		const Dict& AsMap() const;
		const Array& AsArray() const;

		explicit operator std::string() const;

		friend std::ostream& operator<<(std::ostream& out, const Node& node);

		//		const Array& AsArray() const;
		//		const Dict& AsMap() const;
		//		int AsInt() const;
		//		const std::string& AsString() const;
		friend bool operator==(const Node& l, const Node& r);
		friend bool operator!=(const Node& l, const Node& r);

	  private:
		//                  0          1      2     3    4     5          6
		std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> node_item_;
	};

	class Document
	{
	  public:
		explicit Document(Node root);

		const Node& GetRoot() const;
		friend bool operator==(const Document& l, const Document& r)
		{
			return l.GetRoot() == r.GetRoot();
		}
		friend bool operator!=(const Document& l, const Document& r)
		{
			return !(l == r);
		}

	  private:
		Node root_;
	};

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

} // namespace json