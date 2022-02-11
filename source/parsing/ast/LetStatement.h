#pragma once

#include <parsing/ast/Node.h>
#include <parsing/ast/Part.h>
#include <parsing/Token.h>

namespace ast
{

struct LetStatement final : public Node
{
  PTR(LetStatement)

private:
  Position d_start;
  bool d_isPub;
  bool d_isMut;
  std::vector<Part::SPtr> d_parts;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  LetStatement(
    Position start,
    bool isPub,
    bool isMut,
    std::vector<Part::SPtr>&& parts,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_start(start)
    , d_isPub(isPub)
    , d_isMut(isMut)
    , d_parts(std::move(parts))
  {}

  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_parts.back()->end(); }
  virtual bool isExpression() const override { return false; }

  bool isPub() const { return d_isPub; }
  bool isMut() const { return d_isMut; }
  std::vector<Part::SPtr> const& parts() const { return d_parts; }

  static LetStatement::SPtr make_shared(
    Position start,
    bool isPub,
    bool isMut,
    std::vector<Part::SPtr>&& parts,
    Node::SPtr pParent = nullptr);
};

} // namespace ast