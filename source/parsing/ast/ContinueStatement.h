#pragma once

#include <parsing/ast/Node.h>

namespace ast
{

struct ContinueStatement final : public Node
{
  PTR(ContinueStatement)

private:
  Token d_tokContinue;
  bool d_isLabeled;
  Token d_tokLabel;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  ContinueStatement(
    Token _continue,
    bool isLabeled,
    Token label,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_tokContinue(_continue)
    , d_isLabeled(isLabeled)
    , d_tokLabel(label)
  {}

  virtual Position start() const override { return d_tokContinue.start(); }
  virtual Position end() const override;

  virtual bool isExpression() const override { return false; }

  bool isLabeled() const { return d_isLabeled; }
  Token label() const { return d_tokLabel; }
};

} // namespace ast