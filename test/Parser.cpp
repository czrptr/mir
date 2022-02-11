#include <doctest.h>
#include <fmt/core.h>
#include <NodeBuilder.h>
#include <parsing/Parser.h>

TEST_SUITE_BEGIN("Parser");

Parser parser(std::string const& text)
{
  return Parser(Tokenizer(text, "<file>"));
}

/* ================== TokenExpression ================== */

TEST_CASE("symbols")
{
  auto prs = parser("main Ab__2c");
  auto s1 = prs.tokenExpression();
  auto s2 = prs.tokenExpression();

  REQUIRE(equal(s1, symbol("main")));
  REQUIRE(equal(s2, symbol("Ab__2c")));
}

TEST_CASE("builtins")
{
  auto prs = parser("@TypeOf @type_info");
  auto b1 = prs.tokenExpression();
  auto b2 = prs.tokenExpression();

  REQUIRE(equal(b1, builtin("TypeOf")));
  REQUIRE(equal(b2, builtin("type_info")));
}

TEST_CASE("string literals")
{
  auto prs = parser("\"er gw \\r etewr\" \"\"");
  auto s1 = prs.tokenExpression();
  auto s2 = prs.tokenExpression();

  REQUIRE(equal(s1, string("er gw \\r etewr")));
  REQUIRE(equal(s2, string("")));
}

TEST_CASE("number literals")
{
  auto prs = parser("0004324 4353.43463");
  auto n1 = prs.tokenExpression();
  auto n2 = prs.tokenExpression();

  REQUIRE(equal(n1, number("0004324")));
  REQUIRE(equal(n2, number("4353.43463")));
}

TEST_CASE("boolean literals")
{
  auto prs = parser("true false");
  auto b1 = prs.tokenExpression();
  auto b2 = prs.tokenExpression();

  REQUIRE(equal(b1, boolean(true)));
  REQUIRE(equal(b2, boolean(false)));
}

TEST_CASE("null")
{
  auto prs = parser("null");
  auto n = prs.tokenExpression();

  REQUIRE(equal(n, null()));
}

TEST_CASE("undefined")
{
  auto prs = parser("undefined");
  auto u = prs.tokenExpression();

  REQUIRE(equal(u, undefined()));
}

TEST_CASE("unreachable")
{
  auto prs = parser("unreachable");
  auto u = prs.tokenExpression();

  REQUIRE(equal(u, unreachable()));
}

/* ================== LetStatement ================== */

TEST_CASE("pub lets")
{
  auto prs = parser("pub let a = 0;");
  auto l = prs.letStatement();

  REQUIRE(equal(l,
    let(true, false, {
      part("a", nullptr, number("0"))
    })));
}

TEST_CASE("mut lets")
{
  auto prs = parser("let mut a = 0;");
  auto l = prs.letStatement();

  REQUIRE(equal(l,
    let(false, true, {
      part("a", nullptr, number("0"))
    })));
}

TEST_CASE("pub mut lets")
{
  auto prs = parser("pub let mut a = 0;");
  auto l = prs.letStatement();

  REQUIRE(equal(l,
    let(true, true, {
      part("a", nullptr, number("0"))
    })));
}

TEST_CASE("lets with types")
{
  auto prs = parser("let a: isize = 0;");
  auto l = prs.letStatement();

  REQUIRE(equal(l,
    let(false, false, {
      part("a", symbol("isize"), number("0"))
    })));
}

TEST_CASE("lets (multiple parts)")
{
  auto prs = parser("let a = 0, b = c, d = \"AAH\";");
  auto l = prs.letStatement();

  REQUIRE(equal(l,
    let(false, false, {
      part("a", nullptr, number("0")),
      part("b", nullptr, symbol("c")),
      part("d", nullptr, string("AAH"))
    })));
}

TEST_CASE("lets (complex)")
{
  auto prs = parser("pub let mut a: isize = 0, b = c, d: string = \"AAH\";");
  auto l = prs.letStatement();

  REQUIRE(equal(l,
    let(true, true, {
      part("a", symbol("isize"), number("0")),
      part("b", nullptr, symbol("c")),
      part("d", symbol("string"), string("AAH"))
    })));
}

/* ================== BlockExpression ================== */

// TODO

/* ================== FunctionExpression ================== */

// TODO

/* ================== TypeExpression ================== */

// TODO

TEST_SUITE_END();