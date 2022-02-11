#include "parsing/Parser.h"

#include <Utils.h>

#include <optional>

// TODO better error messages

using namespace ast;

ast::TokenExpression::SPtr Parser::tokenExpression()
{
  if (next(Token::Symbol))
  {
    Token const tokSymbol = match(Token::Symbol);

    // TODO use macro to turn case into value lambda pairs to make the result lazily
    // evaluated so that the untaken "branches" don't cause memory allocations
    return Switch<std::string_view, TokenExpression::SPtr>(tokSymbol.text())
      .Case("true",
        std::make_shared<BoolExpression>(tokSymbol, true))
      .Case("false",
        std::make_shared<BoolExpression>(tokSymbol, false))
      .Case("null",
        std::make_shared<NullExpression>(tokSymbol))
      .Case("undefined",
        std::make_shared<UndefinedExpression>(tokSymbol))
      .Case("unreachable",
        std::make_shared<UnreachableExpression>(tokSymbol))
      .Default(
        std::make_shared<SymbolExpression>(tokSymbol));
  }
  if (next(Token::StringLiteral))
  {
    Token const tokString = match(Token::StringLiteral);
    return std::make_shared<StringExpression>(tokString);
  }
  if (next(Token::NumberLiteral))
  {
    Token const tokNumber = match(Token::NumberLiteral);
    return std::make_shared<NumberExpression>(tokNumber);
  }
  return nullptr;
}

/* ===================== Helpers ===================== */

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

void Parser::setRollbackPoint()
{
  d_rollbacks.push_back(d_currentTokenIdx);
}

void Parser::rollback()
{
  d_currentTokenIdx = d_rollbacks.empty() ? 0 : d_rollbacks.back();
  d_rollbacks.pop_back();
}

void Parser::commit()
{
  // TODO actual commit that clears the vector
  if (!d_rollbacks.empty())
  {
    d_rollbacks.pop_back();
  }
}