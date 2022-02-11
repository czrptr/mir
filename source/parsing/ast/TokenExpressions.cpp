#include <parsing/ast/TokenExpressions.h>

using namespace ast;

void SymbolExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  *nodeName = "Identifier";
  *additionalInfo = fmt::format("'{}'", name());
}

void BuiltinExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  *nodeName = "Builtin";
  *additionalInfo = fmt::format("'{}'", name());
}

void StringExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  *nodeName = "StringLiteral";
  *additionalInfo = fmt::format("{} len {}", quotedValue(), value().length());
}

std::string_view StringExpression::value() const
{
  auto const text = d_token.text();
  return text.substr(1, text.length() - 2);
}

void NumberExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  *nodeName = "NumberLiteral";
  *additionalInfo = valueToString();
}

void BoolExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  *nodeName = "BoolLiteral";
  *additionalInfo = fmt::to_string(value());
}

void NullExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  *nodeName = "Null";
  assert(additionalInfo->empty());
}

void UndefinedExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  *nodeName = "Undefined";
  assert(additionalInfo->empty());
}

void UnreachableExpression::toStringData(
  std::vector<Node::SPtr>* subNodes,
  std::string* nodeName,
  std::string* additionalInfo) const
{
  assert(subNodes->empty());
  *nodeName = "Unreachable";
  assert(additionalInfo->empty());
}