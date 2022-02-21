#include "parsing/ast/FunctionExpression.h"

#include <parsing/ast/TokenExpressions.h>

using namespace ast;

void FunctionExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  // + 2 for the return type and body
  subNodes->reserve(parameters().size() + 2);
  for (auto const& parameter : parameters())
  {
    subNodes->push_back(parameter);
  }
  subNodes->push_back(returnType());
  if (!isType())
  {
    subNodes->push_back(body());
  }

  *nodeName = "FunctionLiteral";
  *additionalInfo = isType() ? "type" : "";
}

FunctionExpression::SPtr FunctionExpression::make_shared(
  Token fn,
  std::vector<Part::SPtr>&& parameters,
  Node::SPtr pReturnType,
  BlockExpression::SPtr pBody,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<FunctionExpression>(
    fn, std::move(parameters), pReturnType, pBody, pParent);

  for (auto& pParameter : pRes->d_parameters)
  {
    pParameter->setParent(pRes);
  }
  pRes->d_pReturnType->setParent(pRes);
  if (pRes->d_pBody != nullptr)
  {
    pRes->d_pBody->setParent(pRes);
  }
  return pRes;
}