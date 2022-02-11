#include "parsing/Parser.h"

#include <Utils.h>

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
      else if (tokSymbol.text()[0] == '@')
      {
        return std::make_shared<BuiltinExpression>(tokSymbol);
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
  Node::SPtr pUnderlyingType = nullptr;

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

    if (tag == TypeExpression::Enum && next(Token::LParen))
    {
      Token const tokLParen = match(Token::LParen);
      pUnderlyingType = expression();
      if (pUnderlyingType == nullptr || !pUnderlyingType->canBeUsedAsExpression())
      {
        // TODO add note: "enum require an underlying type"
        throw Error(d_tokenizer.sourcePath(), tokLParen.start(), tokLParen.end(), Error::Type::Error,
          fmt::format("type expression expected", tag));
      }
      match(Token::RParen, "')' expected");
    }
    // TODO assigne usize if user does not provide an underlying type

    // TODO match with default message '{fmt::to_string}' expected
    match(Token::LBrace, "'{' expected");
  }

  // TODO enum fields

  std::vector<Field::SPtr> fields;
  std::vector<LetStatement::SPtr>
    declsPre, declsPost, *decls = &declsPre;
  bool
    declsSection = false,
    fieldsSection = false;

  // TODO faster implementation with fewer branches
  while (auto pExpr = expressionOrField())
  {
    auto pDecl = std::dynamic_pointer_cast<LetStatement>(pExpr);
    auto pField = std::dynamic_pointer_cast<Field>(pExpr);
    if (pDecl == nullptr && pField == nullptr)
    {
      throw Error(d_tokenizer.sourcePath(), pExpr->start(), pExpr->end(), Error::Type::Error,
        fmt::format("{}s can only contain declarations (let statements), fields or comptime blocks", tag));
    }
    if (pDecl != nullptr)
    {
      // is this a decl after a field?
      declsSection = true && fieldsSection;
      decls->push_back(pDecl);
    }
    else // pField != nullptr
    {
      fieldsSection = true;
      decls = &declsPost;

      if (declsSection)
      {
        // TODO change error based on tag
        //   all variants for eunm and union ?
        throw Error(d_tokenizer.sourcePath(), pExpr->start(), pExpr->end(), Error::Type::Error,
          fmt::format("all fields must be grouped together", tag));
      }

      if (tag == TypeExpression::Enum)
      {
        if (pField->type() != nullptr)
        {
          // TODO + note: all enum variants share the same underlying type
          throw Error(d_tokenizer.sourcePath(), pExpr->start(), pExpr->end(), Error::Type::Error,
            "enum variants cannot have type annotations");
        }
        if (pField->value() == nullptr) // TODO check is this possible?
        {
          throw Error(d_tokenizer.sourcePath(), pExpr->start(), pExpr->end(), Error::Type::Error,
            "enum variants must have values");
        }
      }
      else // tag == TypeExpression::Struct || tag == TypeExpression::Union
      {
        auto const fieldsStr = (tag == TypeExpression::Struct)
          ? "struct fields"
          : "union variants";

        if (pField->type() == nullptr)
        {
          throw Error(d_tokenizer.sourcePath(), pExpr->start(), pExpr->end(), Error::Type::Error,
            fmt::format("{} must have type annotations", fieldsStr));
        }
        if (pField->value() != nullptr)
        {
          throw Error(d_tokenizer.sourcePath(), pExpr->start(), pExpr->end(), Error::Type::Error,
            fmt::format("{} cannot have default values", fieldsStr));
        }
      }

      fields.push_back(pField);
      skip(Token::Comma);
    }
  }

  if (!isRoot)
  {
    end = match(Token::RBrace, "'}' expected").end();
  }
  // TODO end

  return TypeExpression::make_shared(
    tag, start, end, std::move(fields), std::move(declsPre), std::move(declsPost), pUnderlyingType);
}

Field::SPtr Parser::field()
{
  if (!next(Token::Symbol))
  {
    return nullptr;
  }

  setRollbackPoint();
  Token const tokName = match(Token::Symbol);
  Node::SPtr
    pType = nullptr,
    pValue = nullptr;

  if (next(Token::Colon))
  {
    Token const tokColon = match(Token::Colon);
    // TODO implement workaround when a = b expression will be implemented and this code will break
    pType = expression();
    if (pType == nullptr || !pType->canBeUsedAsExpression())
    {
      throw Error(
        d_tokenizer.sourcePath(), tokColon.start(), tokColon.end(),
        Error::Type::Error, "type expression expected");
    }
  }

  if (next(Token::Operator))
  {
    Token const tokColon = match(Token::Operator);
    if (tokColon.text() != "=")
    {
      // TODO test this case !!!
      rollback();
      return nullptr;
    }

    pValue = expression();
    if (pValue == nullptr || !pValue->canBeUsedAsExpression())
    {
      throw Error(
        d_tokenizer.sourcePath(), tokColon.start(), tokColon.end(),
        Error::Type::Error, "expression expected");
    }
  }

  if (pType == nullptr && pValue == nullptr)
  {
    rollback();
    return nullptr;
  }

  commit();
  return Field::make_shared(tokName, pType, pValue);
}

ast::LetStatement::SPtr Parser::letStatement()
{
  auto start = Position::invalid();
  bool
    isPub = false,
    isMut = false;
  Token tokLet;

  if (next(Token::KwPub))
  {
    Token tokPub = match(Token::KwPub);
    isPub = true;
    start = tokPub.start();
    tokLet = match(Token::KwLet, "'let' expected");
  }
  else if (next(Token::KwLet))
  {
    tokLet = match(Token::KwLet);
    start = tokLet.start();
  }
  else
  {
    return nullptr;
  }

  if (next(Token::KwMut))
  {
    Token tokMut = match(Token::KwMut);
    isMut = true;
  }

  // TODO end on trailing comma if present?
  std::vector<LetStatement::Part::SPtr> parts;
  while (auto pPart = letStatementPart())
  {
    parts.push_back(pPart);
    skip(Token::Comma);
  }

  if (parts.empty())
  {
    throw Error(d_tokenizer.sourcePath(), tokLet.start(), tokLet.end(), Error::Type::Error,
      "let statement cannot be empty");
  }

  match(Token::Semicolon, "';' expected");

  return LetStatement::make_shared(start, isPub, isMut, std::move(parts));
}

ast::LetStatementPart::SPtr Parser::letStatementPart()
{
  if (!next(Token::Symbol))
  {
    return nullptr;
  }

  Token const tokSymbol = match(Token::Symbol);
  Position const start = tokSymbol.start();

  // TODO optional type
  Token const tokEq = match(Token::Operator, "assignement operator '=' expected");
  if (tokEq.text() != "=") // TODO better API
  {
    throw Error(d_tokenizer.sourcePath(), tokEq.start(), tokEq.end(), Error::Type::Error, "assignement operator '=' expected");
  }

  auto const pValue = expression();
  if (!pValue)
  {
    // TODO last token instead ok tokEq
    throw Error(d_tokenizer.sourcePath(), tokEq.start(), tokEq.end(), Error::Type::Error, "expression expected");
  }

  return LetStatement::Part::make_shared(start, tokSymbol.text(), pValue);
}

ast::Node::SPtr Parser::expression()
{
  ast::Node::SPtr pRes;
  if (pRes = typeExpression(); pRes != nullptr)
  {
    return pRes;
  }
  else if (pRes = letStatement(); pRes != nullptr)
  {
    return pRes;
  }
  else
  {
    return tokenExpression();
  }
}

ast::Node::SPtr Parser::expressionOrField()
{
  /*
    this fixes: a: b = c,
      being parsed as with two recursive calls to field()
    expression()
    ->field() a:
      ->expression()
        ->field() a = c
  */
  if (auto pRes = field(); pRes != nullptr)
  {
    return pRes;
  }
  return expression();
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