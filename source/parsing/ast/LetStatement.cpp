#include "parsing/ast/LetStatement.h"

using namespace ast;

void LetStatementPart::toStringData(
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
  subNodes->push_back(value());

  *nodeName = "Part";
  *additionalInfo = fmt::format("'{}'", name());
}

LetStatementPart::SPtr LetStatementPart::make_shared(
  Position start,
  std::string_view name,
  Node::SPtr pType,
  Node::SPtr pValue,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<LetStatementPart>(start, name, pType, pValue, pParent);
  if (pType != nullptr)
  {
    pType->setParent(pRes);
  }
  pRes->d_pValue->setParent(pRes);
  return pRes;
}

void LetStatement::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  subNodes->reserve(parts().size());
  for (auto const& pPart : parts())
  {
    subNodes->push_back(pPart);
  }

  *nodeName = "LetStatement";

  *additionalInfo = "";
  if (isPub())
  {
    *additionalInfo += "pub";
  }
  if (isMut())
  {
    auto separator = additionalInfo->empty() ? "" : " ";
    *additionalInfo += fmt::format("{}mut", separator);
  }
}

LetStatement::SPtr LetStatement::make_shared(
  Position start,
  bool isPub,
  bool isMut,
  std::vector<Part::SPtr>&& parts,
  Node::SPtr pParent)
{
  auto pRes = std::make_shared<LetStatement>(start, isMut, isPub, std::move(parts), pParent);
  for (auto pPart : pRes->d_parts)
  {
    pPart->setParent(pRes);
  }
  return pRes;
}