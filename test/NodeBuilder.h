#pragma once

#include <parsing/Intern.h>
#include <parsing/ast/Nodes.h>
#include <initializer_list>
#include <tuple>

using namespace ast;

template<typename T>
using list = std::initializer_list<T>;

SymbolExpression::SPtr symbol(std::string const& name)
{
  Token tok(Token::Symbol, Position::invalid(), Position::invalid(), Intern::string(name));
  return std::make_shared<SymbolExpression>(tok);
}

BuiltinExpression::SPtr builtin(std::string const& name)
{
  Token tok(Token::Symbol, Position::invalid(), Position::invalid(), Intern::string('@' + name));
  return std::make_shared<BuiltinExpression>(tok);
}

StringExpression::SPtr string(std::string const& text)
{
  Token tok(Token::StringLiteral, Position::invalid(), Position::invalid(), Intern::string('"' + text + '"'));
  return std::make_shared<StringExpression>(tok);
}

NumberExpression::SPtr number(std::string const& value)
{
  Token tok(Token::NumberLiteral, Position::invalid(), Position::invalid(), Intern::string(value));
  return std::make_shared<NumberExpression>(tok);
}

BoolExpression::SPtr boolean(bool value)
{
  Token tok(Token::Symbol, Position::invalid(), Position::invalid(), Intern::string(fmt::to_string(value)));
  return std::make_shared<BoolExpression>(tok, value);
}

NullExpression::SPtr null()
{
  Token tok(Token::Symbol, Position::invalid(), Position::invalid(), Intern::string("null"));
  return std::make_shared<NullExpression>(tok);
}

UndefinedExpression::SPtr undefined()
{
  Token tok(Token::Symbol, Position::invalid(), Position::invalid(), Intern::string("undefined"));
  return std::make_shared<UndefinedExpression>(tok);
}

UnreachableExpression::SPtr unreachable()
{
  Token tok(Token::Symbol, Position::invalid(), Position::invalid(), Intern::string("unreachable"));
  return std::make_shared<UnreachableExpression>(tok);
}

Field::SPtr field(std::string const& name, Node::SPtr type, Node::SPtr value)
{
  Token tok(Token::Symbol, Position::invalid(), Position::invalid(), Intern::string(name));
  return Field::make_shared(tok, type, value);
}

TypeExpression::SPtr _struct(
  list<LetStatement::SPtr> declsPre, list<Field::SPtr> fields, list<LetStatement::SPtr> declsPost)
{
  return TypeExpression::make_shared(TypeExpression::Struct, Position::invalid(), Position::invalid(),
    fields, declsPre, declsPost);
}

TypeExpression::SPtr _enum(
  list<LetStatement::SPtr> declsPre, list<Field::SPtr> fields, list<LetStatement::SPtr> declsPost)
{
  return TypeExpression::make_shared(TypeExpression::Enum, Position::invalid(), Position::invalid(),
    fields, declsPre, declsPost);
}

TypeExpression::SPtr _enum(
  Node::SPtr underlyingType, list<LetStatement::SPtr> declsPre, list<Field::SPtr> fields, list<LetStatement::SPtr> declsPost)
{
  return TypeExpression::make_shared(TypeExpression::Enum, Position::invalid(), Position::invalid(),
    fields, declsPre, declsPost, underlyingType);
}

TypeExpression::SPtr _union(
  list<LetStatement::SPtr> declsPre, list<Field::SPtr> fields, list<LetStatement::SPtr> declsPost)
{
  return TypeExpression::make_shared(TypeExpression::Union, Position::invalid(), Position::invalid(),
    fields, declsPre, declsPost);
}

LetStatementPart::SPtr part(
  std::string const& name, Node::SPtr type, Node::SPtr value)
{
  return LetStatementPart::make_shared(Position::invalid(), Intern::string(name), type, value);
}

LetStatement::SPtr let(
  bool isPub, bool isMut, list<LetStatementPart::SPtr> parts)
{
  return LetStatement::make_shared(Position::invalid(), isPub, isMut, parts);
}

FunctionExpression::SPtr fn(
  list<FunctionExpression::Parameter> parameters, Node::SPtr returnType, BlockExpression::SPtr body = nullptr)
{
  Token tok(Token::KwFn, Position::invalid(), Position::invalid(), Intern::string("fn"));
  return FunctionExpression::make_shared(tok, parameters, returnType, body);
}

BlockExpression::SPtr block(list<Node::SPtr> statements)
{
  return BlockExpression::make_shared(Position::invalid(), Position::invalid(), std::string_view(), statements);
}

BlockExpression::SPtr block(std::string const& label, list<Node::SPtr> statements)
{
  return BlockExpression::make_shared(Position::invalid(), Position::invalid(), Intern::string(label), statements);
}

template<typename T>
std::tuple<std::shared_ptr<T>, std::shared_ptr<T>, bool> nodesAreImpl(
  Node::SPtr node1, Node::SPtr node2)
{
  std::shared_ptr<T>
    node1asT = std::dynamic_pointer_cast<T>(node1),
    node2asT = std::dynamic_pointer_cast<T>(node2);

  return {node1asT, node2asT, (node1asT != nullptr && node2asT != nullptr)};
}

#define nodesAre(type) auto const [n1, n2, cond] = nodesAreImpl<type>(node1, node2); cond

bool equal(Node::SPtr node1, Node::SPtr node2)
{
  if (node1 == nullptr && node2 == nullptr)
    return true;

  if (nodesAre(SymbolExpression))
    return n1->name() == n2->name();

  if (nodesAre(BuiltinExpression))
    return n1->name() == n2->name();

  if (nodesAre(StringExpression))
    return n1->quotedValue() == n2->quotedValue();

  if (nodesAre(NumberExpression))
    return n1->valueToString() == n2->valueToString();

  if (nodesAre(BoolExpression))
    return n1->value() == n2->value();

  if (nodesAre(NullExpression))
    return true;

  if (nodesAre(UndefinedExpression))
    return true;

  if (nodesAre(UnreachableExpression))
    return true;

  if (nodesAre(LetStatement::Part))
  {
    if (n1->name() != n2->name())
      return false;

    if (!equal(n1->type(), n2->type()))
      return false;

    return equal(n1->value(), n2->value());
  }

  if (nodesAre(LetStatement))
  {
    if (n1->isPub() != n2->isPub())
      return false;

    if (n1->isMut() != n2->isMut())
      return false;

    if (n1->parts().size() != n2->parts().size())
      return false;

    for (size_t i = 0; i < n1->parts().size(); i += 1)
    {
      if (!equal(n1->parts()[i], n2->parts()[i]))
        return false;
    }
    return true;
  }

  return false;
}