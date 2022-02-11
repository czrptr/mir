#pragma once

#include <parsing/ast/Node.h>

namespace ast
{

struct Part final : public Node
{
  PTR(Part)

private:
  Token d_name;
  Node::SPtr d_pType;
  Node::SPtr d_pValue;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  Part(
    Token name,
    Node::SPtr pType,
    Node::SPtr pValue,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_name(name)
    , d_pType(pType)
    , d_pValue(pValue)
  {}

  virtual Position start() const override { return d_name.start(); }
  virtual Position end() const override;
  virtual bool isExpression() const override { return false; }

  std::string_view name() const { return d_name.text(); }
  bool hasType() const { return d_pType != nullptr; }
  Node::SPtr type() const { return d_pType; }
  bool hasValue() const { return d_pValue != nullptr; }
  Node::SPtr value() const { return d_pValue; }

  static Part::SPtr make_shared(
    Token name,
    Node::SPtr pType,
    Node::SPtr pValue,
    Node::SPtr pParent = nullptr);
};

} // namespace ast