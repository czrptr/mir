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

  auto const pValue = expression();
  if (!pValue)
  {
    // TODO last token instead ok tokEq
    throw Error(d_tokenizer.sourcePath(), tokEq.start(), tokEq.end(), Error::Type::Error, "expression expected");
  }

  return LetStatement::Part::make_shared(*start, tokSymbol.text(), pValue);
}

/* ===================== Expressions ===================== */

Expression::SPtr Parser::atomicExpression()
{
  if (next(Token::Symbol))
  {
    Token const tokSymbol = match(Token::Symbol);

    if (tokSymbol.text() == "true")
    {
      return std::make_shared<BoolExpression>(tokSymbol.start(), tokSymbol.end(), true);
    }
    else if (tokSymbol.text() == "false")
    {
      return std::make_shared<BoolExpression>(tokSymbol.start(), tokSymbol.end(), false);
    }
    else if (tokSymbol.text() == "null")
    {
      return std::make_shared<NullExpression>(tokSymbol.start(), tokSymbol.end());
    }
        else if (tokSymbol.text() == "undefined")
    {
      return std::make_shared<UndefinedExpression>(tokSymbol.start(), tokSymbol.end());
    }
        else if (tokSymbol.text() == "unreachable")
    {
      return std::make_shared<UnreachableExpression>(tokSymbol.start(), tokSymbol.end());
    }

    return std::make_shared<SymbolExpression>(tokSymbol.start(), tokSymbol.end(), tokSymbol.text());

    // TODO use macro to turn case into value lambda pairs to make the result lazily
    // evaluated so that the untaken "branches" don't cause memory allocations
    // return Switch<std::string_view, Expression::SPtr>(tokSymbol.text())
    //   .Case("true",
    //     std::make_shared<BoolExpression>(tokSymbol.start(), tokSymbol.end(), true))
    //   .Case("false",
    //     std::make_shared<BoolExpression>(tokSymbol.start(), tokSymbol.end(), false))
    //   .Default(
    //     std::make_shared<SymbolExpression>(tokSymbol.start(), tokSymbol.end(), tokSymbol.text()));
  }

  if (next(Token::StringLiteral))
  {
    Token const tokString = match(Token::StringLiteral);
    return std::make_shared<StringExpression>(tokString.start(), tokString.end(), tokString.text());
  }

  return nullptr;
}

ast::Expression::SPtr Parser::binaryExpression(
  ExpressionParser nextParser,
  std::vector<Operator::Tag> operators)
{
  auto pLhs = (this->*nextParser)();
  if (pLhs == nullptr)
  {
    return nullptr;
  }

  Expression::SPtr pRes = pLhs;
  while (next(Token::Operator))
  {
    setRollbackPoint();
    auto op = Operator(match(Token::Operator));

    auto const tagIt = std::find_if(operators.begin(), operators.end(),
      [&op](Operator::Tag const& tag)
      {
        return op.text() == fmt::to_string(tag);
      });

    if (tagIt == operators.end())
    {
      rollback();
      return pRes;
    }
    else
    {
      op.setTag(*tagIt);
    }

    auto pRhs = (this->*nextParser)();
    if (pRhs == nullptr)
    {
      // TODO better positions
      throw Error(d_tokenizer.sourcePath(), op.start(), op.end(), Error::Type::Error, "expression expected");
    }

    commit();
    pRes = BinaryExpression::make_shared(pRes, pRhs, op);
  }

  return pRes;
}

Expression::SPtr Parser::expr00()
{
  auto pLhs = expr01();
  if (pLhs == nullptr)
  {
    return nullptr;
  }

  Expression::SPtr pRes = pLhs;
  size_t count = 0;
  while (next(Token::Operator))
  {
    setRollbackPoint();
    auto op = Operator(match(Token::Operator));
    if (op.text() != fmt::to_string(Operator::DotDot))
    {
      rollback();
      return pRes;
    }
    else
    {
      op.setTag(Operator::DotDot);
    }

    count += 1;
    if (count > 1)
    {
      throw Error(d_tokenizer.sourcePath(), op.start(), op.end(), Error::Type::Error, "operator '..' cannot be chained");
    }

    auto pRhs = expr01();
    if (pRhs == nullptr)
    {
      // TODO better positions
      throw Error(d_tokenizer.sourcePath(), op.start(), op.end(), Error::Type::Error, "expression expected");
    }

    commit();
    pRes = BinaryExpression::make_shared(pRes, pRhs, op);
  }

  return pRes;
}

Expression::SPtr Parser::expr01()
{
  return binaryExpression(&Parser::expr02, {Operator::OrOr});
}

Expression::SPtr Parser::expr02()
{
  return binaryExpression(&Parser::atomicExpression, {Operator::AndAnd});
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