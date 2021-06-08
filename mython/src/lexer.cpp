#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <iostream>

using namespace std;

namespace parse
{

bool operator==(const Token& lhs, const Token& rhs)
{
	using namespace token_type;

	if (lhs.index() != rhs.index())
	{
		return false;
	}
	if (lhs.Is<Char>())
	{
		return lhs.As<Char>().value == rhs.As<Char>().value;
	}
	if (lhs.Is<Number>())
	{
		return lhs.As<Number>().value == rhs.As<Number>().value;
	}
	if (lhs.Is<String>())
	{
		return lhs.As<String>().value == rhs.As<String>().value;
	}
	if (lhs.Is<Id>())
	{
		return lhs.As<Id>().value == rhs.As<Id>().value;
	}
	return true;
}

bool operator!=(const Token& lhs, const Token& rhs)
{
	return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const Token& rhs)
{
	using namespace token_type;

#define VALUED_OUTPUT(type)                                                                                            \
	if (auto p = rhs.TryAs<type>())                                                                                    \
		return os << #type << '{' << p->value << '}';

	VALUED_OUTPUT(Number);
	VALUED_OUTPUT(Id);
	VALUED_OUTPUT(String);
	VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type)                                                                                          \
	if (rhs.Is<type>())                                                                                                \
		return os << #type;

	UNVALUED_OUTPUT(Class);
	UNVALUED_OUTPUT(Return);
	UNVALUED_OUTPUT(If);
	UNVALUED_OUTPUT(Else);
	UNVALUED_OUTPUT(Def);
	UNVALUED_OUTPUT(Newline);
	UNVALUED_OUTPUT(Print);
	UNVALUED_OUTPUT(Indent);
	UNVALUED_OUTPUT(Dedent);
	UNVALUED_OUTPUT(And);
	UNVALUED_OUTPUT(Or);
	UNVALUED_OUTPUT(Not);
	UNVALUED_OUTPUT(Eq);
	UNVALUED_OUTPUT(NotEq);
	UNVALUED_OUTPUT(LessOrEq);
	UNVALUED_OUTPUT(GreaterOrEq);
	UNVALUED_OUTPUT(None);
	UNVALUED_OUTPUT(True);
	UNVALUED_OUTPUT(False);
	UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

	return os << "Unknown token :("sv;
}

namespace
{
inline bool _line_with_comment_(const std::string& s)
{
	for (const auto& c : s)
	{
		if (c != ' ' && c != '#')
		{
			return false;
		}
		if (c == '#')
		{
			break;
		}
	}
	return true;
}

inline bool _is_alphabet_(const char& c)
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'))
	{
		return true;
	}
	return false;
}

inline bool _is_digit_(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	s[0] == '-' ? it += 1 : it += 0;
	while (it != s.end() && std::isdigit(*it))
		++it;
	return !s.empty() && it == s.end();
}

inline bool _is_digit_(const char& c)
{
	return std::isdigit(c);
}

inline std::string _read_id_(std::istream& input)
{
	char ch;
	std::string id;
	while (_is_alphabet_(input.peek()) || _is_digit_(input.peek()))
	{
		input >> ch;
		id += ch;
	}
	return id;
}

inline bool _is_operation_(const std::string& c)
{
	if (c == "==" || c == ">=" || c == "<=" || c == "!=")
	{
		return true;
	}
	return false;
}

inline bool _is_char_(const std::string& c)
{
	if (c == "." || c == "," || c == "(" || c == "+" || c == ")" || c == "-" || c == "*" || c == "/" || c == ":" ||
		c == "@" || c == "%" || c == "$" || c == "^" || c == "&" || c == ";" || c == "?" || c == "=" || c == "<" ||
		c == ">" || c == "!" || c == "{" || c == "}" || c == "[" || c == "]")
	{
		return true;
	}
	return false;
}

inline bool _is_char_(const char c)
{
	if (c == '.' || c == ',' || c == '(' || c == '+' || c == ')' || c == '-' || c == '*' || c == '/' || c == ':' ||
		c == '@' || c == '%' || c == '$' || c == '^' || c == '&' || c == ';' || c == '?' || c == '=' || c == '<' ||
		c == '>' || c == '!' || c == '{' || c == '}' || c == '[' || c == ']')
	{
		return true;
	}
	return false;
}

inline Token _get_number_(const std::string& input)
{
	token_type::Number token = {std::stoi(input)};
	return token;
}

inline std::string _read_number_(istream& input)
{
	char ch;
	std::string num;
	while (_is_digit_(input.peek()))
	{
		input >> ch;
		num += ch;
	}
	return num;
}

inline std::string _read_string_(std::istream& input)
{
	using namespace std;
	std::string result;
	char quote;
	input >> std::noskipws >> quote;
	char ch;
	while (true)
	{
		if (input >> std::noskipws >> ch)
		{
			if (ch == quote)
			{
				break;
			}
			else if (ch == '\\')
			{
				char escaped_char;
				if (input >> escaped_char)
				{
					switch (escaped_char)
					{
					case 'n':
						result.push_back('\n');
						break;
					case 't':
						result.push_back('\t');
						break;
					case '"':
						result.push_back('"');
						break;
					case '\'':
						result.push_back('\'');
						break;
					case '\\':
						result.push_back('\\');
						break;
					default:
						throw LexerError("Error in _read_string_ unrecognized escape sequence \\"s +
										 std::string(1, escaped_char));
					}
				}
				else
				{
					throw LexerError("Error in _read_string_ unexpected end of line");
				}
			}
			else if (ch == '\n' || ch == '\r')
			{
				throw LexerError("Error in _read_string_ unexpected end of line");
			}
			else
			{
				result.push_back(ch);
			}
		}
		else
		{
			throw LexerError("Error in _read_string_ unexpected end of line");
		}
	}
	return result;
}

inline Token _get_char_(const char& input)
{
	return token_type::Char{input};
}

inline Token _get_str_(const std::string& input)
{
	std::string tmp_in = input;
	if (tmp_in.find_first_of('\'') != tmp_in.find_last_of('\''))
	{
		tmp_in.erase(std::remove(tmp_in.begin(), tmp_in.end(), '\\'), tmp_in.end());
	}
	if (tmp_in.find_first_of('\"') != tmp_in.find_last_of('\"'))
	{
		tmp_in.erase(std::remove(tmp_in.begin(), tmp_in.end(), '\\'), tmp_in.end());
	}
	return token_type::String{tmp_in};
}

inline Token _get_id_(const std::string& input)
{
	if (input == "class")
	{
		return token_type::Class{};
	}
	else if (input == "if")
	{
		return token_type::If{};
	}
	else if (input == "else")
	{
		return token_type::Else{};
	}
	else if (input == "def")
	{
		return token_type::Def{};
	}
	else if (input == "\n")
	{
		return token_type::Newline{};
	}
	else if (input == "print")
	{
		return token_type::Print{};
	}
	else if (input == "ident")
	{
		return token_type::Indent{};
	}
	else if (input == "dedent")
	{
		return token_type::Dedent{};
	}
	else if (input == "&&" || input == "and")
	{
		return token_type::And{};
	}
	else if (input == "||" || input == "or")
	{

		return token_type::Or{};
	}
	else if (input == "not")
	{
		return token_type::Not{};
	}
	else if (input == "==")
	{
		return token_type::Eq{};
	}
	else if (input == "!=")
	{
		return token_type::NotEq{};
	}
	else if (input == "<=")
	{

		return token_type::LessOrEq{};
	}
	else if (input == ">=")
	{
		return token_type::GreaterOrEq{};
	}
	else if (input == "None")
	{
		return token_type::None{};
	}
	else if (input == "True")
	{
		return token_type::True{};
	}
	else if (input == "False")
	{
		return token_type::False{};
	}
	else if (input == "eof")
	{
		return token_type::Eof{};
	}
	else if (input == "return")
	{
		return token_type::Return{};
	}
	else
	{
		return token_type::Id{input};
	}
}

inline std::string _read_op_(std::istream& input)
{
	char ch;
	input >> ch;
	std::string id;
	return id;
}

inline void _indent_dedent_(int cur, int last, std::vector<Token>& doc)
{
	if (cur > last)
	{
		if (cur % 2 != 0)
		{
			return;
		}
		while (cur > last)
		{
			doc.emplace_back(token_type::Indent{});
			cur -= 2;
		}
	}
	else if (last > cur)
	{
		if (last % 2 != 0)
		{
			return;
		}
		while (last > cur)
		{
			doc.emplace_back(token_type::Dedent{});
			last -= 2;
		}
	}
	return;
}
} // namespace

Lexer::Lexer(std::istream& input)
{
	int last_sps = 0;
	int now_sps = 0;
	std::string line;
	while (!input.eof())
	{
		std::getline(input, line);
		if (line.empty())
		{
			continue;
		}
		if (_line_with_comment_(line))
		{
			continue;
		}

		now_sps = 0;
		if (line[0] == ' ')
		{
			int ptr = 0;
			while (line[ptr++] == 32)
			{
				now_sps++;
			}
		}
		_indent_dedent_(now_sps, last_sps, doc_);
		last_sps = now_sps;

		stringstream ss;
		ss << line;
		char ch;
		while (!ss.eof())
		{
			if (ss.peek() == ' ')
			{
				ss >> std::noskipws >> ch;
			}
			if (ss.peek() == '#')
			{
				break;
			}
			if (_is_digit_(ss.peek()))
			{
				this->doc_.emplace_back(_get_number_(_read_number_(ss)));
			}
			if (ss.peek() == '\'' || ss.peek() == '\"')
			{
				this->doc_.emplace_back(_get_str_(_read_string_(ss)));
			}
			if (_is_alphabet_(ss.peek()))
			{
				this->doc_.emplace_back(_get_id_(_read_id_(ss)));
			}
			if (_is_char_(ss.peek()))
			{
				if (ss.peek() == '=')
				{
					ss >> ch;
					if (ss.peek() == '=')
					{
						this->doc_.emplace_back(_get_id_("=="));
						ss >> ch;
					}
					else
					{
						this->doc_.emplace_back(_get_char_(ch));
					}
				}
				else if (ss.peek() == '!')
				{
					ss >> ch;
					if (ss.peek() == '=')
					{
						this->doc_.emplace_back(_get_id_("!="));
						ss >> ch;
					}
					else
					{
						this->doc_.emplace_back(_get_char_(ch));
					}
				}
				else if (ss.peek() == '>')
				{
					ss >> ch;
					if (ss.peek() == '=')
					{
						this->doc_.emplace_back(_get_id_(">="));
						ss >> ch;
					}
					else
					{
						this->doc_.emplace_back(_get_char_(ch));
					}
				}
				else if (ss.peek() == '<')
				{
					ss >> ch;
					if (ss.peek() == '=')
					{
						this->doc_.emplace_back(_get_id_("<="));
						ss >> ch;
					}
					else
					{
						this->doc_.emplace_back(_get_char_(ch));
					}
				}
				else
				{
					this->doc_.push_back(_get_char_(ss.peek()));
					ss >> std::noskipws >> ch;
				}
			}
		}
		this->doc_.emplace_back(token_type::Newline{});
	}
	_indent_dedent_(0, last_sps, doc_);
	this->doc_.emplace_back(token_type::Eof{});
}

const Token& Lexer::CurrentToken() const
{
	if (this->counter_ < this->doc_.size())
	{
		return this->doc_[this->counter_];
	}
	throw LexerError("Error in CurrentToken()");
}

Token Lexer::NextToken()
{
	this->counter_++;
	if (this->counter_ < this->doc_.size())
	{
		return this->doc_[this->counter_];
	}
	else
	{
		return token_type::Eof{};
	}
}

} // namespace parse