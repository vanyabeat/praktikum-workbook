#include "json.h"

#include <sstream>
#include <variant>

static int max_null = 4;
static int max_bool = 5;

using namespace std;

namespace json
{

	bool operator==(const Node& lhs, const Node& rhs)
	{
		return lhs.GetValue() == rhs.GetValue();
	}

	bool operator!=(const Node& lhs, const Node& rhs)
	{
		return !(lhs.GetValue() == rhs.GetValue());
	}

	bool operator==(const Document& lhs, const Document& rhs)
	{
		return lhs.GetRoot() == rhs.GetRoot();
	}

	bool operator!=(const Document& lhs, const Document& rhs)
	{
		return !(lhs.GetRoot() == rhs.GetRoot());
	}

	Node LoadNumber(std::istream& input)
	{
		using namespace std::literals;

		std::string parsed_num;

		// Считывает в parsed_num очередной символ из input
		auto read_char = [&parsed_num, &input] {
			parsed_num += static_cast<char>(input.get());
			if (!input)
			{
				throw ParsingError("Failed to read number from stream"s);
			}
		};

		// Считывает одну или более цифр в parsed_num из input
		auto read_digits = [&input, read_char] {
			if (!std::isdigit(input.peek()))
			{
				throw ParsingError("A digit is expected"s);
			}
			while (std::isdigit(input.peek()))
			{
				read_char();
			}
		};

		if (input.peek() == '-')
		{
			read_char();
		}
		// Парсим целую часть числа
		if (input.peek() == '0')
		{
			read_char();
			// После 0 в JSON не могут идти другие цифры
		}
		else
		{
			read_digits();
		}

		bool is_int = true;
		// Парсим дробную часть числа
		if (input.peek() == '.')
		{
			read_char();
			read_digits();
			is_int = false;
		}

		// Парсим экспоненциальную часть числа
		if (int ch = input.peek(); ch == 'e' || ch == 'E')
		{
			read_char();
			if (ch = input.peek(); ch == '+' || ch == '-')
			{
				read_char();
			}
			read_digits();
			is_int = false;
		}

		try
		{
			if (is_int)
			{
				// Сначала пробуем преобразовать строку в int
				try
				{
					return Node(std::stoi(parsed_num));
				}
				catch (...)
				{
					// В случае неудачи, например, при переполнении
					// код ниже попробует преобразовать строку в double
				}
			}
			return Node(std::stod(parsed_num));
		}
		catch (...)
		{
			throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
		}
	}

	namespace
	{

		Node LoadNode(istream& input);

		// class ParsingError : public std::runtime_error {
		// public:
		//  using runtime_error::runtime_error;
		//};

		Node LoadInt(istream& input)
		{
			return LoadNumber(input);
			//  return Node(0);
		}

		Node LoadString(istream& input)
		{
			char ch;
			string line;
			bool no_error = false;
			while (input.get(ch))
			{
				if (ch == '"')
				{
					no_error = true;
					break;
				}
				if (ch == '\\')
				{
					input.get(ch);
					if (ch == 'r')
					{
						ch = '\r';
					}
					if (ch == 'n')
					{
						ch = '\n';
					}
					if (ch == '"')
					{
						ch = '\"';
					}
					if (ch == 't')
					{
						ch = '\t';
					}
					if (ch == '\\')
					{
						ch = '\\';
					}
				}

				line += ch;
			}

			if (!no_error)
			{
				throw json::ParsingError("Error parsing string");
			}

			return Node(move(line));
		}

		Node LoadBool(istream& input)
		{
			char ch;
			string line;
			int cnt = 0;
			while (cnt != max_bool)
			{
				++cnt;
				input.get(ch);
				line += ch;
				if (line == "true")
				{
					break;
				}
			}

			if (line != "true" && line != "false")
			{
				throw json::ParsingError("");
			}
			if (line == "true")
			{
				return Node(true);
			}

			return Node(false);
		}

		Node LoadArray(istream& input)
		{
			Array result;
			//    bool error = false;
			for (char c; input >> c && c != ']';)
			{
				if (c != ',')
				{
					input.putback(c);
				}
				result.push_back(LoadNode(input));
			}

			return Node(move(result));
		}

		Node LoadDict(istream& input)
		{
			Dict result;
			for (char c; input >> c && c != '}';)
			{
				if (c == ',')
				{
					input >> c;
				}
				string key = LoadString(input).AsString();
				input >> c;
				result.insert({move(key), LoadNode(input)});
			}
			return Node(move(result));
		}

		Node LoadNull(istream& input)
		{
			char ch;
			string line;
			int cnt = 0;
			while (cnt != max_null)
			{
				++cnt;
				input.get(ch);
				line += ch;
				ch = ' ';
			}
			if (line != "null")
			{
				throw json::ParsingError("");
			}
			return Node();
		}

		Node LoadNode(istream& input)
		{
			char c;
			input >> c;

			if (c == '[')
			{
				if (input >> c)
				{
					input.putback(c);
					return LoadArray(input);
				}
				throw json::ParsingError("Wrong array");
			}
			else if (c == '{')
			{
				if (input >> c)
				{
					input.putback(c);
					return LoadDict(input);
				}
				throw json::ParsingError("Wrong map");
			}
			else if (c == '"')
			{
				return LoadString(input);
			}
			else if (c == 'n')
			{
				input.putback(c);
				return LoadNull(input);
			}
			else if (c == 't' || c == 'f')
			{
				input.putback(c);
				return LoadBool(input);
			}
			else if (c == ']' || c == '}')
			{
				throw json::ParsingError("");
			}
			else
			{
				input.putback(c);
				return LoadInt(input);
			}
		}

	} // namespace

	Node::Node() : is_null_(true)
	{
	}

	Node::Node(std::nullptr_t value) : value_(value), is_null_(true)
	{
	}

	Node::Node(string value) : value_(move(value)), is_string_(true)
	{
	}

	Node::Node(int value) : value_(value), is_int_(true), is_double_(true)
	{
	}

	Node::Node(double value) : value_(value), is_double_(true), is_puredouble_(true)
	{
	}

	Node::Node(bool value) : value_(value), is_bool_(true)
	{
	}

	Node::Node(Array array) : value_(move(array)), is_array_(true)
	{
	}

	Node::Node(Dict map) : value_(move(map)), is_map_(true)
	{
	}

	bool Node::IsNull() const
	{
		return is_null_;
	}

	bool Node::IsString() const
	{
		return is_string_;
	}

	bool Node::IsInt() const
	{
		return is_int_;
	}

	bool Node::IsDouble() const
	{
		return is_double_;
	}

	bool Node::IsPureDouble() const
	{
		return is_puredouble_;
	}

	bool Node::IsBool() const
	{
		return is_bool_;
	}

	bool Node::IsArray() const
	{
		return is_array_;
	}

	bool Node::IsMap() const
	{
		return is_map_;
	}

	string Node::AsString() const
	{
		if (!IsString())
		{
			throw std::logic_error("");
		}
		return std::get<std::string>(value_);
	}

	int Node::AsInt() const
	{
		if (!IsInt())
		{
			throw std::logic_error("");
		}
		return std::get<int>(value_);
	}

	double Node::AsDouble() const
	{
		if (!IsDouble() && !IsInt())
		{
			throw std::logic_error("");
		}
		if (IsInt())
		{
			return std::get<int>(value_);
		}
		return std::get<double>(value_);
	}

	bool Node::AsBool() const
	{
		if (!IsBool())
		{
			throw std::logic_error("");
		}
		return std::get<bool>(value_);
	}

	Array Node::AsArray() const
	{
		if (!IsArray())
		{
			throw std::logic_error("");
			;
		}
		return std::get<Array>(value_);
	}

	Dict Node::AsMap() const
	{
		if (!IsMap())
		{
			throw std::logic_error("");
		}
		return std::get<Dict>(value_);
	}

	Document::Document(Node root) : root_(move(root))
	{
	}

	const Node& Document::GetRoot() const
	{
		return root_;
	}

	Document Load(istream& input)
	{
		return Document{LoadNode(input)};
	}

	std::string NodePrinter::operator()(nullptr_t)
	{
		return "null"s;
	}

	std::string NodePrinter::operator()(std::string value)
	{
		std::string res;
		res += "\"";
		for (auto c : value)
		{
			if (c == '\\')
			{
				res += R"(\\)";
			}
			else if (c == '\"')
			{
				res += R"(\")";
			}
			else if (c == '\n')
			{
				res += R"(\n)";
			}
			else if (c == '\r')
			{
				res += R"(\r)";
			}
			else if (c == '\t')
			{
				res += R"(\t)";
			}
			else
			{
				res += c;
			}
		}
		res += "\"";
		return res;
	}

	std::string NodePrinter::operator()(double value)
	{
		std::ostringstream strs;
		strs << value;
		return strs.str();
	}

	std::string NodePrinter::operator()(int value)
	{
		std::ostringstream strs;
		strs << value;
		return strs.str();
	}

	std::string NodePrinter::operator()(bool value)
	{
		if (value)
		{
			return "true"s;
		}

		return "false"s;
	}

	std::string NodePrinter::operator()(Array value)
	{
		string out = "[";
		bool first = true;
		for (Node& node : value)
		{
			if (first)
			{
				first = false;
				out.append(std::visit(NodePrinter{}, node.GetValue()));
			}
			else
			{
				out.append(", "s);
				out.append(std::visit(NodePrinter{}, node.GetValue()));
			}
		}
		out += "]";
		return out;
	}

	std::string NodePrinter::operator()(Dict value)
	{
		string out = "{";
		bool first = true;
		for (auto& pair : value)
		{
			if (first)
			{
				first = false;
			}
			else
			{
				out.append(", ");
			}
			out.append("\"");
			out.append(pair.first);
			out.append("\"");
			out.append(": ");
			out.append(std::visit(NodePrinter{}, pair.second.GetValue()));
		}
		out += " }";
		return out;
	}

	Value Node::GetValue() const
	{
		return value_;
	}

	void Print(const Document& doc, std::ostream& output)
	{
		std::string out = std::visit(NodePrinter{}, doc.GetRoot().GetValue());
		output << out;
	}

} // namespace json