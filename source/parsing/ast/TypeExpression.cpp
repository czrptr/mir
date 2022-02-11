#include "parsing/ast/TypeExpression.h"

#include <parsing/ast/Utils.h>

using namespace ast;

std::string Field::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{} '{}'\n{}",
    prefix(indent, lines, isLast),
    header("Field", start(), end(), false),
    name(),
    childrenToString(std::vector<decltype(Field::d_pType)>{type()}, indent, lines));
}

Field::SPtr Field::make_shared(Token name, Node::SPtr pType, Node::SPtr pParent)
{
  auto pRes = std::make_shared<Field>(name, pType, pParent);
  pType->setParent(pRes);
  return pRes;
}

std::string TypeExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  // TODO: size, aligmnent, wasted bits

  if (fields().empty())
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
    header("TypeLiteral", start(), end(), false),
    tag(),
    childrenToString(fields(), indent, lines));
}

TypeExpression::SPtr TypeExpression::make_shared(
    Tag tag,
    Position start,
    Position end,
    std::vector<Field::SPtr>&& fields,
    Node::SPtr pParent)
{
  auto pRes = std::make_shared<TypeExpression>(tag, start, end, std::move(fields), pParent);
  for (auto pField : pRes->d_fields)
  {
    pField->setParent(pRes);
  }
  return pRes;
}