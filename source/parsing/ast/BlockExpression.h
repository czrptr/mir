#pragma once

#include <parsing/ast/Node.h>

namespace ast
{

struct BlockExpression final : public Node
{
  PTR(BlockExpression)

private:
  Position d_start;
  Position d_end;
  std::string_view d_label;
  std::vector<Node::SPtr> d_statements;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  BlockExpression(
    Position start,
    Position end,
    std::string_view label,
    std::vector<Node::SPtr>&& statements,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_start(start)
    , d_end(end)
    , d_label(label)
    , d_statements(std::move(statements))
  {}

  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_end; }

  // TODO this needs to be determined in sema
  virtual bool isExpression() const override { return true; }

  bool isLabeled() const { return d_label == std::string_view(); }
  std::string_view label() const { return d_label; }
  void setLabel(std::string_view label) { d_label = label; }

  std::vector<Node::SPtr> const& statements() const { return d_statements; }

  static BlockExpression::SPtr make_shared(
    Position start,
    Position end,
    std::string_view label,
    std::vector<Node::SPtr>&& statements,
    Node::SPtr pParent = nullptr);
};

} // namespace ast