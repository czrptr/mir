#include "parsing/ast/BlockExpression.h"

#include <parsing/ast/Utils.h>

using namespace ast;

std::string BlockExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  if (statements().empty())
  {
    return fmt::format(
      "{}{}{}",
      prefix(indent, lines, isLast),
      header("BlockExpression", start(), end(), false),
      (isLabeled() ? label() : "")); // TODO just use label?
  }
  return fmt::format(
    "{}{}{}\n{}",
    prefix(indent, lines, isLast),
    header("BlockExpression", start(), end(), true),
    (isLabeled() ? label() : ""),
    childrenToString(statements(), indent, lines));
}

BlockExpression::SPtr BlockExpression::make_shared(
  Position start,
  Position end,
  std::string_view label,
  std::vector<Node::SPtr>&& statements,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<BlockExpression>(
    start, end, label, std::move(statements), pParent);

  for (auto pStatement : pRes->d_statements)
  {
    pStatement->setParent(pRes);
  }
  return pRes;
}