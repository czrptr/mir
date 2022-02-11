#include "parsing/ast/FunctionExpression.h"

#include <parsing/ast/TokenExpressions.h>
#include <parsing/ast/Utils.h>

using namespace ast;

struct ParameterNode final : public Node
{
  PTR(ParameterNode)

private:
  FunctionExpression::Parameter d_value;

public:
  ParameterNode(FunctionExpression::Parameter value)
    : Node(nullptr)
    , d_value(value)
  {}

  virtual Position start() const override { return d_value.name.start(); }
  virtual Position end() const override { return d_value.type->end(); }
  virtual bool isExpression() const override { return false; }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;
};

std::string ParameterNode::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  std::vector<Node::SPtr> const members
  {
    std::make_shared<SymbolExpression>(d_value.name),
    d_value.type
  };

  return fmt::format(
    "{}{}\n{}",
    prefix(indent, lines, isLast),
    header("Parameter", start(), end(), true),
    childrenToString(members, indent, lines)
  );
}

std::string FunctionExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  std::vector<Node::SPtr> members;
  for (auto const& parameter : parameters())
  {
    members.push_back(std::make_shared<ParameterNode>(parameter));
  }
  members.push_back(returnType());
  if (body() != nullptr)
  {
    members.push_back(body());
  }

  return fmt::format(
    "{}{}{}\n{}",
    prefix(indent, lines, isLast),
    header("FunctionLiteral", start(), end(), true),
    (body() != nullptr ? " type" : ""),
    childrenToString(members, indent, lines)
  );
}

FunctionExpression::SPtr FunctionExpression::make_shared(
  Token fn,
  std::vector<Parameter>&& parameters,
  Node::SPtr pReturnType,
  Node::SPtr pBody,
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