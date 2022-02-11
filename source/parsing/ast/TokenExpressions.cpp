#include <parsing/ast/TokenExpressions.h>

#include <parsing/ast/Utils.h>

using namespace ast;

std::string SymbolExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{} '{}'",
    prefix(indent, lines, isLast),
    header("Identifier", start(), end(), false),
    name());
}

std::string StringExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{} {} len {}",
    prefix(indent, lines, isLast),
    header("StringLiteral", start(), end(), false),
    quotedValue(),
    value().length());
}

std::string NumberExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{} {}",
    prefix(indent, lines, isLast),
    header("NumberLiteral", start(), end(), false),
    valueToString());
}

std::string BoolExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{} {}",
    prefix(indent, lines, isLast),
    header("BoolLiteral", start(), end(), false),
    value());
}

std::string NullExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{}",
    prefix(indent, lines, isLast),
    header("Null", start(), end(), false));
}

std::string UndefinedExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{}",
    prefix(indent, lines, isLast),
    header("Undefined", start(), end(), false));
}

std::string UnreachableExpression::toString(size_t indent, std::vector<size_t> lines, bool isLast) const
{
  return fmt::format(
    "{}{}",
    prefix(indent, lines, isLast),
    header("Unreachable", start(), end(), false));
}