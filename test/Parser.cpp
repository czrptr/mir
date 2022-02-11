#include <doctest.h>
#include <fmt/core.h>
#include <NodeBuilder.h>
#include <parsing/Parser.h>

namespace
{

Parser parser(std::string const& text)
{
  return Parser(Tokenizer(text, "<file>"));
}

} // anonymous namespace

#define REQUIRE_AST_EQ(ast1, ast2) REQUIRE(equal((ast1), (ast2)))

TEST_SUITE_BEGIN("Parser");

/* ================== TokenExpression ================== */

TEST_CASE("symbols")
{
  auto prs = parser("main Ab__2c");
  auto s1 = prs.expression();
  auto s2 = prs.expression();

  REQUIRE_AST_EQ(s1, symbol("main"));
  REQUIRE_AST_EQ(s2, symbol("Ab__2c"));
}

TEST_CASE("builtins")
{
  auto prs = parser("@TypeOf @type_info");
  auto b1 = prs.expression();
  auto b2 = prs.expression();

  REQUIRE_AST_EQ(b1, builtin("TypeOf"));
  REQUIRE_AST_EQ(b2, builtin("type_info"));
}

TEST_CASE("string literals")
{
  auto prs = parser("\"er gw \\r etewr\" \"\"");
  auto s1 = prs.expression();
  auto s2 = prs.expression();

  REQUIRE_AST_EQ(s1, string("er gw \\r etewr"));
  REQUIRE_AST_EQ(s2, string(""));
}

TEST_CASE("number literals")
{
  auto prs = parser("0004324 4353.43463");
  auto n1 = prs.expression();
  auto n2 = prs.expression();

  REQUIRE_AST_EQ(n1, number("0004324"));
  REQUIRE_AST_EQ(n2, number("4353.43463"));
}

TEST_CASE("boolean literals")
{
  auto prs = parser("true false");
  auto b1 = prs.expression();
  auto b2 = prs.expression();

  REQUIRE_AST_EQ(b1, boolean(true));
  REQUIRE_AST_EQ(b2, boolean(false));
}

TEST_CASE("null")
{
  auto prs = parser("null");
  auto n = prs.expression();

  REQUIRE_AST_EQ(n, null());
}

TEST_CASE("undefined")
{
  auto prs = parser("undefined");
  auto u = prs.expression();

  REQUIRE_AST_EQ(u, undefined());
}

TEST_CASE("unreachable")
{
  auto prs = parser("unreachable");
  auto u = prs.expression();

  REQUIRE_AST_EQ(u, unreachable());
}

/* ================== LetStatement ================== */

TEST_CASE("let statements cannot be empty")
{
  auto prs = parser("let");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:0: error: let statement cannot be empty";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("let statement parts must be assigned")
{
  auto prs = parser("let a");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:5: error: assignement operator '=' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("let statement parts must be assigned a value")
{
  auto prs = parser("let a = ");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:7: error: expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("pub let statements")
{
  auto prs = parser("pub let a = 0");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(true, false, "a", number("0")));
}

TEST_CASE("mut let statements")
{
  auto prs = parser("let mut a = 0");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(false, true, "a", number("0")));
}

TEST_CASE("pub mut let statements")
{
  auto prs = parser("pub let mut a = 0");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(true, true, "a", number("0")));
}

TEST_CASE("let statements with types")
{
  auto prs = parser("let a: isize = 0");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(false, false, "a", symbol("isize"), number("0")));
}

TEST_CASE("let statements (multiple parts)")
{
  auto prs = parser("let a = 0, b = c, d = \"AAH\"");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(false, false, {
      part("a", number("0")),
      part("b", symbol("c")),
      part("d", string("AAH"))
    }));
}

TEST_CASE("let statements (complex)")
{
  auto prs = parser("pub let mut a: isize = 0, b = c, d: string = \"AAH\"");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(true, true, {
      part("a", symbol("isize"), number("0")),
      part("b", symbol("c")),
      part("d", symbol("string"), string("AAH"))
    }));
}

/* ================== BlockExpression ================== */

TEST_CASE("block labels")
{
  auto prs = parser("labelName : {}");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:10: error: did you mean 'labelName:'";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("only blocks, if and loops can be labeled")
{
  auto prs = parser("labelName: let a = 0;");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:0: error: only block, ifs and loops can be labeled";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("blocks")
{
  auto prs = parser("{} {{}}");
  auto b1 = prs.expression();
  auto b2 = prs.expression();

  REQUIRE_AST_EQ(b1, block({}));
  REQUIRE_AST_EQ(b2, block({block({})}));
}

TEST_CASE("labeled blocks")
{
  auto prs = parser("blk: {}");
  auto b = prs.expression();

  REQUIRE_AST_EQ(b, block("blk", {}));
}

TEST_CASE("blocks (complex)")
{
  auto prs = parser(
    "blk: {\n"
    "  pub let a = 0;\n"
    "  {\n"
    "    let mut b = \"AHH\";\n"
    "  }\n"
    "}");
  auto b = prs.expression();

  REQUIRE_AST_EQ(b,
    block("blk", {
      let(true, false, "a", number("0")),
      block({ let(false, true, "b", string("AHH"))})
    }));
}

/* ================== FunctionExpression ================== */

TEST_CASE("function type")
{
  auto prs = parser("fn () void");
  auto f = prs.expression();

  REQUIRE_AST_EQ(f, fn({}, symbol("void")));
}

TEST_CASE("function type (complex)")
{
  auto prs = parser("fn (arg1: Type1, arg2: char) fn () void");
  auto f = prs.expression();

  REQUIRE_AST_EQ(f,
    fn({{"arg1", symbol("Type1")}, {"arg2", symbol("char")}},
      fn({}, symbol("void"))));
}

TEST_CASE("function")
{
  auto prs = parser("fn () void {}");
  auto f = prs.expression();

  REQUIRE_AST_EQ(f, fn({}, symbol("void"), block({})));
}

TEST_CASE("function (complex)")
{
  auto prs = parser(
    "fn () void {\n"
    "  let a = b;\n"
    "}");
  auto f = prs.expression();

  REQUIRE_AST_EQ(f, fn({}, symbol("void"),
    block({let(false, false, "a", symbol("b"))})));
}

// TODO "fn() blk: {}" and "fn() Type {}" not being parsed as "fn() (Type{})""

/* ================== TypeExpression ================== */

// TODO

TEST_SUITE_END();