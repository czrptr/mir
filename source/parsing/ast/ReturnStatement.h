#pragma once

#include <parsing/ast/Node.h>

namespace ast
{

struct ReturnStatement final : public Node
{
  PTR(ReturnStatement)

private:
  Token d_tokReturn;
  Node::SPtr d_pTarget;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  ReturnStatement(
    Token _return,
    Node::SPtr pTarget,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_tokReturn(_return)
    , d_pTarget(pTarget)
  {}

  virtual Position start() const override { return d_tokReturn.start(); }
  virtual Position end() const override;

  virtual bool isExpression() const override { return false; }

  Node::SPtr target() const { return d_pTarget; }

  static ReturnStatement::SPtr make_shared(
    Token _return,
    Node::SPtr pTarget,
    Node::SPtr pParent = nullptr);
};

} // namespace ast