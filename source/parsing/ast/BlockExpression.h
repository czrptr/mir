#pragma once

#include <parsing/ast/LabeledNode.h>

namespace ast
{

struct BlockExpression final : public LabeledNode
{
  PTR(BlockExpression)

private:
  Position d_start;
  Position d_end;
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
    std::vector<Node::SPtr>&& statements,
    Node::SPtr pParent = nullptr)
    : LabeledNode(pParent)
    , d_start(start)
    , d_end(end)
    , d_statements(std::move(statements))
  {}

  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_end; }

  // TODO check for break statements
  virtual bool isExpression() const override { return true; }

  std::vector<Node::SPtr> const& statements() const { return d_statements; }

  static BlockExpression::SPtr make_shared(
    Position start,
    Position end,
    std::vector<Node::SPtr>&& statements,
    Node::SPtr pParent = nullptr);
};

} // namespace ast