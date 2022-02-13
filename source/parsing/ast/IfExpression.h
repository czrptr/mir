#pragma once

#include <parsing/ast/Node.h>
#include <parsing/ast/BlockExpression.h>

namespace ast
{

struct IfExpression final : public Node
{
  PTR(IfExpression)

public:
  struct Clause
  {
    enum Tag
    {
      If,
      ElseIf,
      Else,
    };

    Tag tag;
    Token tokStart;
    Node::SPtr condition;
    Node::SPtr capture;
    BlockExpression::SPtr block;
  };

private:
  std::vector<Clause> d_clauses;

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  IfExpression(
    std::vector<Clause>&& clauses,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_clauses(std::move(clauses))
  {
    assert(d_clauses.size() > 0);
  }

  virtual Position start() const override;
  virtual Position end() const override;

  // TODO check for break statements
  virtual bool isExpression() const override { return true; }

  std::vector<Clause> const& clauses() const { return d_clauses; }

  static IfExpression::SPtr make_shared(
    std::vector<Clause>&& clauses,
    Node::SPtr pParent = nullptr);
};

} // namespace ast

template<>
struct fmt::formatter<ast::IfExpression::Clause::Tag> : public fmt::formatter<fmt::string_view>
{
  template<typename FormatContext>
  auto format(ast::IfExpression::Clause::Tag tag, FormatContext& ctx)
  {
    std::string_view name = "IF_EXPRESSION_CLAUSE_TAG_INVALID";

    switch (tag)
    {
    case ast::IfExpression::Clause::Tag::If: name = "if"; break;
    case ast::IfExpression::Clause::Tag::ElseIf: name = "else if"; break;
    case ast::IfExpression::Clause::Tag::Else: name = "else"; break;
    }
    return fmt::formatter<fmt::string_view>::format(name, ctx);
  }
};