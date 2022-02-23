#include "parsing/ast/BreakStatement.h"

#include <fmt/core.h>
#include <fmt/color.h>

using namespace ast;

void BreakStatement::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  if (value() != nullptr)
  {
    subNodes->push_back(value());
  }

  *nodeName = "Break";

  *additionalInfo  = isLabeled() ? fmt::format(fmt::emphasis::italic, "{}", label().text()) : "";
}

Position BreakStatement::end() const
{
  if (value() != nullptr)
  {
    return value()->end();
  }
  if (isLabeled())
  {
    return label().end();
  }
  return d_tokBreak.end();
}

BreakStatement::SPtr BreakStatement::make_shared(
  Token _break,
  bool isLabeled,
  Token label,
  Node::SPtr pValue,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<BreakStatement>(_break, isLabeled, label, pValue, pParent);
  if (pValue != nullptr)
  {
    pValue->setParent(pRes);
  }
  return pRes;
}