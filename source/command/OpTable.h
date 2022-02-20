#pragma once

#include <parsing/Operator.h>

#include <string_view>
#include <vector>

namespace command
{

struct OpTable
{

static constexpr std::string_view name = "op-table";

static constexpr std::string_view helpString = R"TEXT(
Usage: mir op-table

  Print a table showing all language constructs and operators,
  their precedence, a short description, their chainability and
  associativity.

Options:

  --ascii             Print table frame using only ascii characters
  --color [on|off]    Enable or disable colored output
  -h, --help          Print this and exit

)TEXT";

static int exec(std::string_view pathToSelf, std::vector<std::string_view> const& args);

};

} // namespace command