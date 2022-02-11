#include "parsing/ast/LetStatement.h"

using namespace ast;

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