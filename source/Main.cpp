#include <parsing/Position.h>
#include <parsing/Token.h>
#include <parsing/Error.h>
#include <parsing/Tokenizer.h>
#include <parsing/Parser.h>

#include <fmt/core.h>
#include <fmt/color.h>

#include <fstream>

#include <Asserts.h>

std::string sourceText =
R"SOURCE(

let NothingFn = fn () void;

let nothingFn: NothingFn = fn () void {};

pub let main = fn (args: ArgsType) void {
  let
    num = 0,
    str = "Hello, world!";
};

)SOURCE";

int main()
{
  sourceText = sourceText.substr(1, sourceText.length() - 2);
	try
  {
    Tokenizer tokenizer(sourceText, "<file>");
		auto tok = tokenizer.next();
		while(tok.tag() != Token::Eof)
		{
			fmt::print("{:s}\n", tok);
			tok = tokenizer.next();
		}
    fmt::print("{:s}\n\n", tok);

    Parser parser(Tokenizer(sourceText, "<file>"));

    auto expr = parser.typeExpression(true);
    fmt::print("{}\n", expr->toString());
  }
  catch (Error const& err)
	{
		fmt::print("\n{}", err);
	}
}