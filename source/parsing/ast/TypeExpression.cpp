#include "parsing/ast/TypeExpression.h"

using namespace ast;

void Field::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());

  // due to parsing logic, hasType() and hasValue()
  // cannot both be true at the same time

  char const* fieldType = nullptr;
  // try AST implementing ast "notes" show info above the node same color ar tree branches
  if (hasType())
  {
    fieldType = "type";
    subNodes->push_back(type());
  }
  if (hasValue()) // d_pValue != nullptr <- idea, IDE deduce & show these types of comments automatically
  {
    fieldType = "value";
    subNodes->push_back(value());
  }

  *nodeName = "Field";

  *additionalInfo = fmt::format("'{}'", name());
  if (fieldType != nullptr)
  {
    *additionalInfo += fmt::format(" {}", fieldType);
  }
}

Position Field::end() const
{
  if (d_pValue != nullptr)
  {
    return d_pValue->end();
  }
  return d_pType != nullptr
    ? d_pType->end()
    : d_name.end();
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

void TypeExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  // TODO: size, aligmnent, wasted bits

  // + 1 for enum underlying type
  subNodes->reserve(fields().size() + decls().size() + 1);
  if (tag() == Tag::Enum && hasUnderlyingType())
  {
    subNodes->push_back(underlyingType());
  }
  subNodes->insert(subNodes->end(), declsPre().begin(), declsPre().end());
  subNodes->insert(subNodes->end(), fields().begin(), fields().end());
  subNodes->insert(subNodes->end(), declsPost().begin(), declsPost().end());

  *nodeName = "TypeLiteral";
  *additionalInfo = fmt::to_string(tag());
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