#pragma once

#include <parsing/ast/Node.h>

namespace ast
{

struct Part final : public Node
{
  PTR(Part)

private:
  Node::SPtr d_pAsign;
  Node::SPtr d_pType;
  Node::SPtr d_pValue;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  Part(
    Node::SPtr pAsign,
    Node::SPtr pType,
    Node::SPtr pValue,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_pAsign(pAsign)
    , d_pType(pType)
    , d_pValue(pValue)
  {}

  virtual Position start() const override { return d_pAsign->start(); }
  virtual Position end() const override;
  virtual bool isExpression() const override { return false; }

  Node::SPtr asign() const { return d_pAsign; }
  bool hasType() const { return d_pType != nullptr; }
  Node::SPtr type() const { return d_pType; }
  bool hasValue() const { return d_pValue != nullptr; }
  Node::SPtr value() const { return d_pValue; }

  static Part::SPtr make_shared(
    Node::SPtr pAsign,
    Node::SPtr pType,
    Node::SPtr pValue,
    Node::SPtr pParent = nullptr);
};

} // namespace ast