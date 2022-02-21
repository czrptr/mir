#include "command/AstDump.h"

#include <parsing/Error.h>
#include <parsing/Tokenizer.h>
#include <parsing/Parser.h>

#include <fmt/core.h>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

using namespace command;

int AstDump::exec(std::string_view pathToSelf, std::vector<std::string_view> const& args)
{
  (void)pathToSelf;

  // TODO parse arguments

  ast::Node::SPtr pAst = nullptr;
  if (args.empty())
  {
    auto parser = Parser(Tokenizer(std::cin, "<stdout>"));

    fmt::print("Waiting for import from stdout...   (Use Ctrl+D to stop)\n\n");

    try
    {
      pAst = parser.root();
    }
    catch(Error const& err)
    {
      // TODO +1 to all line info
      fmt::print("\n{}\n\n", err);
    }
  }
  else
  {
    std::string const path = args[0].data();
    if (!fs::exists(path))
    {
      fmt::print("error: file '{}' doesn't exist", path);
    }

    auto fileStream = std::ifstream(path, std::ios::in);
    auto parser = Parser(Tokenizer(fileStream, path));
    try
    {
      pAst = parser.root();
    }
    catch(Error const& err)
    {
      // TODO +1 to all line info
      fmt::print("\n{}\n\n", err);
    }
    fileStream.close();
  }

  if (pAst == nullptr)
  {
    return 1;
  }

  fmt::print("\n{}\n\n", pAst->toString());

  return 0;
}