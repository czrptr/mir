#include "parsing/ast/Nodes.h"

#include <algorithm>
#include <cassert>

#include <fmt/core.h>
#include <fmt/color.h>

/*
fmt::print("[{: >{}}]\n", "ceva", 8);
// royal_blue -> dodger_blue,
*/

namespace
{

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

template<typename T>
std::string childrenToString(std::vector<T> const& nodes, size_t indent, std::vector<size_t> lines)
{
  std::string res = "";
  for (size_t i = 0; i < nodes.size() - 1; i += 1)
  {
    auto newLines = lines; // TODO use set
    newLines.push_back(indent);
    std::sort(newLines.begin(), newLines.end());

    res += nodes[i]->toString(indent + 1, newLines, false) + "\n";
  }
  if (!nodes.empty())
  {
    res += nodes[nodes.size() - 1]->toString(indent + 1, lines, true);
  }
  return res;
}

// TODO emphasize node value, ex 'a', true, "AAAAH"

} // namespace anonymous

namespace ast
{

std::string Root::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  if (d_statements.empty())
  {
    return fmt::format("{}{}", prefix(indent, lines, isLast), header(nodeName(), start(), end(), false));
  }
  return fmt::format(
    "{}{}\n{}",
    prefix(indent, lines, isLast),
    header(nodeName(), start(), end(), true),
    childrenToString(d_statements, indent, lines));
}

std::string LetStatementPart::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{} '{}'\n{}",
    prefix(indent, lines, isLast),
    header(nodeName(), start(), end(), true),
    name(),
    value()->toString(indent + 1, lines, true));
}

std::string LetStatement::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{}{}{}\n{}",
    prefix(indent, lines, isLast),
    header(nodeName(), start(), end(), true),
    (d_isPub ? " pub" : ""),
    (d_isMut ? " mut" : ""),
    childrenToString(d_parts, indent, lines));
}

std::string SymbolExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{} '{}'",
    prefix(indent, lines, isLast),
    header(nodeName(), start(), end(), false),
    name());
}

std::string StringExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{} {} len {}",
    prefix(indent, lines, isLast),
    header(nodeName(), start(), end(), false),
    value(),
    unquotedValue().length());
}

std::string BoolExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{} {}",
    prefix(indent, lines, isLast),
    header(nodeName(), start(), end(), false),
    value());
}

std::string NullExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{}",
    prefix(indent, lines, isLast),
    header(nodeName(), start(), end(), false));
}

std::string UndefinedExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{}",
    prefix(indent, lines, isLast),
    header(nodeName(), start(), end(), false));
}

std::string UnreachableExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{}",
    prefix(indent, lines, isLast),
    header(nodeName(), start(), end(), false));
}

std::string BinaryExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{} '{}'\n{}",
    prefix(indent, lines, isLast),
    header(nodeName(), start(), end(), true),
    d_op,
    childrenToString(std::vector<Expression::SPtr>{d_lhs, d_rhs}, indent, lines));
}

} // namespace ast