#pragma once

#include <parsing/ast/LabeledNode.h>
#include <parsing/ast/BlockExpression.h>

namespace ast
{

struct LoopExpression final : public LabeledNode
{
  PTR(LoopExpression)

private:
  Token d_tokLoop;
  Node::SPtr d_pCondition;
  Node::SPtr d_pCapture;
  BlockExpression::SPtr d_pBody;
  Token d_tokElse;
  Node::SPtr d_pElseCapture;
  BlockExpression::SPtr d_pElseBody;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  LoopExpression(
    Token loop,
    Node::SPtr pCondition,
    Node::SPtr pCapture,
    BlockExpression::SPtr pBody,
    Token _else,
    Node::SPtr pElseCapture,
    BlockExpression::SPtr pElseBody,
    Node::SPtr pParent = nullptr);

  virtual Position start() const override { return d_tokLoop.start(); };
  virtual Position end() const override;

  // TODO check for break statements
  virtual bool isExpression() const override { return true; }

  Node::SPtr condition() const { return d_pCondition; }
  bool hasCapture() const { return d_pCapture != nullptr; }
  Node::SPtr capture() const { return d_pCapture; }
  BlockExpression::SPtr body() const { return d_pBody; }
  bool hasElseClause() const { return d_pElseBody != nullptr; }
  bool hasElseCapture() const { return d_pElseCapture != nullptr; }
  Node::SPtr elseCapture() const { return d_pElseCapture; }
  BlockExpression::SPtr elseBody() const { return d_pElseBody; }

  static LoopExpression::SPtr make_shared(
    Token loop,
    Node::SPtr pCondition,
    Node::SPtr pCapture,
    BlockExpression::SPtr pBody,
    Token _else,
    Node::SPtr pElseCapture,
    BlockExpression::SPtr pElseBody,
    Node::SPtr pParent = nullptr);
};

} // namespace ast