#include "parsing/ast/BlockExpression.h"

#include <fmt/core.h>
#include <fmt/color.h>

using namespace ast;

void BlockExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  *subNodes = statements();
  *nodeName = "BlockExpression";
  *additionalInfo = isLabeled() ? fmt::format(fmt::emphasis::italic, "{}", label().text()) : "";
}

BlockExpression::SPtr BlockExpression::make_shared(
  Position start,
  Position end,
  std::vector<Node::SPtr>&& statements,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<BlockExpression>(
    start, end, std::move(statements), pParent);

  for (auto pStatement : pRes->d_statements)
  {
    pStatement->setParent(pRes);
  }
  return pRes;
}