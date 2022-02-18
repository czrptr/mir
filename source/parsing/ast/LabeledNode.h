#pragma once

#include <parsing/ast/Node.h>

#include <string_view>

namespace ast
{

struct LabeledNode : public Node
{
  PTR(LabeledNode)

private:
  bool d_isLabeled;
  Token d_label;

public:
  LabeledNode(Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_isLabeled(false)
  {}

  bool isLabeled() const noexcept { return d_isLabeled; }

  std::string_view labelName() const noexcept { return d_label.text(); }

  Token label() const noexcept { return d_label; }

  void setLabel(Token label) noexcept
  {
    d_isLabeled = true;
    d_label = label;
  }
};

} // namespace ast