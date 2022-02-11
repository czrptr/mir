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
a + b * c / d - e % f
)SOURCE";
//R"SOURCE(
// pub let mut
//  a = b .. c && d || e && f,
//   c = "AAAH";

// // let a = true;

// // pub let
// //   j = false,
// //   k = null,
// //   l = undefined,
// //   m = unreachable;
//)SOURCE";

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
    fmt::print("{:ds}\n\n", tok);

    Parser parser(Tokenizer(sourceText, "<file>"));
  	// auto pAstRoot = parser.parse();
		// fmt::print("{}\n", pAstRoot->toString());

    // auto pAAAH =
    //   std::dynamic_pointer_cast<ast::StringExpression>(
    //     std::dynamic_pointer_cast<ast::LetStatement>(
    //       pAstRoot->statements()[0])
    //         ->parts()[1]->value());

    // auto pRootFromLeaf =
    //   std::dynamic_pointer_cast<ast::Root>(
    //     pAAAH->parent<ast::LetStatementPart>().lock()->parent<ast::LetStatement>().lock()->parent<ast::Root>().lock());

    // fmt::print("\n\n{}", pAstRoot.get() == pRootFromLeaf.get());

    auto binaryExpr = parser.expression();
    fmt::print("{}\n", binaryExpr->toString());
  }
  catch (Error const& err)
	{
		fmt::print("{}", err);
	}
}