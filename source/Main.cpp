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
pub let mut
  a = b,
  c = d,
;

"gew  er" + 213.42
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
			fmt::print("{:ds}\n", tok);
			tok = tokenizer.next();
		}
    fmt::print("{:ds}\n", tok);

    // Parser parser(sourceText);
  	// auto pAstRoot = parser.parse();
		// fmt::print("{}", pAstRoot->toString());
  }
  catch (Error const& err)
	{
		fmt::print("{}", err);
	}
}
