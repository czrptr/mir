#pragma once

#include <parsing/ast/Node.h>

namespace ast
{

struct BreakStatement final : public Node
{
  PTR(BreakStatement)

private:
  Token d_tokBreak;
  bool d_isLabeled;
  Token d_tokLabel;
  Node::SPtr d_pValue;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  BreakStatement(
    Token _break,
    bool isLabeled,
    Token label,
    Node::SPtr pValue,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_tokBreak(_break)
    , d_isLabeled(isLabeled)
    , d_tokLabel(label)
    , d_pValue(pValue)
  {}

  virtual Position start() const override { return d_tokBreak.start(); }
  virtual Position end() const override;

  virtual bool isExpression() const override { return false; }

  bool isLabeled() const { return d_isLabeled; }
  Token label() const { return d_tokLabel; }
  Node::SPtr value() const { return d_pValue; }

  static BreakStatement::SPtr make_shared(
    Token _break,
    bool isLabeled,
    Token label,
    Node::SPtr pValue,
    Node::SPtr pParent = nullptr);
};

} // namespace ast