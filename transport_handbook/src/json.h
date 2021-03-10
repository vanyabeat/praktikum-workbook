#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json
{

	class ParsingError : public std::runtime_error
	{
	  public:
		using runtime_error::runtime_error;
	};

	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;
	using Value = std::variant<std::nullptr_t, std::string, double, int, bool, Array, Dict>;

	class Node
	{
	  public:
		Node();
		Node(std::nullptr_t value);
		Node(int value);
		Node(double value);
		Node(std::string value);
		Node(bool value);
		Node(Array array);
		Node(Dict map);

		bool IsNull() const;
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsString() const;
		bool IsBool() const;
		bool IsArray() const;
		bool IsMap() const;

		std::nullptr_t AsNull() const;
		std::string AsString() const;
		double AsDouble() const;
		int AsInt() const;
		Array AsArray() const;
		Dict AsMap() const;
		bool AsBool() const;

		//  inline bool operator==(const Node& right) {
		//    return this->value_ == right.value_;
		//  }

		Value GetValue() const;

	  private:
		Value value_;
		bool is_null_ = false;
		bool is_int_ = false;
		bool is_double_ = false;
		bool is_puredouble_ = false;
		bool is_bool_ = false;
		bool is_array_ = false;
		bool is_map_ = false;
		bool is_string_ = false;
	};

	bool operator==(const Node& lhs, const Node& rhs);

	bool operator!=(const Node& lhs, const Node& rhs);

	class Document
	{
	  public:
		explicit Document(Node root);

		const Node& GetRoot() const;

	  private:
		Node root_;
	};

	bool operator==(const Document& lhs, const Document& rhs);
	bool operator!=(const Document& lhs, const Document& rhs);

	Document Load(std::istream& input);

	struct NodePrinter
	{
		std::string operator()(std::nullptr_t);
		std::string operator()(std::string other);
		std::string operator()(double value);
		std::string operator()(int value);
		std::string operator()(bool value);
		std::string operator()(Array value);
		std::string operator()(Dict dict);
	};

	void Print(const Document& doc, std::ostream& output);

	Node LoadNumber(std::istream& input);

} // namespace json