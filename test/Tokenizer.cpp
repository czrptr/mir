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

TEST_CASE("string source text is empty")
{
  std::string const filePath = "<file>";
  try
  {
    auto tk = Tokenizer("", filePath);
    FAIL("unreachable");
  }
  catch (Error const& err)
  {
    std::string const msg = fmt::format("{}", err);
    REQUIRE_EQ(msg, fmt::format("{}: source text is empty", filePath));
  }
}

TEST_CASE("file doesn't exist")
{
  std::string const filePath = "./test/files/I_DONT_EXIST.mir";
  try
  {
    auto tk = Tokenizer(filePath);
    FAIL("unreachable");
  }
  catch (Error const& err)
  {
    std::string const msg = fmt::format("{}", err);
    REQUIRE_EQ(msg, fmt::format("{}: file doesn't exist", filePath));
  }
}

TEST_CASE("file source text is empty")
{
  std::string const filePath = "./test/files/empty.mir";
  try
  {
    auto tk = Tokenizer(filePath);
    FAIL("unreachable");
  }
  catch (Error const& err)
  {
    std::string const msg = fmt::format("{}", err);
    REQUIRE_EQ(msg, fmt::format("{}: source text is empty", filePath));
  }
}

TEST_CASE("last multi char token ends on Eof")
{
  std::string text = "42.314";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::NumberLiteral, 0, 0, 0, 6, text));
}

TEST_CASE("last single char token ends on Eof")
{
  std::string text = ";";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Semicolon, 0, 0, 0, 1, text));
}

TEST_SUITE_END();