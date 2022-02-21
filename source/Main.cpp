#include <Asserts.h>
#include <command/Commands.h>

#include <string_view>
#include <vector>

#include <fmt/core.h>

namespace
{

constexpr std::string_view helpString = R"TEXT(
Usage: mir [command] [options]

Commands:

  ast-dump      Print the syntax tree
  op-table      Print a table containing info about operators

Options:

  -h, --help    Print command-specific usage

)TEXT";

} // anonymous namespace

int main(int argc, char** argv)
{
  auto const [pathToSelf, cmmd, args] = [&]
  {
    std::vector<std::string_view> args;
    std::string_view cmmd;

    size_t argCount = static_cast<size_t>(argc);
    args.reserve(argCount);

    if (argCount >= 2)
    {
      cmmd = argv[1];
    }
    for (size_t i = 2; i < argCount; i += 1)
    {
      args.push_back(argv[i]);
    }
    return std::make_tuple(std::string_view(argv[0]), cmmd, std::move(args));
  }();

  if (cmmd.empty())
  {
    fmt::print("{}", helpString);
    return 0;
  }

  if (cmmd == command::OpTable::name)
  {
    return command::OpTable::exec(pathToSelf, args);
  }
  if (cmmd == command::AstDump::name)
  {
    return command::AstDump::exec(pathToSelf, args);
  }

  fmt::print("\nerror: unknown command '{}'\n{}", cmmd, helpString);
  return 0;
}

/* TODO

suggest location on missing {[()]} pairs by looking at alignments

{ <-- this stands out

  let a = blk: {

  }

...

Precedence      Expression
0               [1, 2, 3], []    // array literals
6               [5]a       []a   // array type literals

*/