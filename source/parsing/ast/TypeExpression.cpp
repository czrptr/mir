#include "parsing/ast/TypeExpression.h"

using namespace ast;

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
    std::vector<Part::SPtr>&& fields,
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

  for (auto pPart : pRes->d_fields)
  {
    pPart->setParent(pRes);
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