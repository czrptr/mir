#include <parsing/Position.h>
#include <parsing/Token.h>
#include <parsing/Error.h>
#include <parsing/Tokenizer.h>
#include <parsing/Parser.h>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fort.hpp>

#include <fstream>

#include <Asserts.h>

std::string sourceText =
R"SOURCE(

let c = switch number {
  0 => false,
  1 => true,
};

let b = loop cond |value| {} else |err| {};

a: a,
b: b,

let MyEnum = enum u8 {
  a = 0,
  b = 2,
};

let nothingFn: NothingFn = fn () void {};

pub let main = fn (args: ArgsType) void {
  let
    num = 0,
    str = "Hello, world!";
};

let a = if 0 |a| {} else if 2 |b| {} else |err| {};

)SOURCE";

int main()
{
  sourceText = sourceText.substr(1, sourceText.length() - 2);
	try
  {
    // Tokenizer tokenizer(sourceText, "<file>");
		// auto tok = tokenizer.next();
		// while(tok.tag() != Token::Eof)
		// {
		// 	fmt::print("{:s}\n", tok);
		// 	tok = tokenizer.next();
		// }
    // fmt::print("{:s}\n\n", tok);

    Parser parser(Tokenizer(sourceText, "<file>"));

    auto expr = parser.typeExpression(true);
    if (expr != nullptr)
    {
      // fmt::print("{}\n", expr->toString());
    }
    else
    {
      fmt::print("didn't parse anything\n");
    }
  }
  catch (Error const& err)
	{
		fmt::print("\n{}", err);
	}

  fmt::print("\n{}\n", Operator::tableWithInfo());
}

/* TODO

suggest location on missing {[()]} pairs by looking at alignments

{ <-- this stands out

  let a = blk: {

  }

...

*/