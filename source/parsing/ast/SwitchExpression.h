#pragma once

#include <parsing/ast/Node.h>
#include <parsing/ast/BlockExpression.h>

namespace ast
{

// TODO add support for (parts)
//   multiple values: 1, 2, 3 => ...
//   else =>

struct SwitchExpression final : public Node
{
  PTR(SwitchExpression)

public:
  struct Case
  {
    Node::SPtr value;
    Node::SPtr capture;
    Node::SPtr result;
  };

private:
  Token d_tokSwitch;
  Node::SPtr d_pValue;
  std::vector<Case> d_cases;
  Position d_end;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  SwitchExpression(
    Token tokSwitch,
    Node::SPtr pValue,
    std::vector<Case>&& cases,
    Position end,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_tokSwitch(tokSwitch)
    , d_pValue(pValue)
    , d_cases(std::move(cases))
    , d_end(end)
  {}

  virtual Position start() const override { return d_tokSwitch.start(); }
  virtual Position end() const override { return d_end; }

  // TODO check in sema
  virtual bool isExpression() const override { return true; }

  Node::SPtr value() const { return d_pValue; }
  std::vector<Case> const& cases() { return d_cases; }

  static SwitchExpression::SPtr make_shared(
    Token tokSwitch,
    Node::SPtr pValue,
    std::vector<Case>&& cases,
    Position end,
    Node::SPtr pParent = nullptr);
};

} // namespace ast