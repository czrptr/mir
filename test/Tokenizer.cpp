#include <doctest.h>
#include <fmt/core.h>
#include <parsing/Intern.h>
#include <parsing/Tokenizer.h>
#include <parsing/Token.h>

// TODO test Intern

namespace
{

Token t(Token::Tag tag, size_t startLine, size_t startColumn, size_t endLine, size_t endColumn)
{
  return Token(tag, {startLine, startColumn}, {endLine, endColumn}, Intern::string(std::move(fmt::format("{}", tag))));
}

Token t(Token::Tag tag, size_t startLine, size_t startColumn, size_t endLine, size_t endColumn, std::string const& text)
{
  return Token(tag, {startLine, startColumn}, {endLine, endColumn}, Intern::string(text));
}


} // namespace

TEST_SUITE_BEGIN("Tokenizer");

TEST_CASE("empty text")
{
  Tokenizer tk("");
  CHECK_EQ(tk.next(), t(Token::Eof, 0, 1, 0, 1));
}

TEST_CASE("token ending on Eof")
{
  std::string text = "42.314";
  Tokenizer tk(text);
  CHECK_EQ(tk.next(), t(Token::NumberLiteral, 0, 0, 0, 6, text));
}

TEST_SUITE_END();