#pragma once

#include <parsing/Intern.h>
#include <parsing/ast/Nodes.h>
#include <initializer_list>
#include <tuple>

using namespace ast;

template<typename T>
using list = std::initializer_list<T>;

Token t(Token::Tag tag, std::string const& text)
{
  return Token(tag, Position::invalid(), Position::invalid(), Intern::string(text));
}

SymbolExpression::SPtr symbol(std::string const& name)
{
  return std::make_shared<SymbolExpression>(t(Token::Symbol, name));
}

BuiltinExpression::SPtr builtin(std::string const& name)
{
  return std::make_shared<BuiltinExpression>(t(Token::Symbol, '@' + name));
}

StringExpression::SPtr string(std::string const& text)
{
  return std::make_shared<StringExpression>(t(Token::StringLiteral, '"' + text + '"'));
}

NumberExpression::SPtr number(std::string const& value)
{
  return std::make_shared<NumberExpression>(t(Token::NumberLiteral, value));
}

BoolExpression::SPtr boolean(bool value)
{
  return std::make_shared<BoolExpression>(t(Token::Symbol, fmt::to_string(value)), value);
}

NullExpression::SPtr null()
{
  return std::make_shared<NullExpression>(t(Token::Symbol, "null"));
}

UndefinedExpression::SPtr undefined()
{
  return std::make_shared<UndefinedExpression>(t(Token::Symbol, "undefined"));
}

UnreachableExpression::SPtr unreachable()
{
  return std::make_shared<UnreachableExpression>(t(Token::Symbol, "unreachable"));
}

TypeExpression::SPtr _struct(
  list<LetStatement::SPtr> declsPre, list<TypeExpression::Field::SPtr> fields, list<LetStatement::SPtr> declsPost)
{
  return TypeExpression::make_shared(TypeExpression::Struct, Position::invalid(), Position::invalid(),
    fields, declsPre, declsPost);
}

TypeExpression::SPtr _enum(
  list<LetStatement::SPtr> declsPre, list<TypeExpression::Field::SPtr> fields, list<LetStatement::SPtr> declsPost)
{
  return TypeExpression::make_shared(TypeExpression::Enum, Position::invalid(), Position::invalid(),
    fields, declsPre, declsPost);
}

TypeExpression::SPtr _enum(
  Node::SPtr underlyingType, list<LetStatement::SPtr> declsPre, list<TypeExpression::Field::SPtr> fields, list<LetStatement::SPtr> declsPost)
{
  return TypeExpression::make_shared(TypeExpression::Enum, Position::invalid(), Position::invalid(),
    fields, declsPre, declsPost, underlyingType);
}

TypeExpression::SPtr _union(
  list<LetStatement::SPtr> declsPre, list<TypeExpression::Field::SPtr> fields, list<LetStatement::SPtr> declsPost)
{
  return TypeExpression::make_shared(TypeExpression::Union, Position::invalid(), Position::invalid(),
    fields, declsPre, declsPost);
}

Part::SPtr part(
  std::string const& name, Node::SPtr type, Node::SPtr value)
{
  return Part::make_shared(t(Token::Symbol, name), type, value);
}

Part::SPtr part(
  std::string const& name, Node::SPtr value)
{
  return Part::make_shared(t(Token::Symbol, name), nullptr, value);
}

TypeExpression::Field::SPtr field(
  std::string const& name, Node::SPtr type, Node::SPtr value)
{
  return part(name, type, value);
}

TypeExpression::Field::SPtr field(
  std::string const& name, Node::SPtr value)
{
  return part(name, nullptr, value);
}

LetStatement::SPtr let(
  bool isPub, bool isMut, list<Part::SPtr> parts)
{
  return LetStatement::make_shared(Position::invalid(), isPub, isMut, parts);
}

LetStatement::SPtr let(
  bool isPub, bool isMut, std::string const& name, Node::SPtr value)
{
  return LetStatement::make_shared(Position::invalid(), isPub, isMut, {part(name, value)});
}

LetStatement::SPtr let(
  bool isPub, bool isMut, std::string const& name, Node::SPtr type, Node::SPtr value)
{
  return LetStatement::make_shared(Position::invalid(), isPub, isMut, {part(name, type, value)});
}

BlockExpression::SPtr block(list<Node::SPtr> statements)
{
  return BlockExpression::make_shared(Position::invalid(), Position::invalid(), statements);
}

BlockExpression::SPtr block(std::string const& label, list<Node::SPtr> statements)
{
  auto res = BlockExpression::make_shared(Position::invalid(), Position::invalid(), statements);
  res->setLabel(t(Token::Symbol, label));
  return res;
}

FunctionExpression::SPtr fn(
  list<std::tuple<std::string, Node::SPtr>> parameters, Node::SPtr returnType, BlockExpression::SPtr body = nullptr)
{
  std::vector<FunctionExpression::Parameter> params;
  params.reserve(parameters.size());
  for (auto const& param : parameters)
  {
    auto const tokArgName = Token(
      Token::Symbol,
      Position::invalid(), Position::invalid(),
      Intern::string(std::get<0>(param)));

    params.push_back({tokArgName, std::get<1>(param)});
  }

  Token tok(Token::KwFn, Position::invalid(), Position::invalid(), Intern::string("fn"));
  return FunctionExpression::make_shared(tok, std::move(params), returnType, body);
}

IfExpression::SPtr _if(
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr block)
{
  std::vector<IfExpression::Clause> clauses;
  Token tokIf(Token::KwIf, Position::invalid(), Position::invalid(), Intern::string("if"));
  clauses.push_back({IfExpression::Clause::If, tokIf, condition, capture, block});

  return IfExpression::make_shared(std::move(clauses));
}

IfExpression::SPtr _if(
  std::string const& label,
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr block)
{
  auto const pRes = _if(condition, capture, block);
  pRes->setLabel(t(Token::Symbol, label));
  return pRes;
}

IfExpression::SPtr _if(
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr block,
  list<IfExpression::Clause> clauses)
{
  std::vector<IfExpression::Clause> temp;
  temp.reserve(1 + clauses.size());

  temp.push_back({IfExpression::Clause::If, t(Token::KwIf, "if"), condition, capture, block});
  temp.insert(temp.end(), clauses);

  return IfExpression::make_shared(std::move(temp));
}

IfExpression::SPtr _if(
  std::string const& label,
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr block,
  list<IfExpression::Clause> clauses)
{
  auto const pRes = _if(condition, capture, block, clauses);
  pRes->setLabel(t(Token::Symbol, label));
  return pRes;
}

IfExpression::Clause _elseIf(
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr block)
{
  return {IfExpression::Clause::ElseIf, t(Token::KwElse, "else"), condition, capture, block};
}

IfExpression::Clause _else(
  Node::SPtr capture,
  BlockExpression::SPtr block)
{
  return {IfExpression::Clause::Else, t(Token::KwElse, "else"), nullptr, capture, block};
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

  if (nodesAre(Part))
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

  if (nodesAre(BlockExpression))
  {
    if (n1->labelName() != n2->labelName())
      return false;

    if (n1->statements().size() != n2->statements().size())
      return false;

    for (size_t i = 0; i < n1->statements().size(); i += 1)
    {
      if (!equal(n1->statements()[i], n2->statements()[i]))
        return false;
    }
    return true;
  }

  if (nodesAre(FunctionExpression))
  {
    if (!equal(n1->returnType(), n2->returnType()))
      return false;

    if (n1->parameters().size() != n2->parameters().size())
      return false;

    for (size_t i = 0; i < n1->parameters().size(); i += 1)
    {
      auto const
        p1 = n1->parameters()[i],
        p2 = n2->parameters()[i];

      if (p1.name.text() != p2.name.text())
        return false;

      if (!equal(p1.type, p2.type))
        return false;
    }
    return true;
  }

  if (nodesAre(TypeExpression::Field))
  {
    if (n1->name() != n2->name())
      return false;

    if (!equal(n1->type(), n2->type()))
      return false;

    return equal(n1->value(), n2->value());
  }

  if (nodesAre(TypeExpression))
  {
    if (n1->tag() != n2->tag())
      return false;

    if (n1->tag() == TypeExpression::Enum
      && !equal(n1->underlyingType(), n2->underlyingType()))
      return false;

    if (n1->declsPre().size() != n2->declsPre().size())
      return false;

    if (n1->fields().size() != n2->fields().size())
      return false;

    if (n1->declsPost().size() != n2->declsPost().size())
      return false;

    for (size_t i = 0; i < n1->declsPre().size(); i += 1)
    {
      if (!equal(n1->declsPre()[i], n2->declsPre()[i]))
        return false;
    }
    for (size_t i = 0; i < n1->fields().size(); i += 1)
    {
      if (!equal(n1->fields()[i], n2->fields()[i]))
        return false;
    }
    for (size_t i = 0; i < n1->declsPost().size(); i += 1)
    {
      if (!equal(n1->declsPost()[i], n2->declsPost()[i]))
        return false;
    }
    return true;
  }

  if (nodesAre(IfExpression))
  {
    if (n1->labelName() != n2->labelName())
      return false;

    if (n1->clauses().size() != n2->clauses().size())
      return false;

    for (size_t i = 0; i < n1->clauses().size(); i += 1)
    {
      auto const&
        c1 = n1->clauses()[i],
        c2 = n2->clauses()[i];

      if (c1.tag != c2.tag)
        return false;

      if (!equal(c1.condition, c2.condition))
        return false;

      if (!equal(c1.capture, c2.capture))
        return false;

      return equal(c1.block, c2.block);
    }
  }
  return false;
}

#undef nodesAre