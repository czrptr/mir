#pragma once

#include <parsing/ast/Node.h>

#include <string_view>

namespace ast
{

struct LabeledNode : public Node
{
  PTR(LabeledNode)

private:
  std::string_view d_label;

public:
  LabeledNode(Node::SPtr pParent = nullptr)
    : Node(pParent)
  {}

  bool isLabeled() const noexcept { return !d_label.empty(); }
  std::string_view label() const noexcept { return d_label; }
  void setLabel(std::string_view label) noexcept { d_label = label; }
};

} // namespace ast