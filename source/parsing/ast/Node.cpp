#include "parsing/ast/Node.h"

#include <fmt/core.h>
#include <fmt/color.h>

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
    if (front < lines.size() && lines[front] == i)
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
  if (nodes.empty())
  {
    return res;
  }

  for (size_t i = 0; i < nodes.size() - 1; i += 1)
  {
    auto newLines = lines; // TODO use set
    newLines.push_back(indent);
    std::sort(newLines.begin(), newLines.end());

    res += nodes[i]->toString(indent + 1, newLines, false) + "\n";
  }
  res += nodes[nodes.size() - 1]->toString(indent + 1, lines, true);

  return res;
}

using namespace ast;

std::string Node::toString(size_t indent = 0, std::vector<size_t> lines = {}, bool isLast = false) const
{
  std::vector<Node::SPtr> subNodes;
  std::string name, additionalInfo;
  toStringData(&subNodes, &name, &additionalInfo);

  return fmt::format(
    "{}{}{}{}",
    prefix(indent, lines, isLast),
    header(name, start(), end(), !subNodes.empty()),
    (additionalInfo.empty() ? "" : (" " + additionalInfo)),
    (subNodes.empty() ? "" : ("\n" + childrenToString(subNodes, indent, lines))));
}