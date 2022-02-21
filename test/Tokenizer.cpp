#include <doctest.h>
#include <ParsingUtils.h>
#include <parsing/Tokenizer.h>

// TODO test Intern

TEST_SUITE_BEGIN("Tokenizer");

TEST_CASE("string source text is empty")
{
  TOKENIZER_TEXT("");

  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, 0, 0, 0, ""));
}

TEST_CASE("file source text is empty")
{
  TOKENIZER_FILE("./test/files/empty.mir");

  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, 0, 0, 0, ""));
  fileStream.close();
}

TEST_CASE("last multi char token ends on Eof")
{
  std::string const text = "aLpH4_Num3r1C";
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::Symbol, 0, 0, 0, text.length(), text));
}

TEST_CASE("symbols (builtins)")
{
  std::string const text = "@aLpH4_Num3r1C";
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::Symbol, 0, 0, 0, text.length(), text));
}

TEST_CASE("symbols (builtins) wrong format")
{
  TOKENIZER_TEXT("@buil@in");
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

TEST_CASE("operators")
{
  TOKENIZER_TEXT("*%= try not orelse catch and or return break continue defer");

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

TEST_CASE("number literals (ints base 10)")
{
  std::string const text = "000124234286579";
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::NumberLiteral, 0, 0, 0, text.length(), text));
}

TEST_CASE("number literals (floats base 10)")
{
  std::string const text = "000124234286579.3463452";
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::NumberLiteral, 0, 0, 0, text.length(), text));
}

TEST_CASE("number literals can't start with decimal separator")
{
  TOKENIZER_TEXT(".14");

  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 0, 0, 1, "."));
  REQUIRE_EQ(tk.next(), t(Token::NumberLiteral, 0, 1, 0, 3, "14"));
}

TEST_CASE("number literals can't end with decimal separator")
{
  TOKENIZER_TEXT("3.");

  REQUIRE_EQ(tk.next(), t(Token::NumberLiteral, 0, 0, 0, 1, "3"));
  REQUIRE_EQ(tk.next(), t(Token::Operator, 0, 1, 0, 2, "."));
}

TEST_CASE("string literals")
{
  std::string const text = "\"fwe gre \\\\ \\n \\t \\r \\\" 8468&^*646&^%&# \"";
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::StringLiteral, 0, 0, 0, text.length(), text));
}

TEST_CASE("string literals can't contain unescaped newlines")
{
  TOKENIZER_TEXT("\"stri\nng\"");
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
  TOKENIZER_TEXT("\"stri\tng\"");
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
  TOKENIZER_TEXT("\"stri\rng\"");
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
  TOKENIZER_TEXT("\" \\x \"");
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
  TOKENIZER_TEXT(" \" ");
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
  TOKENIZER_TEXT(",:;()[]{}=>");

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
  TOKENIZER_TEXT(
    "pub let mut comptime struct enum union fn "
    "if else switch loop import"
  );

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
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::Semicolon, 0, 0, 0, text.length(), text));
}

TEST_CASE("correct Eof after single char token")
{
  std::string const text = ",";
  size_t const length = text.length();
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::Comma, 0, 0, 0, length, text));
  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, length, 0, length, ""));
}

TEST_CASE("correct Eof after multi char token")
{
  std::string const text = "id3nti_fier";
  size_t const length = text.length();
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::Symbol, 0, 0, 0, length, text));
  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, length, 0, length, ""));
}

TEST_CASE("Eof remains correct on multiple calls")
{
  std::string const text = ",";
  size_t const length = text.length();
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::Comma, 0, 0, 0, length, text));
  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, length, 0, length, ""));
  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, length, 0, length, ""));
  REQUIRE_EQ(tk.next(), t(Token::Eof, 0, length, 0, length, ""));
}

TEST_CASE("line comments")
{
  std::string const text = "// comment // /* this is not a block comment */ ";
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::Comment, 0, 0, 0, text.length(), text));
}

TEST_CASE("line comments (minimal)")
{
  std::string const text = "//";
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::Comment, 0, 0, 0, text.length(), text));
}

TEST_CASE("line comments end on new line but doesn't contain it")
{
  TOKENIZER_TEXT("// comment\n");

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
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::Comment, 0, 0, 8, 2, text));
}

TEST_CASE("block comments (minimal)")
{
  std::string const text = "/**/";
  TOKENIZER_TEXT(text);

  REQUIRE_EQ(tk.next(), t(Token::Comment, 0, 0, 0, text.length(), text));
}

TEST_CASE("block comments must end")
{
  // TODO how to treat "/* */ */", special error on operator '*/' ?
  // TODO how to treat "/* /* */" ?

  TOKENIZER_TEXT("/* /* */");
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
  TOKENIZER_TEXT(
    "i     i\n"
    "// comment\n"
    "  iiii   i   \n"
    "\n"
    "/* commment\n"
    "  /*\n"
    "    /**/\n"
    "  */\n"
    "*/ i\n"
    "i"
  );

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
  TOKENIZER_TEXT("<==^|^==>");
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