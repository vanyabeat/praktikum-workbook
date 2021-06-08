#include "lexer.h"
#include <gtest/gtest.h>
#include <sstream>
#include <string>
using namespace std;

TEST(LexerTest, SimpleAssigment)
{
	using namespace parse;
	std::istringstream input("x = 42\n"s);
	parse::Lexer lexer(input);

	ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Id{"x"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'='}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Number{42}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
}

TEST(LexerTest, TestKeywords)
{
	using namespace parse;
	std::istringstream input("class return if else def print or None and not True False"s);
	Lexer lexer(input);

	ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Class{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Return{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::If{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Else{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Def{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Print{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Or{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::None{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::And{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Not{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::True{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::False{}));
}

TEST(LexerTest, TestNumbers)
{
	using namespace parse;
	std::istringstream input("42 15 -53"s);
	Lexer lexer(input);

	ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Number{42}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Number{15}));
	// Отрицательные числа формируются на этапе синтаксического анализа
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'-'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Number{53}));
}

TEST(LexerTest, TestIds)
{
	using namespace parse;
	std::istringstream input("x    _42 big_number   Return Class  dEf"s);
	Lexer lexer(input);

	ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Id{"x"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"_42"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"big_number"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"Return"s})); // keywords are case-sensitive
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"Class"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"dEf"s}));
}

TEST(LexerTest, TestStrings)
{
	using namespace parse;
	std::istringstream input(
		R"('word' "two words" 'long string with a double quote " inside' "another long string with single quote ' inside")"s);
	Lexer lexer(input);

	ASSERT_EQ(lexer.CurrentToken(), Token(token_type::String{"word"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::String{"two words"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::String{"long string with a double quote \" inside"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::String{"another long string with single quote ' inside"s}));
}

TEST(LexerTest, TestOperations)
{
	using namespace parse;
	std::istringstream input("+-*/= > < != == <> <= >="s);
	Lexer lexer(input);

	ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Char{'+'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'-'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'*'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'/'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'='}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'>'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'<'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::NotEq{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Eq{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'<'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'>'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::LessOrEq{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::GreaterOrEq{}));
}

TEST(LexerTest, TestIndentsAndNewlines)
{
	using namespace parse;
	std::istringstream input(R"(
no_indent
  indent_one
    indent_two
      indent_three
      indent_three
      indent_three
    indent_two
  indent_one
    indent_two
no_indent
)"s);

	Lexer lexer(input);

	ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Id{"no_indent"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Indent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"indent_one"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Indent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"indent_two"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Indent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"indent_three"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"indent_three"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"indent_three"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Dedent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"indent_two"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Dedent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"indent_one"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Indent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"indent_two"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Dedent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Dedent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"no_indent"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
}

TEST(LexerTest, TestEmptyLinesAreIgnored)
{
	using namespace parse;
	std::istringstream input(R"(
x = 1
  y = 2

  z = 3


)"s);
	Lexer lexer(input);

	ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Id{"x"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'='}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Number{1}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Indent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"y"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'='}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Number{2}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	// Пустая строка, состоящая только из пробельных символов не меняет текущий отступ,
	// поэтому следующая лексема — это Id, а не Dedent
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"z"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'='}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Number{3}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Dedent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
}

TEST(LexerTest, TestMythonProgram)
{
	using namespace parse;
	std::istringstream input(R"(
x = 4
y = "hello"

class Point:
  def __init__(self, x, y):
    self.x = x
    self.y = y

  def __str__(self):
    return str(x) + ' ' + str(y)

p = Point(1, 2)
print str(p)
)"s);
	Lexer lexer(input);

	ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Id{"x"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'='}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Number{4}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"y"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'='}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::String{"hello"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Class{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"Point"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{':'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Indent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Def{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"__init__"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'('}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"self"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{','}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"x"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{','}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"y"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{')'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{':'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Indent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"self"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'.'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"x"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'='}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"x"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"self"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'.'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"y"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'='}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"y"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Dedent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Def{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"__str__"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'('}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"self"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{')'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{':'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Indent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Return{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"str"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'('}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"x"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{')'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'+'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::String{" "s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'+'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"str"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'('}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"y"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{')'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Dedent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Dedent{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"p"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'='}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"Point"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'('}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Number{1}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{','}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Number{2}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{')'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Print{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"str"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{'('}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"p"s}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Char{')'}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
	ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
}

TEST(LexerTest, TestExpect)
{
	using namespace parse;
	std::istringstream is("bugaga"s);
	Lexer lex(is);

	ASSERT_NO_THROW(lex.Expect<token_type::Id>());
	ASSERT_EQ(lex.Expect<token_type::Id>().value, "bugaga"s);
	ASSERT_NO_THROW(lex.Expect<token_type::Id>("bugaga"s));
	ASSERT_THROW(lex.Expect<token_type::Id>("widget"s), LexerError);
	ASSERT_THROW(lex.Expect<token_type::Return>(), LexerError);
}

TEST(LexerTest, TestExpectNext)
{
	using namespace parse;
	std::istringstream is("+ bugaga + def 52"s);
	Lexer lex(is);

	ASSERT_EQ(lex.CurrentToken(), Token(token_type::Char{'+'}));
	ASSERT_NO_THROW(lex.ExpectNext<token_type::Id>());
	ASSERT_NO_THROW(lex.ExpectNext<token_type::Char>('+'));
	ASSERT_THROW(lex.ExpectNext<token_type::Newline>(), LexerError);
	ASSERT_THROW(lex.ExpectNext<token_type::Number>(57), LexerError);
}

TEST(LexerTest, TestAlwaysEmitsNewlineAtTheEndOfNonemptyLine)
{
	using namespace parse;
	{
		std::istringstream is("a b"s);
		Lexer lexer(is);

		ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Id{"a"s}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"b"s}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
	}
	{
		std::istringstream is("+"s);
		Lexer lexer(is);

		ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Char{'+'}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
	}
}
TEST(LexerTest, TestCommentsAreIgnored)
{
	using namespace parse;
	{
		std::istringstream is(R"(# comment
)"s);
		Lexer lexer(is);

		ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Eof{}));
	}
	{
		std::istringstream is(R"(# comment

)"s);
		Lexer lexer(is);
		ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Eof{}));
	}
	{
		std::istringstream is(R"(# comment
x #another comment
abc#
'#'
"#123"
#)"s);

		Lexer lexer(is);
		ASSERT_EQ(lexer.CurrentToken(), Token(token_type::Id{"x"s}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Id{"abc"s}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::String{"#"s}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::String{"#123"s}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Newline{}));
		ASSERT_EQ(lexer.NextToken(), Token(token_type::Eof{}));
	}
}
