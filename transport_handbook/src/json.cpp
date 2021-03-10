#include "json.h"

using namespace std;

namespace json
{

	namespace
	{

		Node LoadNode(istream& input);

		Node LoadArray(istream& input)
		{
			Array result;

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

		Node LoadInt(istream& input)
		{
			int result = 0;
			while (isdigit(input.peek()))
			{
				result *= 10;
				result += input.get() - '0';
			}
			return Node(result);
		}

		Node LoadString(istream& input)
		{
			string line;
			getline(input, line, '"');
			return Node(move(line));
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

				string key = std::string(LoadString(input));
				input >> c;
				result.insert({move(key), LoadNode(input)});
			}

			return Node(move(result));
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

		Node LoadNullBool(std::istream& input)
		{
			string line;
			getline(input, line, ',');
			if (line.find("true"s) != std::string::npos)
			{
				return Node(true);
			}
			else if (line.find("false"s) != std::string::npos)
			{
				return Node(false);
			}
			else if (line.find("null") != std::string::npos)
			{
				return Node();
			}
			else
			{
				throw ParsingError("Failed to read json word from stream"s);
			}
		}

		Node LoadNode(istream& input)
		{
			char c;
			input >> c;
			if (c == '[')
			{
				return LoadArray(input);
			}
			else if (c == '{')
			{
				return LoadDict(input);
			}
			else if (c == '"')
			{
				return LoadString(input);
			}
			else if (isdigit(c) || c == '-' || c == '+')
			{
				input.putback(c);
				return LoadNumber(input);
			}
			else if (isalpha(c))
			{
				input.putback(c);
				return LoadNullBool(input);
			}
			throw ParsingError("warning");
		}

	} // namespace

	Node::Node(Array array) : node_item_(move(array))
	{
	}

	Node::Node(Dict map) : node_item_(move(map))
	{
	}

	Node::Node(int value) : node_item_(value)
	{
	}

	Node::Node(string value) : node_item_(move(value))
	{
	}

	Node::Node(bool value) : node_item_(value)
	{
	}
	Node::Node(Number value)
	{
		if (value.index() == 0)
		{
			node_item_ = std::get<int>(value);
		}
		else
		{
			node_item_ = std::get<double>(value);
		}
	}
	Node::Node(double value) : node_item_(value)
	{
	}
	Node::Node(std::nullptr_t nullv) : node_item_()
	{
	}
	bool Node::IsNull() const
	{
		return node_item_.index() == 0;
	}
	bool Node::IsArray() const
	{
		return node_item_.index() == 1;
	}
	bool Node::IsDict() const
	{
		return node_item_.index() == 2;
	}
	bool Node::IsBool() const
	{
		return node_item_.index() == 3;
	}
	bool Node::IsInt() const
	{
		return node_item_.index() == 4;
	}
	bool Node::IsDouble() const
	{
		if (IsInt())
		{
			return true;
		}
		return node_item_.index() == 5;
	}
	bool Node::IsPureDouble() const
	{
		return node_item_.index() == 5;
	}
	double Node::AsDouble() const
	{
		if (IsInt())
		{
			return static_cast<double>(std::get<int>(node_item_));
		}
		if (IsPureDouble())
		{
			return std::get<double>(node_item_);
		}
		return {};
	}
	bool Node::IsString() const
	{
		return node_item_.index() == 6;
	}
	int Node::AsInt() const
	{
		if (IsInt())
		{
			return std::get<int>(node_item_);
		}
		return {};
	}
	std::ostream& operator<<(ostream& out, const Node& node)
	{
		if (node.IsString())
		{
			out << "\"" << std::string(node) << "\"";
		}
		return out;
	}
	bool operator==(const Node& l, const Node& r)
	{
		return l.node_item_ == r.node_item_;
	}
	Node::operator std::string() const
	{
		using namespace std;
		if (IsString())
		{
			return std::get<std::string>(node_item_);
		}
		else if (IsNull())
		{
			return "null"s;
		}
		else if (IsBool())
		{
			return std::get<bool>(node_item_) ? "true"s : "false"s;
		}
		else if (IsPureDouble())
		{
			std::stringstream ss;
			ss << std::get<double>(node_item_);
			return ss.str();
		}
		else if (IsInt())
		{
			return std::to_string(std::get<int>(node_item_));
		}
		else if (IsArray())
		{
			std::string result = "[";
			size_t array_length = std::get<Array>(node_item_).size() - 1;
			size_t counter = 0;
			for (auto t = 0; t < array_length - 1; ++t)
			{
				result += std::string(std::get<Array>(node_item_)[t]) + ", ";
			}
			result += std::string(std::get<Array>(node_item_)[array_length]) + "]";
		}
		return {};
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

	void Print(const Document& doc, std::ostream& out)
	{
		if (doc.GetRoot().IsNull())
		{
			out << "null";
		}
		else if (doc.GetRoot().IsArray())
		{
			out << std::string(doc.GetRoot());
		}
		else if (doc.GetRoot().IsInt())
		{
			out << std::string(doc.GetRoot());
		}
		else if (doc.GetRoot().IsPureDouble())
		{
			out << std::string(doc.GetRoot());
		}
		//		return out;
		// Реализуйте функцию самостоятельно
	}

} // namespace json