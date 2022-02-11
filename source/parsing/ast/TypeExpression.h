#pragma once

#include <parsing/ast/Node.h>
#include <parsing/Token.h>

#include <fmt/format.h>

namespace ast
{

struct Field final : public Node
{
  PTR(Field)

private:
  Token d_name;
  Node::SPtr d_pType;

public:
  Field(Token name, Node::SPtr pType, Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_name(name)
    , d_pType(pType)
  {
    assert(d_pType->canBeUsedAsExpression());
  }

  virtual Position start() const override { return d_name.start(); }
  virtual Position end() const override { return d_pType->end(); }
  virtual bool canBeUsedAsExpression() const override { return false; }

  std::string_view name() const { return d_name.text(); }
  Node::SPtr type() const { return d_pType; }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;

  static Field::SPtr make_shared(Token name, Node::SPtr pType, Node::SPtr pParent = nullptr);
};

struct TypeExpression final : public Node
{
  PTR(TypeExpression)

public:
  enum Tag
  {
    Struct,
    Enum,
    Union,
  };

private:
  Tag d_tag;
  Position d_start;
  Position d_end;
  std::vector<Field::SPtr> d_fields;

public:
  TypeExpression(
    Tag tag,
    Position start,
    Position end,
    std::vector<Field::SPtr>&& fields,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_tag(tag)
    , d_start(start)
    , d_end(end)
    , d_fields(std::move(fields))
  {}

  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_end; }
  virtual bool canBeUsedAsExpression() const override { return true; }

  Tag tag() const { return d_tag; }
  std::vector<Field::SPtr> const& fields() const { return d_fields; }

  using Node::toString;
  virtual std::string toString(size_t indent, std::vector<size_t> lines, bool isLast) const override;

  static TypeExpression::SPtr make_shared(
    Tag tag,
    Position start,
    Position end,
    std::vector<Field::SPtr>&& fields,
    Node::SPtr pParent = nullptr);
};

} // namespace ast

template<>
struct fmt::formatter<ast::TypeExpression::Tag> : fmt::formatter<fmt::string_view>
{
  // TODO {:s} for lower case

  template<typename FormatContext>
  auto format(ast::TypeExpression::Tag tag, FormatContext& ctx)
  {
    std::string_view name = "TYPE_EXPRESSION_TAG_INVALID";
    switch (tag)
    {
    case ast::TypeExpression::Tag::Struct: name = "struct"; break;
    case ast::TypeExpression::Tag::Enum: name = "enum"; break;
    case ast::TypeExpression::Tag::Union: name = "union"; break;
    }
    return fmt::formatter<fmt::string_view>::format(name, ctx);
  }
};
