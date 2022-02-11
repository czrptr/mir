#include <parsing/Position.h>
#include <parsing/Token.h>
#include <parsing/Error.h>
#include <parsing/Tokenizer.h>
#include <parsing/Parser.h>

#include <fmt/core.h>
#include <fmt/color.h>

#include <Asserts.h>

std::string sourceText =
R"SOURCE(
pub let
  a = b,
  c = d,
;

let mut e = f;
)SOURCE";

int main()
{
	sourceText = sourceText.substr(1, sourceText.length() - 2);
  fmt::print("{}\n\n", sourceText);

	Tokenizer tokenizer(sourceText);
	Parser parser(sourceText);
	try
  {
		auto tok = tokenizer.next();
		while(tok.tag() != Token::Eof)
		{
			fmt::print("{:ds}\n", tok);
			tok = tokenizer.next();
		}
    fmt::print("{:ds}\n\n", tok);

		auto pAstRoot = parser.parse();
		fmt::print("{}", pAstRoot->toString());
	}
	catch (Error const& err)
	{
		fmt::print("{}", err);
	}
  fmt::print("\n");
}
