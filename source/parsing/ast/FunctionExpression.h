#pragma once

#include <parsing/ast/Node.h>

namespace ast
{

struct FunctionExpression final : public Node
{
  PTR(FunctionExpression)

public:
  struct Parameter
  {
    Token name;
    Node::SPtr type;
  };

private:
  Token d_fn;
  std::vector<Parameter> d_parameters;
  Node::SPtr d_pReturnType;
  Node::SPtr d_pBody;

public:
  FunctionExpression(
    Token fn,
    std::vector<Parameter>&& parameters,
    Node::SPtr pReturnType,
    Node::SPtr pBody = nullptr,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_fn(fn)
    , d_parameters(std::move(parameters))
    , d_pReturnType(pReturnType)
    , d_pBody(pBody)
  {}

  virtual Position start() const override { return d_fn.start(); }
  virtual Position end() const override { return (d_pBody != nullptr ? d_pBody : d_pReturnType)->end(); }
  virtual bool isExpression() const override { return true; }

  std::vector<Parameter> const& parameters() const { return d_parameters; }
  Node::SPtr returnType() const { return d_pReturnType; }
  Node::SPtr body() const { return d_pBody; }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;

  static FunctionExpression::SPtr make_shared(
    Token fn,
    std::vector<Parameter>&& arguments,
    Node::SPtr pReturnType,
    Node::SPtr pBody = nullptr,
    Node::SPtr pParent = nullptr);
};

} // namespace ast