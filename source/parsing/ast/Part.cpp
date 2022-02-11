#include "parsing/ast/Part.h"

#include <parsing/ast/TypeExpression.h>

using namespace ast;

void Part::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  subNodes->reserve(2);
  if (hasType())
  {
    subNodes->push_back(type());
  }
  if (hasValue())
  {
    subNodes->push_back(value());
  }

  *nodeName = "Part";
  if (auto const pParent =
    std::dynamic_pointer_cast<TypeExpression>(parent().lock()); pParent != nullptr)
  {
    if (pParent->tag() == TypeExpression::Struct)
    {
      *nodeName = "Field";
    }
    else
    {
      *nodeName = "Variant";
    }
  }

  *additionalInfo = fmt::format("'{}'", name());
}

Position Part::end() const
{
  if (d_pValue != nullptr)
  {
    return d_pValue->end();
  }
  return d_pType != nullptr
    ? d_pType->end()
    : d_name.end();
}

Part::SPtr Part::make_shared(
  Token name,
  Node::SPtr pType,
  Node::SPtr pValue,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<Part>(name, pType, pValue, pParent);
  if (pType != nullptr)
  {
    pType->setParent(pRes);
  }
  if (pValue != nullptr)
  {
    pRes->d_pValue->setParent(pRes);
  }
  return pRes;
}