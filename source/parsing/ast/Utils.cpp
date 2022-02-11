#include "parsing/ast/Utils.h"

#include <fmt/core.h>
#include <fmt/color.h>

// royal_blue -> dodger_blue,
// TODO emphasize node value, ex 'a', true, "AAAAH"

std::string hUnderline(std::string const& str)
{
  static constexpr auto style = fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green);
  static constexpr auto underlinedStyle = style | fmt::emphasis::underline;
  static constexpr size_t LEN = 1;

  return fmt::format(underlinedStyle, "{}", str.substr(0, LEN))
    + fmt::format(style, "{}", str.substr(LEN));
}

std::string hNormal(std::string const& str)
{
  static constexpr auto style = fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green);
  return fmt::format(style, "{}", str);
}

std::string header(std::string const& name, Position start, Position end, bool hasChildren)
{
  // TODO {:+1}
  static constexpr auto style = fmt::fg(fmt::color::yellow);
  auto const coloredStart = fmt::format(style, "{}", start);
  auto const coloredEnd = fmt::format(style, "{}", end);
  return fmt::format("{} ({}, {})", (hasChildren ? hUnderline(name) : hNormal(name)), coloredStart, coloredEnd);
}

std::string prefix(size_t indent, std::vector<size_t> lines, bool isLast)
{
  // │ ─ ┌ ┐ └ ┘ ┬ ┴ ├ ┤ ┼
  // ║ ═ ╔ ╗ ╚ ╝ ╦ ╩ ╠ ╣ ╬

  static constexpr auto style = fmt::fg(fmt::color::steel_blue);
  static constexpr auto
    middlePrefix = "├╾",  // ├─ ├╼ ├╾ ╞╾
    lastPrefix = "└╾",    // └─ ╰╼ ╰╾ ╘╾ ⸦
    longLine = "│ ";      // ┆ ┊ ╎

  std::string res = "";

  if (indent == 0)
  {
    return res;
  }

  if (indent == 1)
  {
    return fmt::format(style, "{}", (isLast ? lastPrefix : middlePrefix));
  }

  size_t front = 0;
  for (size_t i = 0; i < indent - 1; i += 1)
  {
    if (!lines.empty() && lines[front] == i)
    {
      res += longLine;
      front += 1;
    }
    else
    {
      res += "  ";
    }
  }
  res += (isLast ? lastPrefix : middlePrefix);

  return fmt::format(style, "{}", res);
}