#include "parsing/ast/LetStatement.h"

#include <parsing/ast/Utils.h>

using namespace ast;

std::string LetStatementPart::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  std::vector<Node::SPtr> children;
  children.reserve(2);
  if (type() != nullptr)
  {
    children.push_back(type());
  }
  children.push_back(value());

  return fmt::format(
    "{}{} '{}'\n{}",
    prefix(indent, lines, isLast),
    header("LetStatementPart", start(), end(), true),
    name(),
    childrenToString(children, indent, lines));
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

std::string LetStatement::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{}{}{}\n{}",
    prefix(indent, lines, isLast),
    header("LetStatement", start(), end(), true),
    (isPub() ? " pub" : ""),
    (isMut() ? " mut" : ""),
    childrenToString(parts(), indent, lines));
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