#include "parsing/ast/ReturnStatement.h"

using namespace ast;

void ReturnStatement::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  if (target() != nullptr)
  {
    subNodes->push_back(target());
  }

  *nodeName = "Return";

  *additionalInfo = "";
}

Position ReturnStatement::end() const
{
  if (target() != nullptr)
  {
    return target()->end();
  }
  return d_tokReturn.end();
}

ReturnStatement::SPtr ReturnStatement::make_shared(
  Token defer,
  Node::SPtr pTarget,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<ReturnStatement>(defer, pTarget, pParent);
  if (pTarget != nullptr)
  {
    pTarget->setParent(pRes);
  }
  return pRes;
}