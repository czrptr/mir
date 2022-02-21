#pragma once

#include <string_view>
#include <vector>

namespace command
{

struct AstDump
{

// rename into ast-check
// add --dump flag

static constexpr std::string_view name = "ast-dump";

static constexpr std::string_view helpString = R"TEXT(
Usage: mir ast-dump [file]

  Given a .mir source file, reports any compile errors that can be
  ascertained on the basis of the source code alone. If no such
  errors are found, the syntax tree representation of the source
  file will be printed.

  if [file] is ommited, stdin is used.

Options:

  --ascii             Print tree using only ascii characters
  --color [on|off]    Enable or disable colored output
  -h, --help    Print this and exit

)TEXT";

static int exec(std::string_view pathToSelf, std::vector<std::string_view> const& args);

};

} // namespace command