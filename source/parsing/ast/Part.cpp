#include "parsing/ast/Part.h"

#include <parsing/ast/TypeExpression.h>
#include <parsing/ast/FunctionExpression.h>

using namespace ast;

void Part::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  subNodes->reserve(3);
  subNodes->push_back(asign());
  if (hasType())
  {
    subNodes->push_back(type());
  }
  if (hasValue())
  {
    subNodes->push_back(value());
  }

  *nodeName = "Part";
  auto const pParent = parent().lock();
  if (pParent->is<TypeExpression>())
  {
    auto const tag = pParent->as<TypeExpression>()->tag();
    if (tag == TypeExpression::Struct)
    {
      *nodeName = "Field";
    }
    else
    {
      *nodeName = "Variant";
    }
  }
  else if (pParent->is<FunctionExpression>())
  {
    *nodeName = "Parameter";
  }

  *additionalInfo = "";
}

Position Part::end() const
{
  if (d_pValue != nullptr)
  {
    return d_pValue->end();
  }
  return d_pType != nullptr
    ? d_pType->end()
    : d_pAsign->end();
}

Part::SPtr Part::make_shared(
  Node::SPtr pAsign,
  Node::SPtr pType,
  Node::SPtr pValue,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<Part>(pAsign, pType, pValue, pParent);
  pAsign->setParent(pRes);
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