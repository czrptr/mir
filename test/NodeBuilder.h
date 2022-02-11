#pragma once

#include <parsing/Intern.h>
#include <parsing/ast/Nodes.h>
#include <initializer_list>

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