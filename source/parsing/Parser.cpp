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

      if (tokSymbol.text() == "true")
      {
        return std::make_shared<BoolExpression>(tokSymbol, true);
      }
      else if (tokSymbol.text() == "false")
      {
        return std::make_shared<BoolExpression>(tokSymbol, false);
      }
      else if (tokSymbol.text() == "null")
      {
        return std::make_shared<NullExpression>(tokSymbol);
      }
      else if (tokSymbol.text() == "undefined")
      {
        return std::make_shared<UndefinedExpression>(tokSymbol);
      }
      else if (tokSymbol.text() == "unreachable")
      {
        return std::make_shared<UnreachableExpression>(tokSymbol);
      }
      else
      {
        return std::make_shared<SymbolExpression>(tokSymbol);
      }
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

ast::TypeExpression::SPtr Parser::typeExpression(bool isRoot)
{
  Position start = { 0, 0 };
  Position end = Position::invalid();
  TypeExpression::Tag tag = TypeExpression::Struct;
  if (!isRoot)
  {
    if (next(Token::KwStruct))
    {
      start = match(Token::KwStruct).start();
      tag = TypeExpression::Struct;
    }
    else if (next(Token::KwEnum))
    {
      // TODO enum () { ... }
      start = match(Token::KwEnum).start();
      tag = TypeExpression::Enum;
    }
    else if (next(Token::KwUnion))
    {
      start = match(Token::KwUnion).start();
      tag = TypeExpression::Union;
    }
    else
    {
      return nullptr;
    }

    // TODO match with default message '<>' expected
    match(Token::LBrace, "'{' expected");
  }

  std::vector<Field::SPtr> fields;
  while (auto pField = field())
  {
    fields.push_back(pField);
    skip(Token::Comma);
  }

  if (!isRoot)
  {
    end = match(Token::RBrace, "'}' expected").end();
  }
  // TODO end

  return TypeExpression::make_shared(tag, start, end, std::move(fields));
}

Field::SPtr Parser::field()
{
  if (!next(Token::Symbol))
  {
    return nullptr;
  }

  Token const tokName = match(Token::Symbol);
  Token const tokColon = match(Token::Colon, "':' expected");

  auto const pType = expression();
  if (pType == nullptr)
  {
    throw Error(
      d_tokenizer.sourcePath(), tokColon.start(), tokColon.end(),
      Error::Type::Error, "type expression expected");
  }

  return Field::make_shared(tokName, pType);
}

ast::Node::SPtr Parser::expression()
{
  ast::Node::SPtr pRes;
  if (pRes = tokenExpression(); pRes != nullptr)
  {
    return pRes;
  }
  else
  {
    pRes = typeExpression();
    return pRes;
  }
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