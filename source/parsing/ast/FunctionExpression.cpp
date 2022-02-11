#include "parsing/ast/FunctionExpression.h"

#include <parsing/ast/TokenExpressions.h>

using namespace ast;

struct ParameterNode final : public Node
{
  PTR(ParameterNode)

private:
  FunctionExpression::Parameter d_value;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  ParameterNode(FunctionExpression::Parameter value)
    : Node(nullptr)
    , d_value(value)
  {}

  virtual Position start() const override { return d_value.name.start(); }
  virtual Position end() const override { return d_value.type->end(); }
  virtual bool isExpression() const override { return false; }
};

void ParameterNode::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  subNodes->reserve(2);
  subNodes->push_back(std::make_shared<SymbolExpression>(d_value.name));
  subNodes->push_back(d_value.type);

  *nodeName = "Parameter";
  assert(additionalInfo->empty());
}

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
    subNodes->push_back(std::make_shared<ParameterNode>(parameter));
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
  std::vector<Parameter>&& parameters,
  Node::SPtr pReturnType,
  BlockExpression::SPtr pBody,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<FunctionExpression>(
    fn, std::move(parameters), pReturnType, pBody, pParent);

  for (auto& parameter : pRes->d_parameters)
  {
    parameter.type->setParent(pRes);
  }
  pRes->d_pReturnType->setParent(pRes);
  if (pRes->d_pBody != nullptr)
  {
    pRes->d_pBody->setParent(pRes);
  }
  return pRes;
}