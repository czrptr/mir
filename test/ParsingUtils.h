#pragma once

#include <parsing/Intern.h>
#include <parsing/Tokenizer.h>
#include <parsing/Token.h>

#include <parsing/ast/Nodes.h>

#include <initializer_list>
#include <tuple>

#include <fmt/core.h>

using namespace ast;

template<typename T>
using list = std::initializer_list<T>;

#define TOKENIZER_TEXT(text) \
  auto textStream = std::istringstream((text), std::ios::in); \
  auto tk = Tokenizer(textStream, "<file>")

#define TOKENIZER_FILE(path) \
  auto fileStream = std::ifstream((path), std::ios::in); \
  auto tk = Tokenizer(fileStream, path)

#define PARSER_TEXT(text) \
  auto textStream = std::istringstream((text), std::ios::in); \
  auto prs = Parser(Tokenizer(textStream, "<file>"))

Token t(
  Token::Tag tag,
  size_t startLine, size_t startColumn,
  size_t endLine, size_t endColumn,
  std::string const& text);

Token t(Token::Tag tag, std::string const& text);

SymbolExpression::SPtr symbol(std::string const& name);

BuiltinExpression::SPtr builtin(std::string const& name);

StringExpression::SPtr string(std::string const& text);

NumberExpression::SPtr number(std::string const& value);

BoolExpression::SPtr boolean(bool value);

NullExpression::SPtr null();

UndefinedExpression::SPtr undefined();

UnreachableExpression::SPtr unreachable();

TypeExpression::SPtr _struct(
  list<LetStatement::SPtr> declsPre,
  list<Part::SPtr> fields,
  list<LetStatement::SPtr> declsPost);

TypeExpression::SPtr _enum(
  list<LetStatement::SPtr> declsPre,
  list<Part::SPtr> fields,
  list<LetStatement::SPtr> declsPost);

TypeExpression::SPtr _enum(
  Node::SPtr underlyingType,
  list<LetStatement::SPtr> declsPre,
  list<Part::SPtr> fields,
  list<LetStatement::SPtr> declsPost);

TypeExpression::SPtr _union(
  list<LetStatement::SPtr> declsPre,
  list<Part::SPtr> fields,
  list<LetStatement::SPtr> declsPost);

Part::SPtr part(
  Node::SPtr asign,
  Node::SPtr type,
  Node::SPtr value);

Part::SPtr part(
  Node::SPtr asign,
  Node::SPtr value);

Part::SPtr field(
  std::string const& name,
  Node::SPtr type,
  Node::SPtr value);

Part::SPtr field(
  std::string const& name,
  Node::SPtr value);

LetStatement::SPtr let(
  bool isPub,
  bool isMut,
  list<Part::SPtr> parts);

LetStatement::SPtr let(
  bool isPub,
  bool isMut,
  std::string const& name,
  Node::SPtr value);

LetStatement::SPtr let(
  bool isPub,
  bool isMut,
  std::string const& name,
  Node::SPtr type,
  Node::SPtr value);

BlockExpression::SPtr block(
  list<Node::SPtr> statements);

BlockExpression::SPtr block(
  std::string const& label,
  list<Node::SPtr> statements);

FunctionExpression::SPtr fn(
  list<std::tuple<std::string, Node::SPtr>> parameters,
  Node::SPtr returnType,
  BlockExpression::SPtr body = nullptr);

IfExpression::SPtr _if(
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr block);

IfExpression::SPtr _if(
  std::string const& label,
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr block);

IfExpression::SPtr _if(
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr block,
  list<IfExpression::Clause> clauses);

IfExpression::SPtr _if(
  std::string const& label,
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr block,
  list<IfExpression::Clause> clauses);

IfExpression::Clause _elseIf(
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr block);

IfExpression::Clause _else(
  Node::SPtr capture,
  BlockExpression::SPtr block);

LoopExpression::SPtr loop(
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr body,
  Node::SPtr elseCapture = nullptr,
  BlockExpression::SPtr elseBody = nullptr);

LoopExpression::SPtr loop(
  std::string const& label,
  Node::SPtr condition,
  Node::SPtr capture,
  BlockExpression::SPtr body,
  Node::SPtr elseCapture = nullptr,
  BlockExpression::SPtr elseBody = nullptr);

SwitchExpression::SPtr _switch(
  Node::SPtr value,
  list<SwitchExpression::Case> cases);

bool equal(Node::SPtr node1, Node::SPtr node2);