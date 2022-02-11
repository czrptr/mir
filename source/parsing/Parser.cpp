#include "parsing/Parser.h"

#include <optional>

// TODO better error messages

namespace
{

std::string viewToString(std::string_view view)
{
  return std::string(view.data(), view.length());
}

} // namespace anonymous

/* ===================== CurrentParsing ===================== */

bool Parser::CurrentParsing::next(Token::Tag tag)
{
  if (d_pParser->d_currentTokenIdx < d_pParser->d_tokens.size())
  {
    return d_pParser->d_tokens[d_pParser->d_currentTokenIdx].tag() == tag;
  }

  Token nextToken = d_pParser->d_tokenizer.next();
  while (nextToken.tag() == Token::Comment) // TODO save these
  {
    nextToken = d_pParser->d_tokenizer.next();
  }

  d_pParser->d_tokens.push_back(nextToken);
  // now d_currentTokenIdx = d_tokens.size() - 1
  return d_pParser->d_tokens[d_pParser->d_currentTokenIdx].tag() == tag;
}

Token Parser::CurrentParsing::match(Token::Tag tag, std::string const& errorMessage)
{
  if (d_pParser->d_currentTokenIdx == d_pParser->d_tokens.size())
  {
    Token nextToken = d_pParser->d_tokenizer.next();
    while (nextToken.tag() == Token::Comment) // TODO save these
    {
      nextToken = d_pParser->d_tokenizer.next();
    }
    d_pParser->d_tokens.push_back(nextToken);
  }

  Token const currentToken = d_pParser->d_tokens[d_pParser->d_currentTokenIdx];
  if (currentToken.tag() != tag)
  {
    throw Error("", currentToken.start(), currentToken.end(), Error::Type::Error, errorMessage);
  }
  d_pParser->d_currentTokenIdx++;
  return currentToken;
}

bool Parser::CurrentParsing::skip(Token::Tag tag)
{
  if (next(tag))
  {
    match(tag);
    return true;
  }
  return false;
}

Parser::CurrentParsing::~CurrentParsing()
{
  if (d_commit)
  {
    for (size_t i = 0; i < d_pParser->d_currentTokenIdx - d_prevTokenIdx; i++)
    {
      d_pParser->d_tokens.pop_back();
    }
  }
  d_pParser->d_currentTokenIdx = d_prevTokenIdx;
}

/* ===================== Nodes ===================== */

using namespace ast;

Root::SPtr Parser::parse()
{
  CurrentParsing currentParsing(this);

  std::vector<Statement::SPtr> statements;
  while (auto pStatement = statement())
  {
    statements.push_back(pStatement);
  }

  Token const Eof = currentParsing.match(Token::Eof, "???");

  return std::make_shared<Root>(Eof.end(), std::move(statements));
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
  CurrentParsing currentParsing(this);

  std::optional<Position> start;
  bool
    isPub = false,
    isMut = false;

  if (currentParsing.next(Token::KwPub))
  {
    Token tokPub = currentParsing.match(Token::KwPub);
    isPub = true;
    start = tokPub.start();
  }

  if (!currentParsing.next(Token::KwLet))
  {
    return nullptr;
  }

  Token tokLet = currentParsing.match(Token::KwLet);
  if (!start)
  {
    start = tokLet.start();
  }


  if (currentParsing.next(Token::KwMut))
  {
    Token TokMut = currentParsing.match(Token::KwMut);
    isMut = true;
  }

  std::vector<LetStatement::Part::SPtr> parts;
  while (auto part = letStatementPart())
  {
    parts.push_back(part);
    currentParsing.skip(Token::Comma);
  }

  currentParsing.match(Token::Semicolon, "';' expected");

  currentParsing.commit();
  return std::make_shared<LetStatement>(*start, currentParsing.end(), isPub, isMut, std::move(parts));
}

LetStatement::Part::SPtr Parser::letStatementPart()
{
  CurrentParsing currentParsing(this);

  std::optional<Position> start;

  if (!currentParsing.next(Token::Symbol))
  {
    return nullptr;
  }

  Token const tokSymbol = currentParsing.match(Token::Symbol);
  if (!start)
  {
    start = tokSymbol.start();
  }

  // TODO optional type

  Token const tokEq = currentParsing.match(Token::Operator, "assignement operator '=' expected");
  if (tokEq.text() != "=") // TODO better API
  {
    throw Error("", tokEq.start(), tokEq.end(), Error::Type::Error, "assignement operator '=' expected");
  }

  auto const pValue = expression();
  if (!pValue)
  {
    // TODO
    throw Error("", Position(0, 0), Position(0, 0), Error::Type::Error, "expression expected");
  }

  currentParsing.commit();
  return std::make_shared<LetStatement::Part>(*start, viewToString(tokSymbol.text()), pValue);
}

Expression::SPtr Parser::expression()
{
  CurrentParsing currentParsing(this);

  if (currentParsing.next(Token::Symbol))
  {
    Token const tokSymbol = currentParsing.match(Token::Symbol);

    currentParsing.commit();
    return std::make_shared<SymbolExpression>(tokSymbol.start(), tokSymbol.end(),
      viewToString(tokSymbol.text()));
  }

  return nullptr;
}