#include "parsing/ast/DeferStatement.h"

using namespace ast;

void DeferStatement::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  subNodes->push_back(target());

  *nodeName = "Defer";

  *additionalInfo = "";
}

DeferStatement::SPtr DeferStatement::make_shared(
  Token defer,
  Node::SPtr pTarget,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<DeferStatement>(defer, pTarget, pParent);
  pTarget->setParent(pRes);
  return pRes;
}