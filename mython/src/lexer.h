#pragma once

#include <algorithm>
#include <iosfwd>
#include <optional>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace parse
{

namespace token_type
{
struct Number
{
	int value;
};

struct Id
{
	std::string value;
};

struct Char
{
	char value;
};

struct String
{
	std::string value;
};

struct Class
{
};
struct Return
{
};
struct If
{
};
struct Else
{
};
struct Def
{
};
struct Newline
{
};
struct Print
{
};
struct Indent
{
};
struct Dedent
{
};
struct Eof
{
};
struct And
{
};
struct Or
{
};
struct Not
{
};
struct Eq
{
};
struct NotEq
{
};
struct LessOrEq
{
};
struct GreaterOrEq
{
};
struct None
{
};
struct True
{
};
struct False
{
};
} // namespace token_type

using TokenBase =
	std::variant<token_type::Number, token_type::Id, token_type::Char, token_type::String, token_type::Class,
				 token_type::Return, token_type::If, token_type::Else, token_type::Def, token_type::Newline,
				 token_type::Print, token_type::Indent, token_type::Dedent, token_type::And, token_type::Or,
				 token_type::Not, token_type::Eq, token_type::NotEq, token_type::LessOrEq, token_type::GreaterOrEq,
				 token_type::None, token_type::True, token_type::False, token_type::Eof>;

struct Token : TokenBase
{
	using TokenBase::TokenBase;

	template <typename T> [[nodiscard]] bool Is() const
	{
		return std::holds_alternative<T>(*this);
	}

	template <typename T> [[nodiscard]] const T& As() const
	{
		return std::get<T>(*this);
	}

	template <typename T> [[nodiscard]] const T* TryAs() const
	{
		return std::get_if<T>(this);
	}
};

bool operator==(const Token& lhs, const Token& rhs);
bool operator!=(const Token& lhs, const Token& rhs);

std::ostream& operator<<(std::ostream& os, const Token& rhs);

class LexerError : public std::runtime_error
{
  public:
	using std::runtime_error::runtime_error;
};

class Lexer
{
  public:
	explicit Lexer(std::istream& input);

	[[nodiscard]] const Token& CurrentToken() const;

	Token NextToken();

	template <typename T> const T& Expect() const
	{
		using namespace std::literals;
		if (this->CurrentToken().Is<T>())
		{
			return this->CurrentToken().As<T>();
		}
		throw LexerError("Error Expect()");
	}

	template <typename T, typename U> void Expect(const U& value) const
	{
		using namespace std::literals;
		this->Expect<T>();
		if ((this->doc_[this->counter_].As<T>().value != value))
		{
			throw LexerError("Error Expect(const U& value)");
		}
	}

	template <typename T> const T& ExpectNext()
	{
		using namespace std::literals;
		this->counter_++;
		if (this->counter_ < this->doc_.size())
		{
			return this->Expect<T>();
		}
		throw LexerError("Error in ExpectNext()");
	}

	template <typename T, typename U> void ExpectNext(const U& value)
	{
		using namespace std::literals;
		this->counter_++;
		if (this->counter_ < this->doc_.size())
		{
			return this->Expect<T>(value);
		}
		throw LexerError("Error in ExpectNext(const U& value)");
	}

  private:
	size_t counter_{};
	std::vector<Token> doc_{};
};

} // namespace parse