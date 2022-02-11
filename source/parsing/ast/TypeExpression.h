#pragma once

#include <parsing/ast/Node.h>
#include <parsing/Token.h>
#include <parsing/ast/LetStatement.h>

#include <fmt/format.h>

namespace ast
{

struct Field final : public Node
{
  PTR(Field)

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
  Field(Token name, Node::SPtr pType, Node::SPtr pValue, Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_name(name)
    , d_pType(pType)
    , d_pValue(pValue)
  {
    if (pType != nullptr)
    {
      assert(pType->isExpression());
    }
    if (pValue != nullptr)
    {
      assert(pValue->isExpression());
    }
  }

  virtual Position start() const override { return d_name.start(); }
  virtual Position end() const override;
  virtual bool isExpression() const override { return false; }

  std::string_view name() const { return d_name.text(); }
  bool hasType() const { return d_pType != nullptr; }
  Node::SPtr type() const { return d_pType; }
  bool hasValue() const { return d_pValue != nullptr; }
  Node::SPtr value() const { return d_pValue; }

  static Field::SPtr make_shared(Token name, Node::SPtr pType, Node::SPtr pValue, Node::SPtr pParent = nullptr);
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
  std::vector<LetStatement::SPtr> d_declsPre;
  std::vector<LetStatement::SPtr> d_declsPost;
  Node::SPtr d_pUnderlyingType; // in case of enum

protected:
  virtual void toStringData(
    std::vector<Node::SPtr>* subNodes,
    std::string* nodeName,
    std::string* additionalInfo) const override;

public:
  TypeExpression(
    Tag tag,
    Position start,
    Position end,
    std::vector<Field::SPtr>&& fields,
    std::vector<LetStatement::SPtr>&& declsPre,
    std::vector<LetStatement::SPtr>&& declsPost,
    Node::SPtr pUnderlyingType = nullptr,
    Node::SPtr pParent = nullptr)
    : Node(pParent)
    , d_tag(tag)
    , d_start(start)
    , d_end(end)
    , d_fields(std::move(fields))
    , d_declsPre(std::move(declsPre))
    , d_declsPost(std::move(declsPost))
    , d_pUnderlyingType(pUnderlyingType)
  {}

  virtual Position start() const override { return d_start; }
  virtual Position end() const override { return d_end; }
  virtual bool isExpression() const override { return true; }

  Tag tag() const { return d_tag; }
  std::vector<Field::SPtr> const& fields() const { return d_fields; }
  std::vector<LetStatement::SPtr> const& declsPre() const { return d_declsPre; }
  std::vector<LetStatement::SPtr> const& declsPost() const { return d_declsPost; }
  std::vector<LetStatement::SPtr> decls() const;
  bool hasUnderlyingType() const { return d_pUnderlyingType != nullptr; }
  Node::SPtr underlyingType() const { return d_pUnderlyingType; }

  static TypeExpression::SPtr make_shared(
    Tag tag,
    Position start,
    Position end,
    std::vector<Field::SPtr>&& fields,
    std::vector<LetStatement::SPtr>&& declsPre,
    std::vector<LetStatement::SPtr>&& declsPost,
    Node::SPtr pUnderlyingType = nullptr,
    Node::SPtr pParent = nullptr);
};

} // namespace ast

template<>
struct fmt::formatter<ast::TypeExpression::Tag>
{
  bool debug { false };
  fmt::formatter<fmt::string_view> underlying_formatter;

  constexpr auto parse(fmt::format_parse_context& ctx)
  {
    auto
      it = ctx.begin(),
      end = ctx.end();

    if (it != end)
    {
      if (*it == 'd')
      {
        debug = true;
        it += 1;
        ctx.advance_to(it);
        return underlying_formatter.parse(ctx);
      }

      return it;
    }
    return it;
  }

  template<typename FormatContext>
  auto format(ast::TypeExpression::Tag tag, FormatContext& ctx)
  {
    std::string_view name = "TYPE_EXPRESSION_TAG_INVALID";
    if (debug)
    {
      switch (tag)
      {
      case ast::TypeExpression::Tag::Struct: name = "Struct"; break;
      case ast::TypeExpression::Tag::Enum: name = "Enum"; break;
      case ast::TypeExpression::Tag::Union: name = "Union"; break;
      }
      return underlying_formatter.format(name, ctx);
    }

    switch (tag)
    {
    case ast::TypeExpression::Tag::Struct: name = "struct"; break;
    case ast::TypeExpression::Tag::Enum: name = "enum"; break;
    case ast::TypeExpression::Tag::Union: name = "union"; break;
    }
    return underlying_formatter.format(name, ctx);
  }
};
