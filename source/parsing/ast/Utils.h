#pragma once

#include <parsing/Position.h>

#include <string>
#include <vector>

std::string hUnderline(std::string const& str);
std::string hNormal(std::string const& str);
std::string header(std::string const& name, Position start, Position end, bool hasChildren);
std::string prefix(size_t indent, std::vector<size_t> lines, bool isLast);

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