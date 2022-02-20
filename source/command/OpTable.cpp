#include "command/OpTable.h"

#include <fmt/core.h>

using namespace command;

int OpTable::exec(std::string_view pathToSelf, std::vector<std::string_view> const& args)
{
  (void)pathToSelf;
  (void)args;

  // TODO parse arguments

  bool
    color = true,
    ascii = false;

  fmt::print("{}", Operator::tableWithInfo(color, ascii));

  return 0;
}