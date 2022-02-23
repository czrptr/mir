#include "parsing/ast/ReturnStatement.h"

using namespace ast;

void ReturnStatement::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  if (value() != nullptr)
  {
    subNodes->push_back(value());
  }

  *nodeName = "Return";

  *additionalInfo = "";
}

Position ReturnStatement::end() const
{
  if (value() != nullptr)
  {
    return value()->end();
  }
  return d_tokReturn.end();
}

ReturnStatement::SPtr ReturnStatement::make_shared(
  Token defer,
  Node::SPtr pValue,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<ReturnStatement>(defer, pValue, pParent);
  if (pValue != nullptr)
  {
    pValue->setParent(pRes);
  }
  return pRes;
}