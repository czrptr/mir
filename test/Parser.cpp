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

// TODO test non expressions, ex a = 0..2; enum (let b = 0) { ... }

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

/* ================== Structs ================== */

TEST_CASE("struct fields must be grouped together")
{
  auto prs = parser(
    "struct {\n"
    "  a: i128,\n"
    "  let A = 0;\n"
    "  b: f32,\n"
    "}");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:3:2: error: struct fields must be grouped together";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("struct fields must have type annotations")
{
  auto prs = parser("struct { a, }");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:9: error: struct fields must have type annotations";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("struct fields cannot have default values")
{
  auto prs = parser("struct { a: u0 = 0, }");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:9: error: struct fields cannot have default values";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("structs")
{
  auto prs = parser("struct {}");
  auto s = prs.expression();

  REQUIRE_AST_EQ(s, _struct({}, {}, {}));
}

TEST_CASE("structs (complex)")
{
  auto prs = parser(
    "struct {\n"
    "  let Self = @Self;\n"
    "\n"
    "  field: i2,\n"
    "\n"
    "  pub let a = 0;\n"
    "}");
  auto s = prs.expression();

  REQUIRE_AST_EQ(s, _struct({
    let(false, false, "Self", builtin("Self"))
  }, {
    field("field", symbol("i2"), nullptr)
  }, {
    let(true, false, "a", number("0"))
  }));
}

/* ================== Enums ================== */

TEST_CASE("enums with explicit underlying types must have underlying types")
{
  auto prs = parser("enum() {}");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:5: error: type expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("enum variants must be grouped together")
{
  auto prs = parser(
    "enum {\n"
    "  a,\n"
    "  let B = 0;\n"
    "  b,\n"
    "}");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:3:2: error: enum variants must be grouped together";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("enum variants cannot have type annotations")
{
  auto prs = parser("enum { a, }");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg =
        "<file>:0:7: error: enum variants cannot have type annotations"
        "<file>:0:7: note: all enum variants share the same underlying type";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("enums")
{
  auto prs = parser("enum {}");
  auto e = prs.expression();

  REQUIRE_AST_EQ(e, _enum({}, {}, {}));
}

TEST_CASE("enums with explicity underlying types")
{
  auto prs = parser("enum(u8) {}");
  auto e = prs.expression();

  REQUIRE_AST_EQ(e, _enum(symbol("u8"), {}, {}, {}));
}

TEST_CASE("enum (complex)")
{
  auto prs = parser(
    "enum (ceva) {\n"
    "  let Self = @Self;\n"
    "\n"
    "  a = 0,\n"
    "  b,\n"
    "  c = 1\n"
    "\n"
    "  pub let a = 0;\n"
    "}");
  auto e = prs.expression();

  REQUIRE_AST_EQ(e, _enum( symbol("ceva"), {
    let(false, false, "Self", builtin("Self"))
  }, {
    field("a", nullptr, number("0")),
    field("b", nullptr, nullptr),
    field("c", nullptr, number("1"))
  }, {
    let(true, false, "a", number("0"))
  }));
}

/* ================== Unions ================== */

TEST_CASE("union variants must be grouped together")
{
  auto prs = parser(
    "union {\n"
    "  a: i128,\n"
    "  let A = 0;\n"
    "  b: f32,\n"
    "}");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:3:2: error: union variants must be grouped together";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("union variants must have type annotations")
{
  auto prs = parser("union { a, }");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:8: error: union variants must have type annotations";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("union variants cannot have default values")
{
  auto prs = parser("union { a: u0 = 0, }");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:8: error: union variants cannot have default values";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("unions")
{
  auto prs = parser("union {}");
  auto u = prs.expression();

  REQUIRE_AST_EQ(u, _union({}, {}, {}));
}

TEST_CASE("unions (complex)")
{
  auto prs = parser(
    "union {\n"
    "  let Self = @Self;\n"
    "\n"
    "  v1: i2,\n"
    "  v2: i4,\n"
    "\n"
    "  pub let a = 0;\n"
    "}");
  auto u = prs.expression();

  REQUIRE_AST_EQ(u, _union({
    let(false, false, "Self", builtin("Self"))
  }, {
    field("v1", symbol("i2"), nullptr),
    field("v2", symbol("i4"), nullptr)
  }, {
    let(true, false, "a", number("0"))
  }));
}

TEST_SUITE_END();