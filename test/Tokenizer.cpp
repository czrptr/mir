#include <doctest.h>
#include <ParsingUtils.h>
#include <parsing/Tokenizer.h>

// TODO test Intern

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
  std::string const text = "aLpH4_Num3r1C";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Symbol, 0, 0, 0, text.length(), text));
}

TEST_CASE("symbols (builtins)")
{
  std::string const text = "@aLpH4_Num3r1C";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Symbol, 0, 0, 0, text.length(), text));
}

TEST_CASE("symbols (builtins) wrong format")
{
  std::string const text = "@buil@in";
  Tokenizer tk(text, "<file>");
  try
  {
    tk.next();
    FAIL("unreachable");
  }
  catch (Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:5: error: '@' can only be used as a prefix to denote compiler builtin functions";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("operator (runes)")
{
  std::string const text = "*%= try not orelse catch and or return break continue defer";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 0, 0, 3, "*%="));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 4, 0, 7, "try"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 8, 0, 11, "not"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 12, 0, 18, "orelse"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 19, 0, 24, "catch"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 25, 0, 28, "and"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 29, 0, 31, "or"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 32, 0, 38, "return"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 39, 0, 44, "break"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 45, 0, 53, "continue"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 54, 0, 59, "defer"));
}

TEST_CASE("operator (keywords)")
{
  std::string const text = "not catch orelse and or";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 0, 0, 3, "not"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 4, 0, 9, "catch"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 10, 0, 16, "orelse"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 17, 0, 20, "and"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 21, 0, 23, "or"));
}

TEST_CASE("number literals (ints base 10)")
{
  std::string const text = "000124234286579";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::NumberLiteral, 0, 0, 0, text.length(), text));
}

TEST_CASE("number literals (floats base 10)")
{
  std::string const text = "000124234286579.3463452";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::NumberLiteral, 0, 0, 0, text.length(), text));
}

TEST_CASE("number literals can't start with decimal separator")
{
  std::string const text = ".14";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 0, 0, 1, "."));
  REQUIRE_EQ(tk.next(), t(Token::NumberLiteral, 0, 1, 0, 3, "14"));
}

TEST_CASE("number literals can't end with decimal separator")
{
  std::string const text = "3.";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::NumberLiteral, 0, 0, 0, 1, "3"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 1, 0, 2, "."));
}

TEST_CASE("string literals")
{
  std::string const text = "\"fwe gre \\\\ \\n \\t \\r \\\" 8468&^*646&^%&# \"";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::StringLiteral, 0, 0, 0, text.length(), text));
}

TEST_CASE("string literals can't contain unescaped newlines")
{
  std::string const text = "\"stri\nng\"";
  Tokenizer tk(text, "<file>");
  try
  {
    tk.next();
  }
  catch (Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:5: error: string literals cannot contain unescaped new lines, tabs or carriage returns";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("string literals can't contain unescaped tabs")
{
  std::string const text = "\"stri\tng\"";
  Tokenizer tk(text, "<file>");
  try
  {
    tk.next();
  }
  catch (Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:5: error: string literals cannot contain unescaped new lines, tabs or carriage returns";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("string literals can't contain unescaped carriage returns")
{
  std::string const text = "\"stri\rng\"";
  Tokenizer tk(text, "<file>");
  try
  {
    tk.next();
  }
  catch (Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:5: error: string literals cannot contain unescaped new lines, tabs or carriage returns";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("string literals can't contain unknown unescape sequences")
{
  std::string const text = "\" \\x \"";
  Tokenizer tk(text, "<file>");
  try
  {
    tk.next();
  }
  catch (Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:3: error: unknown escape sequence '\\x'";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("string literals must end")
{
  std::string const text = " \" ";
  Tokenizer tk(text, "<file>");
  try
  {
    tk.next();
  }
  catch (Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg =
        "<file>:0:3: error: string literal missing terminating '\"'\n"
        "<file>:0:1: note: string literal starts here";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("runes")
{
  Tokenizer tk(",:;()[]{}=>", "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Comma, 0, 0, 0, 1, ","));
  REQUIRE_EQ(tk.next(), t(Token::Colon, 0, 1, 0, 2, ":"));
  REQUIRE_EQ(tk.next(), t(Token::Semicolon, 0, 2, 0, 3, ";"));
  REQUIRE_EQ(tk.next(), t(Token::LParen, 0, 3, 0, 4, "("));
  REQUIRE_EQ(tk.next(), t(Token::RParen, 0, 4, 0, 5, ")"));
  REQUIRE_EQ(tk.next(), t(Token::LBracket, 0, 5, 0, 6, "["));
  REQUIRE_EQ(tk.next(), t(Token::RBracket, 0, 6, 0, 7, "]"));
  REQUIRE_EQ(tk.next(), t(Token::LBrace, 0, 7, 0, 8, "{"));
  REQUIRE_EQ(tk.next(), t(Token::RBrace, 0, 8, 0, 9, "}"));
  REQUIRE_EQ(tk.next(), t(Token::ThickArrow, 0, 9, 0, 11, "=>"));
}

TEST_CASE("keywords")
{
  std::string const text =
    "pub let mut comptime struct enum union fn "
    "if else switch loop import";

  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::KwPub, 0, 0, 0, 3, "pub"));
  REQUIRE_EQ(tk.next(), t(Token::KwLet, 0, 4, 0, 7, "let"));
  REQUIRE_EQ(tk.next(), t(Token::KwMut, 0, 8, 0, 11, "mut"));
  REQUIRE_EQ(tk.next(), t(Token::KwComptime, 0, 12, 0, 20, "comptime"));
  REQUIRE_EQ(tk.next(), t(Token::KwStruct, 0, 21, 0, 27, "struct"));
  REQUIRE_EQ(tk.next(), t(Token::KwEnum, 0, 28, 0, 32, "enum"));
  REQUIRE_EQ(tk.next(), t(Token::KwUnion, 0, 33, 0, 38, "union"));
  REQUIRE_EQ(tk.next(), t(Token::KwFn, 0, 39, 0, 41, "fn"));
  REQUIRE_EQ(tk.next(), t(Token::KwIf, 0, 42, 0, 44, "if"));
  REQUIRE_EQ(tk.next(), t(Token::KwElse, 0, 45, 0, 49, "else"));
  REQUIRE_EQ(tk.next(), t(Token::KwSwitch, 0, 50, 0, 56, "switch"));
  REQUIRE_EQ(tk.next(), t(Token::KwLoop, 0, 57, 0, 61, "loop"));
  REQUIRE_EQ(tk.next(), t(Token::KwImport, 0, 62, 0, 68, "import"));
}

TEST_CASE("last single char token ends on Eof")
{
  std::string const text = ";";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Semicolon, 0, 0, 0, 1, text));
}

TEST_CASE("correct Eof after single char token")
{
  std::string const text = ",";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Comma, 0, 0, 0, 1, text));
  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, 1, 0, 1, ""));
}

TEST_CASE("correct Eof after multi char token")
{
  std::string const text = "id3nti_fier";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Symbol, 0, 0, 0, text.length(), text));
  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, text.length(), 0, text.length(), ""));
}

TEST_CASE("Eof remains correct on multiple calls")
{
  std::string const text = ",";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Comma, 0, 0, 0, 1, text));
  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, 1, 0, 1, ""));
  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, 1, 0, 1, ""));
  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, 1, 0, 1, ""));
}

TEST_CASE("line comments")
{
  std::string const text = "// comment // /* this is not a block comment */ ";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Comment, 0, 0, 0, text.length(), text));
}

TEST_CASE("line comments (minimal)")
{
  std::string const text = "//";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Comment, 0, 0, 0, 2, "//"));
}

TEST_CASE("line comments end on new line but doesn't contain it")
{
  std::string const text = "// comment\n";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Comment, 0, 0, 0, 10, "// comment"));
}

TEST_CASE("block comments")
{
  std::string const text =
    "/* comment\n"
    "  text in block comment\n"
    "  \n"
    "  // line comment in bloc comment\n"
    "  * star in block comment\n"
    "  /* neste block comment\n"
    "    text in nested block comment\n"
    "  */\n"
    "*/";

  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Comment, 0, 0, 8, 2, text));
}

TEST_CASE("block comments (minimal)")
{
  std::string const text = "/**/";
  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Comment, 0, 0, 0, text.length(), text));
}

TEST_CASE("block comments must end")
{
  // TODO how to treat "/* */ */", special error on operator '*/' ?
  // TODO how to treat "/* /* */" ?

  std::string const text = "/* /* */";
  Tokenizer tk(text, "<file>");

  try
  {
    tk.next();
    FAIL("unreachable");
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg =
        "<file>:0:8: error: block comment missing terminating '*/'\n"
        "<file>:0:0: note: block comment starts here";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("line info is correct")
{
  std::string const text =
    "i     i\n"
    "// comment\n"
    "  iiii   i   \n"
    "\n"
    "/* commment\n"
    "  /*\n"
    "    /**/\n"
    "  */\n"
    "*/ i\n"
    "i";

  Tokenizer tk(text, "<file>");
  REQUIRE_EQ(tk.next(), t(Token::Symbol, 0, 0, 0, 1, "i"));
  REQUIRE_EQ(tk.next(), t(Token::Symbol, 0, 6, 0, 7, "i"));
  REQUIRE_EQ(tk.next(), t(Token::Comment, 1, 0, 1, 10, "// comment"));
  REQUIRE_EQ(tk.next(), t(Token::Symbol, 2, 2, 2, 6, "iiii"));
  REQUIRE_EQ(tk.next(), t(Token::Symbol, 2, 9, 2, 10, "i"));
  REQUIRE_EQ(tk.next(), t(Token::Comment, 4, 0, 8, 2,
    "/* commment\n"
    "  /*\n"
    "    /**/\n"
    "  */\n"
    "*/"));
  REQUIRE_EQ(tk.next(), t(Token::Symbol, 8, 3, 8, 4, "i"));
  REQUIRE_EQ(tk.next(), t(Token::Symbol, 9, 0, 9, 1, "i"));
}

TEST_CASE("operators (runes) must be valid")
{
  std::string const text = "<==^|^==>";
  Tokenizer tk(text, "<file>");
  try
  {
    tk.next();
  }
  catch (Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:0: error: unknown operator '<==^|^==>'";

    REQUIRE(msg.substr(0, expectedMsg.length()) == expectedMsg);
  }
}

TEST_SUITE_END();