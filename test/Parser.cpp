#include <doctest.h>
#include <ParsingUtils.h>
#include <parsing/Parser.h>

#define REQUIRE_AST_EQ(ast1, ast2) REQUIRE(equal((ast1), (ast2)))

TEST_SUITE_BEGIN("Parser");

/* TODO test non expressions, ex:
  a: 0..2 = 0..2;
  enum (let b = 0) { ... }
  struct let a = 0

  if cond |capture?| // will operator parsing ever backfire?

  if cond |. capture the operator and split it

  destructuring check on let statements

  comptime tests
*/

/* ================== TokenExpression ================== */

TEST_CASE("symbols")
{
  PARSER_TEXT("main Ab__2c");

  auto s1 = prs.expression();
  auto s2 = prs.expression();

  REQUIRE_AST_EQ(s1, symbol("main"));
  REQUIRE_AST_EQ(s2, symbol("Ab__2c"));
}

TEST_CASE("builtins")
{
  PARSER_TEXT("@TypeOf @type_info");
  auto b1 = prs.expression();
  auto b2 = prs.expression();

  REQUIRE_AST_EQ(b1, builtin("TypeOf"));
  REQUIRE_AST_EQ(b2, builtin("type_info"));
}

TEST_CASE("string literals")
{
  PARSER_TEXT("\"er gw \\r etewr\" \"\"");
  auto s1 = prs.expression();
  auto s2 = prs.expression();

  REQUIRE_AST_EQ(s1, string("er gw \\r etewr"));
  REQUIRE_AST_EQ(s2, string(""));
}

TEST_CASE("number literals")
{
  PARSER_TEXT("0004324 4353.43463");
  auto n1 = prs.expression();
  auto n2 = prs.expression();

  REQUIRE_AST_EQ(n1, number("0004324"));
  REQUIRE_AST_EQ(n2, number("4353.43463"));
}

TEST_CASE("boolean literals")
{
  PARSER_TEXT("true false");
  auto b1 = prs.expression();
  auto b2 = prs.expression();

  REQUIRE_AST_EQ(b1, boolean(true));
  REQUIRE_AST_EQ(b2, boolean(false));
}

TEST_CASE("null")
{
  PARSER_TEXT("null");
  auto n = prs.expression();

  REQUIRE_AST_EQ(n, null());
}

TEST_CASE("undefined")
{
  PARSER_TEXT("undefined");
  auto u = prs.expression();

  REQUIRE_AST_EQ(u, undefined());
}

TEST_CASE("unreachable")
{
  PARSER_TEXT("unreachable");
  auto u = prs.expression();

  REQUIRE_AST_EQ(u, unreachable());
}

/* ================== LetStatement ================== */

TEST_CASE("let statements cannot be empty")
{
  PARSER_TEXT("let");

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

TEST_CASE("use 'undefined' to leave variables uninitialzed")
{
  PARSER_TEXT("let mut a = undefined");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:8: error: assign 'undefined' to leave variables uninitialzed";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("constants must be initialized")
{
  PARSER_TEXT("let a");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:4: error: constants must be initialized";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("constants must be initialized with a proper value")
{
  PARSER_TEXT("let a = undefined");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:4: error: constants must be initialized with a proper value";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("let statement consistent 'comptime' placement")
{
  PARSER_TEXT("let comptime mut a = a");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:4: error: move the 'comptime' keyword before the 'let'";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("individual let statement parts cannot be comptime")
{
  PARSER_TEXT(
    "let mut\n"
    "  a = a,\n"
    "  comptime b = b,"
  );

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg =
        "<file>:2:2: error: individual let statement parts cannot be comptime\n"
        "<file>:0:0: note: mark the whole statement as comptime here";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("let statement parts must be separated by a comma")
{
  PARSER_TEXT("let a = a b = b");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:9: error: ',' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("let statement parts can have a trailing comma")
{
  PARSER_TEXT("let a = a,");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(false, false, "a", nullptr, symbol("a")));
}

TEST_CASE("let statement parts trailing comma is optional")
{
  PARSER_TEXT("let a = a");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(false, false, "a", nullptr, symbol("a")));
}

TEST_CASE("let statement parts must be assigned a value")
{
  PARSER_TEXT("let a = ");

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
  PARSER_TEXT("pub let a = 0");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(true, false, "a", number("0")));
}

TEST_CASE("mut let statements")
{
  PARSER_TEXT("let mut a = 0");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(false, true, "a", number("0")));
}

TEST_CASE("pub mut let statements")
{
  PARSER_TEXT("pub let mut a = 0");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(true, true, "a", number("0")));
}

TEST_CASE("let statements with types")
{
  PARSER_TEXT("let a: isize = 0");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(false, false, "a", symbol("isize"), number("0")));
}

TEST_CASE("let statements (multiple parts)")
{
  PARSER_TEXT("let a = 0, b = c, d = \"AAH\"");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(false, false, {
      part(symbol("a"), number("0")),
      part(symbol("b"), symbol("c")),
      part(symbol("d"), string("AAH"))
    }));
}

TEST_CASE("let statements (complex)")
{
  PARSER_TEXT("pub let mut a: isize = 0, b = c, d: string = \"AAH\"");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    let(true, true, {
      part(symbol("a"), symbol("isize"), number("0")),
      part(symbol("b"), symbol("c")),
      part(symbol("d"), symbol("string"), string("AAH"))
    }));
}

/* ================== BlockExpression ================== */

TEST_CASE("block labels")
{
  PARSER_TEXT("labelName : {}");

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

TEST_CASE("blocks must end")
{
  PARSER_TEXT("{");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:1: error: '}' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("only blocks, if and loops can be labeled")
{
  PARSER_TEXT("labelName: let a = 0;");

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

TEST_CASE("superfluous statement enders are illigal (part 1)")
{
  PARSER_TEXT("{ a;;; }");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:4: error: remove superfluous ';'";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("blocks")
{
  PARSER_TEXT("{} {{}}");
  auto b1 = prs.expression();
  auto b2 = prs.expression();

  REQUIRE_AST_EQ(b1, block({}));
  REQUIRE_AST_EQ(b2, block({block({})}));
}

TEST_CASE("labeled blocks")
{
  PARSER_TEXT("blk: {}");
  auto b = prs.expression();

  REQUIRE_AST_EQ(b, block("blk", {}));
}

TEST_CASE("blocks (complex)")
{
  PARSER_TEXT(
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

// TODO
//  "fn() Type {}" not being parsed as "fn() (Type{})""
//  tests for Part parameters refactor

TEST_CASE("function parameters must be separated by a comma")
{
  PARSER_TEXT("fn (a: a a: a) a");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:8: error: ',' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("function parameters can have a trailing comma")
{
  PARSER_TEXT("fn (a: a,) a");
  auto f = prs.expression();

  REQUIRE_AST_EQ(f, fn({part(symbol("a"), symbol("a"), nullptr)}, symbol("a")));
}

TEST_CASE("function parameters must have types (colon separator)")
{
  PARSER_TEXT("fn (a) a");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:5: error: ':' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("function parameters must have types")
{
  PARSER_TEXT("fn (a:) a");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:6: error: type expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("function return type must be an expression")
{
  PARSER_TEXT("fn () let a = 0");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:6: error: type expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("function block cannot be labeled")
{
  PARSER_TEXT("fn () a a: {}");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:8: error: function blocks cannot be labeled";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("only blocks accepted after function return types")
{
  PARSER_TEXT("fn () a a");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      // TODO better error location
      expectedMsg = "<file>:0:8: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("function type")
{
  PARSER_TEXT("fn () void");
  auto f = prs.expression();

  REQUIRE_AST_EQ(f, fn({}, symbol("void")));
}

TEST_CASE("function type (complex)")
{
  PARSER_TEXT("fn (arg1: Type1, arg2: char) fn () void");
  auto f = prs.expression();

  REQUIRE_AST_EQ(f,
    fn(
      {
        part(symbol("arg1"), symbol("Type1"), nullptr),
        part(symbol("arg2"), symbol("char"), nullptr)
      },
      fn({}, symbol("void"))));
}

TEST_CASE("function")
{
  PARSER_TEXT("fn () void {}");
  auto f = prs.expression();

  REQUIRE_AST_EQ(f, fn({}, symbol("void"), block({})));
}

TEST_CASE("function (complex)")
{
  PARSER_TEXT(
    "fn () void {\n"
    "  let a = b;\n"
    "}");
  auto f = prs.expression();

  REQUIRE_AST_EQ(f, fn({}, symbol("void"),
    block({let(false, false, "a", symbol("b"))})));
}

/* ================== Struct ================== */

TEST_CASE("structs must have blocks")
{
  PARSER_TEXT("struct");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:7: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("struct fields cannot be comptime")
{
  PARSER_TEXT("struct { comptime a: a }");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:18: error: struct fields cannot be comptime";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("struct fields must be separated by a comma")
{
  PARSER_TEXT("struct { a: a b: b }");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:13: error: ',' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("struct fields can have a trailing comma")
{
  PARSER_TEXT("struct { a: a, }");
  auto s = prs.expression();

  REQUIRE_AST_EQ(s, _struct(
    {},
    {field("a", symbol("a"), nullptr)},
    {}));
}

TEST_CASE("struct fields trailing comma is optional")
{
  PARSER_TEXT("struct { a: a,}");
  auto s = prs.expression();

  REQUIRE_AST_EQ(s, _struct(
    {},
    {field("a", symbol("a"), nullptr)},
    {}));
}

TEST_CASE("struct fields must be grouped together")
{
  PARSER_TEXT(
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
  PARSER_TEXT("struct { a, }");
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
  PARSER_TEXT("struct { a: u0 = 0, }");
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

TEST_CASE("superfluous statement enders are illigal (part 2)")
{
  PARSER_TEXT("struct { let a = a;;; }");

  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:19: error: remove superfluous ';'";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("structs")
{
  PARSER_TEXT("struct {}");
  auto s = prs.expression();

  REQUIRE_AST_EQ(s, _struct({}, {}, {}));
}

TEST_CASE("structs (complex)")
{
  PARSER_TEXT(
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

/* ================== Enum ================== */

TEST_CASE("enums must have blocks (part 1)")
{
  PARSER_TEXT("enum");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:5: error: type expression or block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("enums must have blocks (part 2)")
{
  PARSER_TEXT("enum a");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:7: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("enum variants must be separated by a comma (case 1)")
{
  PARSER_TEXT("enum { a b = b }");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:8: error: ',' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("enum variants cannot be comptime")
{
  PARSER_TEXT("enum { comptime a = 0}");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:16: error: enum variants cannot be comptime";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("enum variants must be separated by a comma (case 2)")
{
  PARSER_TEXT("enum { a = a b = b }");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:12: error: ',' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("enum variants can have a trailing comma (case 1)")
{
  PARSER_TEXT("enum { a, }");
  auto e = prs.expression();

  REQUIRE_AST_EQ(e, _enum(
    {},
    {field("a", nullptr, nullptr)},
    {}));
}

TEST_CASE("enum variants can have a trailing comma (case 2)")
{
  PARSER_TEXT("enum { a = a, }");
  auto e = prs.expression();

  REQUIRE_AST_EQ(e, _enum(
    {},
    {field("a", nullptr, symbol("a"))},
    {}));
}

TEST_CASE("enum variants trailing comma is optional (case 1)")
{
  PARSER_TEXT("enum { a, }");
  auto e = prs.expression();

  REQUIRE_AST_EQ(e, _enum(
    {},
    {field("a", nullptr, nullptr)},
    {}));
}

TEST_CASE("enum variants trailing comma is optional (case 2)")
{
  PARSER_TEXT("enum { a = a }");
  auto e = prs.expression();

  REQUIRE_AST_EQ(e, _enum(
    {},
    {field("a", nullptr, symbol("a"))},
    {}));
}

TEST_CASE("enum variants must be grouped together")
{
  PARSER_TEXT(
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
  PARSER_TEXT("enum { a, }");
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
  PARSER_TEXT("enum {}");
  auto e = prs.expression();

  REQUIRE_AST_EQ(e, _enum({}, {}, {}));
}

TEST_CASE("enums with explicity underlying types")
{
  PARSER_TEXT("enum u8 {}");
  auto e = prs.expression();

  REQUIRE_AST_EQ(e, _enum(symbol("u8"), {}, {}, {}));
}

TEST_CASE("enum (complex)")
{
  PARSER_TEXT(
    "enum ceva {\n"
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

/* ================== Union ================== */


TEST_CASE("unions must have blocks")
{
  PARSER_TEXT("union");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:6: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("union variants must be separated by a comma")
{
  PARSER_TEXT("union { a: a b: b }");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:12: error: ',' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("union variants can have a trailing comma")
{
  PARSER_TEXT("union { a: a, }");
  auto u = prs.expression();

  REQUIRE_AST_EQ(u, _union(
    {},
    {field("a", symbol("a"), nullptr)},
    {}));
}

TEST_CASE("union variants cannot be comptime")
{
  PARSER_TEXT("union { comptime a: a}");
  try
  {
    prs.expression();
  }
  catch(Error const& err)
  {
    std::string const
      msg = fmt::to_string(err),
      expectedMsg = "<file>:0:17: error: union variants cannot be comptime";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("union variants trailing comma is optional")
{
  PARSER_TEXT("union { a: a,}");
  auto u = prs.expression();

  REQUIRE_AST_EQ(u, _union(
    {},
    {field("a", symbol("a"), nullptr)},
    {}));
}

TEST_CASE("union variants must be grouped together")
{
  PARSER_TEXT(
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
  PARSER_TEXT("union { a, }");
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
  PARSER_TEXT("union { a: u0 = 0, }");
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
  PARSER_TEXT("union {}");
  auto u = prs.expression();

  REQUIRE_AST_EQ(u, _union({}, {}, {}));
}

TEST_CASE("unions (complex)")
{
  PARSER_TEXT(
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

/* ================== IfExpressions ================== */

TEST_CASE("if clauses must have conditions")
{
  PARSER_TEXT("if |a| {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:3: error: if clauses must have conditions";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else if clauses must have conditions")
{
  PARSER_TEXT("else if |a| {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:8: error: else if clauses must have conditions";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else clause must have a preceding if clause")
{
  PARSER_TEXT("else {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:0: error: else clause must have a preceding if clause";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else if clause must have a preceding if clause")
{
  PARSER_TEXT("else if a {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:0: error: else if clause must have a preceding if clause";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else clauses don't have conditions")
{
  PARSER_TEXT("else a {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:5: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("if clauses unfinished capture (part 1)")
{
  PARSER_TEXT("if a |a {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:7: error: '|' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("if clauses unfinished capture (part 2)")
{
  PARSER_TEXT("if a |");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:6: error: destructuring expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else if clauses unfinished capture (part 1)")
{
  PARSER_TEXT("if a {} else if a |a {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:20: error: '|' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else if clauses unfinished capture (part 2)")
{
  PARSER_TEXT("if a {} else if a |");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:19: error: destructuring expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else clauses unfinished capture (part 1)")
{
  PARSER_TEXT("if a {} else |a {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:15: error: '|' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else clauses unfinished capture (part 2)")
{
  PARSER_TEXT("if a {} else |");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:14: error: destructuring expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("if clauses must have blocks (part 1)")
{
  PARSER_TEXT("if a");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:5: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("if clauses must have blocks (part 2)")
{
  PARSER_TEXT("if a |a|");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:9: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else if clauses must have blocks (part 1)")
{
  PARSER_TEXT("if a else if b");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:15: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else if clauses must have blocks (part 2)")
{
  PARSER_TEXT("if a else if b |b|");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:19: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else clauses must have blocks (part 1)")
{
  PARSER_TEXT("if a else");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:10: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("else clauses must have blocks (part 2)")
{
  PARSER_TEXT("if a else |b|");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:14: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("missing block inside internal clause")
{
  PARSER_TEXT("if a {} else if b else {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:18: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("whole if must be labeled (part 1)")
{
  PARSER_TEXT("if a blk: {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg =
      "<file>:0:5: error: individual clause blocks cannot be labeled\n"
      "<file>:0:0: note: place the label 'blk:' here";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("whole if must be labeled (part 2)")
{
  PARSER_TEXT("if a {} else if a blk: {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg =
      "<file>:0:18: error: individual clause blocks cannot be labeled\n"
      "<file>:0:0: note: place the label 'blk:' here";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("whole if must be labeled (part 3)")
{
  PARSER_TEXT("if a {} else blk: {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg =
      "<file>:0:13: error: individual clause blocks cannot be labeled\n"
      "<file>:0:0: note: place the label 'blk:' here";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("if")
{
  PARSER_TEXT("if a {}");
  auto i = prs.expression();

  REQUIRE_AST_EQ(i, _if(symbol("a"), nullptr, block({})));
}

TEST_CASE("labeled if")
{
  PARSER_TEXT("blk: if a {}");
  auto i = prs.expression();

  REQUIRE_AST_EQ(i, _if("blk", symbol("a"), nullptr, block({})));
}

TEST_CASE("if with capture")
{
  PARSER_TEXT("if a |a| {}");
  auto i = prs.expression();

  REQUIRE_AST_EQ(i, _if(symbol("a"), symbol("a"), block({})));
}

TEST_CASE("labeled if with capture")
{
  PARSER_TEXT("blk: if a |a| {}");
  auto i = prs.expression();

  REQUIRE_AST_EQ(i, _if("blk", symbol("a"), symbol("a"), block({})));
}

TEST_CASE("else if")
{
  PARSER_TEXT("if a {} else if b {}");
  auto i = prs.expression();

  // fmt::print("\n{}\n", i->toString());

  REQUIRE_AST_EQ(i, _if(symbol("a"), nullptr, block({}), {
    _elseIf(symbol("b"), nullptr, block({}))
  }));
}

TEST_CASE("else if with capture")
{
  PARSER_TEXT("if a {} else if b |b| {}");
  auto i = prs.expression();

  REQUIRE_AST_EQ(i, _if(symbol("a"), nullptr, block({}), {
    _elseIf(symbol("b"), symbol("b"), block({}))
  }));
}

TEST_CASE("else")
{
  PARSER_TEXT("if a {} else {}");
  auto i = prs.expression();

  REQUIRE_AST_EQ(i, _if(symbol("a"), nullptr, block({}), {
    _else(nullptr, block({}))
  }));
}

TEST_CASE("else with capture")
{
  PARSER_TEXT("if a {} else |b| {}");
  auto i = prs.expression();

  REQUIRE_AST_EQ(i, _if(symbol("a"), nullptr, block({}), {
    _else(symbol("b"), block({}))
  }));
}

TEST_CASE("if (complex)")
{
  PARSER_TEXT(
    "blk: if a |a| {} else if b {} "
    "else if c |c| {} else |d| {}");
  auto i = prs.expression();

  REQUIRE_AST_EQ(i, _if("blk", symbol("a"), symbol("a"), block({}), {
    _elseIf(symbol("b"), nullptr, block({})),
    _elseIf(symbol("c"), symbol("c"), block({})),
    _else(symbol("d"), block({}))
  }));
}

/* ================== LoopExpressions ================== */ //

TEST_CASE("loops must have conditions")
{
  PARSER_TEXT("loop |a| {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:5: error: expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("loops must have conditions")
{
  PARSER_TEXT("loop |a| {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:5: error: expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("loop unfinished capture (part 1)")
{
  PARSER_TEXT("loop a |a");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:9: error: '|' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("loop unfinished capture (part 2)")
{
  PARSER_TEXT("loop a |");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:8: error: destructuring expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("loops must have blocks (part 1)")
{
  PARSER_TEXT("loop a");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:7: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("loops must have blocks (part 2)")
{
  PARSER_TEXT("loop a else {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:7: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("loop else clause unfinished capture (part 1)")
{
  PARSER_TEXT("loop a {} else |a");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:17: error: '|' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("loop else clause unfinished capture (part 2)")
{
  PARSER_TEXT("loop a {} else |");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:16: error: destructuring expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("loop else clauses must have blocks")
{
  PARSER_TEXT("loop a {} else");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:15: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("loop else clauses don't have conditions")
{
  PARSER_TEXT("loop a {} else a");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:15: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}


TEST_CASE("whole loop must be labeled (part 1)")
{
  PARSER_TEXT("loop a blk: {} else {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg =
      "<file>:0:7: error: individual clause blocks cannot be labeled\n"
      "<file>:0:0: note: place the label 'blk:' here";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("whole loop must be labeled (part 2)")
{
  PARSER_TEXT("loop a {} else blk: {}");
  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg =
      "<file>:0:15: error: individual clause blocks cannot be labeled\n"
      "<file>:0:0: note: place the label 'blk:' here";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("loop")
{
  PARSER_TEXT("loop a |a| {}");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l, loop(symbol("a"), symbol("a"), block({})));
}

TEST_CASE("loop (complex)")
{
  PARSER_TEXT("blk: loop a |a| {} else |b| {}");
  auto l = prs.expression();

  REQUIRE_AST_EQ(l,
    loop("blk",
      symbol("a"), symbol("a"), block({}),
      symbol("b"), block({})));
}

/* ================== SwitchExpression ================== */

TEST_CASE("switches must have values")
{
  PARSER_TEXT("switch {}");

  // TODO for now the block parses as the switch value
  //   BlockExpression::isExpression() needs to be
  //   implemented properly to fix this

  // try
  // {
  //   prs.expression();
  // }
  // catch (Error const& err)
  // {
  //   std::string const
  //   msg = fmt::to_string(err),
  //   expectedMsg = "<file>:0:6: error: expression expected";

  //   REQUIRE_EQ(msg, expectedMsg);
  // }
}

TEST_CASE("switches must have blocks")
{
  PARSER_TEXT("switch a");

  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:9: error: block expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("switch block cannot be labeled")
{
  PARSER_TEXT("switch a blk: {}");

  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:9: error: switch blocks cannot be labeled";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("switch arrow expected")
{
  PARSER_TEXT("switch a { a a }");

  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:13: error: '=>' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("switch case result expected")
{
  PARSER_TEXT("switch a { a =>");

  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:16: error: expression expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("switch block must end")
{
  PARSER_TEXT("switch a { a => a");

  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:18: error: '}' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("switch cases must be separated by a comma")
{
  PARSER_TEXT("switch a { a => a a => a }");

  try
  {
    prs.expression();
  }
  catch (Error const& err)
  {
    std::string const
    msg = fmt::to_string(err),
    expectedMsg = "<file>:0:17: error: ',' expected";

    REQUIRE_EQ(msg, expectedMsg);
  }
}

TEST_CASE("switch cases can have a trailing comma")
{
  PARSER_TEXT("switch a { a => a, }");
  auto s = prs.expression();

  REQUIRE_AST_EQ(s,
    _switch(symbol("a"), {
      { symbol("a"), nullptr, symbol("a")}
    }));
}

TEST_CASE("switch (complex)")
{
  PARSER_TEXT("switch a { a => a, b |c| => {} }");
  auto s = prs.expression();

  REQUIRE_AST_EQ(s,
    _switch(symbol("a"), {
      {symbol("a"), nullptr, symbol("a")},
      {symbol("b"), symbol("c"), block({})},
    }));
}

TEST_SUITE_END();