#include "parsing/Parser.h"

#include <Utils.h>

#include <optional>

// TODO better error messages

namespace
{

} // namespace anonymous

using namespace ast;

Root::SPtr Parser::parse()
{
  std::vector<Statement::SPtr> statements;
  while (auto pStatement = statement())
  {
    statements.push_back(pStatement);
  }

  Token const Eof = match(Token::Eof, "???");

  return Root::make_shared(Eof.end(), std::move(statements));
}

Statement::SPtr Parser::statement()
{
  if (auto pLetStatement = letStatement())
  {
    return pLetStatement;
  }
  return nullptr;
}

LetStatement::SPtr Parser::letStatement()
{
  std::optional<Position> start;
  bool
    isPub = false,
    isMut = false;

  if (next(Token::KwPub))
  {
    Token tokPub = match(Token::KwPub);
    isPub = true;
    start = tokPub.start();
  }

  if (!next(Token::KwLet))
  {
    return nullptr;
  }

  Token tokLet = match(Token::KwLet);
  if (!start)
  {
    start = tokLet.start();
  }

  if (next(Token::KwMut))
  {
    Token tokMut = match(Token::KwMut);
    isMut = true;
  }

  std::vector<LetStatement::Part::SPtr> parts;
  while (auto pPart = letStatementPart())
  {
    parts.push_back(pPart);
    skip(Token::Comma);
  }

  if (parts.empty())
  {
    // TODO tokLet or tokMut
    throw Error(d_tokenizer.sourcePath(), tokLet.start(), tokLet.end(), Error::Type::Error,
      "let statement cannot be empty");
  }

  Token const ender = match(Token::Semicolon, "';' expected");

  // commit();
  return LetStatement::make_shared(*start, ender.end(), isPub, isMut, std::move(parts));
}

LetStatement::Part::SPtr Parser::letStatementPart()
{
  std::optional<Position> start;

  if (!next(Token::Symbol))
  {
    return nullptr;
  }

  Token const tokSymbol = match(Token::Symbol);
  if (!start)
  {
    start = tokSymbol.start();
  }

  // TODO optional type

  Token const tokEq = match(Token::Operator, "assignement operator '=' expected");
  if (tokEq.text() != "=") // TODO better API
  {
    throw Error("", tokEq.start(), tokEq.end(), Error::Type::Error, "assignement operator '=' expected");
  }

  auto const pValue = atomicExpression();
  if (!pValue)
  {
    // TODO last token instead ok tokEq
    throw Error(d_tokenizer.sourcePath(), tokEq.start(), tokEq.end(), Error::Type::Error, "expression expected");
  }

  return LetStatement::Part::make_shared(*start, tokSymbol.text(), pValue);
}

Expression::SPtr Parser::atomicExpression()
{
  if (next(Token::Symbol))
  {
    Token const tokSymbol = match(Token::Symbol);

    return Switch<std::string_view, Expression::SPtr>(tokSymbol.text())
      .Case("true",
        std::make_shared<BoolExpression>(tokSymbol.start(), tokSymbol.end(), true))
      .Case("false",
        std::make_shared<BoolExpression>(tokSymbol.start(), tokSymbol.end(), false))
      .Default(
        std::make_shared<SymbolExpression>(tokSymbol.start(), tokSymbol.end(), tokSymbol.text()));
  }

  if (next(Token::StringLiteral))
  {
    Token const tokString = match(Token::StringLiteral);
    return std::make_shared<StringExpression>(tokString.start(), tokString.end(), tokString.text());
  }

  return nullptr;
}

bool Parser::next(Token::Tag tag)
{
  if (d_currentTokenIdx < d_tokens.size())
  {
    return d_tokens[d_currentTokenIdx].tag() == tag;
  }

  Token nextToken = d_tokenizer.next();
  while (nextToken.tag() == Token::Comment) // TODO save these
  {
    nextToken = d_tokenizer.next();
  }

  d_tokens.push_back(nextToken);
  // now d_currentTokenIdx = d_tokens.size() - 1
  return d_tokens[d_currentTokenIdx].tag() == tag;
}

Token Parser::match(Token::Tag tag, std::string const& errorMessage)
{
  if (d_currentTokenIdx == d_tokens.size())
  {
    Token nextToken = d_tokenizer.next();
    while (nextToken.tag() == Token::Comment) // TODO save these
    {
      nextToken = d_tokenizer.next();
    }
    d_tokens.push_back(nextToken);
  }

  Token const currentToken = d_tokens[d_currentTokenIdx];
  if (currentToken.tag() != tag)
  {
    throw Error(d_tokenizer.sourcePath(), currentToken.start(), currentToken.end(), Error::Type::Error, errorMessage);
  }
  d_currentTokenIdx++;
  return currentToken;
}

bool Parser::skip(Token::Tag tag)
{
  if (next(tag))
  {
    match(tag);
    return true;
  }
  return false;
}

void Parser::commit()
{
  d_tokens.clear();
  d_currentTokenIdx = 0;
}