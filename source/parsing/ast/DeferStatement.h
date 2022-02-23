#pragma once

#include <parsing/ast/Node.h>

namespace ast
{

struct DeferStatement final : public Node
{
  PTR(DeferStatement)

private:
  Token d_tokDefer;
  Node::SPtr d_pTarget;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  DeferStatement(
    Token defer,
    Node::SPtr pTarget,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_tokDefer(defer)
    , d_pTarget(pTarget)
  {}

  virtual Position start() const override { return d_tokDefer.start(); }
  virtual Position end() const override { return d_pTarget->end(); }

  virtual bool isExpression() const override { return false; }

  Node::SPtr target() const { return d_pTarget; }

  static DeferStatement::SPtr make_shared(
    Token defer,
    Node::SPtr pTarget,
    Node::SPtr pParent = nullptr);
};

} // namespace ast