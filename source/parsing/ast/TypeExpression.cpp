#include "parsing/ast/TypeExpression.h"

#include <parsing/ast/Utils.h>

using namespace ast;

std::string Field::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  char const* fieldType;
  std::vector<Node::SPtr> children;
  // try AST implementing ast "notes" show info above the node same color ar tree branches
  if (d_pType != nullptr)
  {
    fieldType = "type";
    children.push_back(d_pType);
  }
  else // d_pValue != nullptr <- idea, IDE deduce &show these types of comments automatically
  {
    fieldType = "value";
    children.push_back(d_pValue);
  }

  return fmt::format(
    "{}{} '{}' {}\n{}",
    prefix(indent, lines, isLast),
    header("Field", start(), end(), true),
    name(),
    fieldType,
    childrenToString(children, indent, lines));
}

Position Field::end() const
{
  return d_pValue != nullptr
    ? d_pValue->end()
    : d_pType->end();
}

Field::SPtr Field::make_shared(Token name, Node::SPtr pType, Node::SPtr pValue, Node::SPtr pParent)
{
  auto pRes = std::make_shared<Field>(name, pType, pValue, pParent);
  if (pType != nullptr)
  {
    pType->setParent(pRes);
  }
  if (pValue != nullptr)
  {
    pValue->setParent(pRes);
  }
  return pRes;
}

std::string TypeExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  // TODO: size, aligmnent, wasted bits

  std::vector<Node::SPtr> members;
  members.reserve(d_fields.size() + d_declsPre.size() + d_declsPost.size() + 1/*enum underlying type*/);
  if (tag() == Tag::Enum && d_pUnderlyingType != nullptr)
  {
    members.push_back(d_pUnderlyingType);
  }
  members.insert(members.end(), d_declsPre.begin(), d_declsPre.end());
  members.insert(members.end(), d_fields.begin(), d_fields.end());
  members.insert(members.end(), d_declsPost.begin(), d_declsPost.end());

  if (members.empty())
  {
    return fmt::format(
      "{}{} {}",
      prefix(indent, lines, isLast),
      header("TypeLiteral", start(), end(), false),
      tag());
  }
  return fmt::format(
    "{}{} {}\n{}",
    prefix(indent, lines, isLast),
    header("TypeLiteral", start(), end(), true),
    tag(),
    childrenToString(members, indent, lines));
}

std::vector<LetStatement::SPtr> TypeExpression::decls() const
{
  std::vector<LetStatement::SPtr> res;
  res.reserve(d_declsPre.size() + d_declsPost.size());
  res.insert(res.end(), d_declsPre.begin(), d_declsPre.end());
  res.insert(res.end(), d_declsPost.begin(), d_declsPost.end());
  return res;
}

TypeExpression::SPtr TypeExpression::make_shared(
    Tag tag,
    Position start,
    Position end,
    std::vector<Field::SPtr>&& fields,
    std::vector<LetStatement::SPtr>&& declsPre,
    std::vector<LetStatement::SPtr>&& declsPost,
    Node::SPtr pUnderlyingType,
    Node::SPtr pParent)
{
  auto pRes = std::make_shared<TypeExpression>(
    tag, start, end,
    std::move(fields),
    std::move(declsPre),
    std::move(declsPost),
    pUnderlyingType,
    pParent);

  for (auto pField : pRes->d_fields)
  {
    pField->setParent(pRes);
  }
  for (auto pDecl : pRes->d_declsPre)
  {
    pDecl->setParent(pRes);
  }
  for (auto pDecl : pRes->d_declsPost)
  {
    pDecl->setParent(pRes);
  }
  if (pUnderlyingType != nullptr)
  {
    pUnderlyingType->setParent(pRes);
  }
  return pRes;
}