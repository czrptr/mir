#include "command/AstDump.h"

#include <parsing/Error.h>
#include <parsing/Tokenizer.h>
#include <parsing/Parser.h>

#include <fmt/core.h>

using namespace command;

int AstDump::exec(std::string_view pathToSelf, std::vector<std::string_view> const& args)
{
  (void)pathToSelf;



  if (args.empty())
  {
    // TODO Refactor tokenizer to take a std::istream and not to construct one
    fmt::print("error: TODO ast-dump stdin\n");
    return 1;
  }

  std::string const pathToFile(args[0].data());

  try
  {
    auto parser = Parser(Tokenizer(pathToFile));
    auto const pAst = parser.typeExpression(true);
    fmt::print("\n{}\n\n", pAst->toString());
  }
  catch(Error const& err)
  {
    // TODO +1 to all line info
    fmt::print("\n{}\n\n", err);
  }

  // TODO parse arguments

  return 0;
}