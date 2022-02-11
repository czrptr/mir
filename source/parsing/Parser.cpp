#include "parsing/Parser.h"

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
    return std::make_shared<StringExpression>(match(Token::StringLiteral));
  }
  if (next(Token::NumberLiteral))
  {
    return std::make_shared<NumberExpression>(match(Token::StringLiteral));
  }
  return nullptr;
}

ast::TypeExpression::SPtr Parser::typeExpression(bool isRoot)
{
  Position start = { 0, 0 };
  TypeExpression::Tag tag = TypeExpression::Struct;
  Node::SPtr pUnderlyingType = nullptr;

  if (!isRoot)
  {
    if (next(Token::KwStruct))
    {
      start = match(Token::KwStruct).start();
    }
    else if (next(Token::KwEnum))
    {
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
      if (pUnderlyingType == nullptr || !pUnderlyingType->isExpression())
      {
        throw error(tokLParen, fmt::format("type expression expected", tag));
      }
      match(Token::RParen, ErrorStrategy::DefaultErrorMessage);
    }
    match(Token::LBrace, ErrorStrategy::DefaultErrorMessage);
  }

  std::vector<Field::SPtr> fields;
  std::vector<LetStatement::SPtr>
    declsPre, declsPost, *decls = &declsPre;
  bool
    declsSection = false,
    fieldsSection = false;

  while (auto pExpr = expressionOrField())
  {
    auto pDecl = std::dynamic_pointer_cast<LetStatement>(pExpr);
    auto pField = std::dynamic_pointer_cast<Field>(pExpr);
    if (pDecl == nullptr && pField == nullptr)
    {
      throw error(pExpr,
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

      // is this a field after a decl after a field?
      // i.e. are the fields separated by decls
      if (declsSection)
      {
        auto fieldsString = "";
        if (tag == TypeExpression::Struct)
        {
          fieldsString = "struct field";
        }
        else if (tag == TypeExpression::Enum)
        {
          fieldsString = "enum variant";
        }
        else // tag == TypeExpression::Union
        {
          fieldsString = "union variant";
        }
        throw error(pExpr, fmt::format("all {}s must be grouped together", fieldsString));
      }

      if (tag == TypeExpression::Enum)
      {
        if (pField->type() != nullptr)
        {
          throw error(pExpr, "enum variants cannot have type annotations")
            .note("all enum variants share the same underlying type");
        }
      }
      else // tag == TypeExpression::Struct || tag == TypeExpression::Union
      {
        auto const fieldsStr = (tag == TypeExpression::Struct)
          ? "struct fields"
          : "union variants";

        if (pField->type() == nullptr)
        {
          throw error(pExpr, fmt::format("{} must have type annotations", fieldsStr));
        }
        if (pField->value() != nullptr)
        {
          throw error(pExpr, fmt::format("{} cannot have default values", fieldsStr));
        }
      }

      fields.push_back(pField);
      skip(Token::Comma);
    }
  }

  Position end = (!isRoot)
    ? match(Token::RBrace, ErrorStrategy::DefaultErrorMessage).end()
    : lastMatchedToken().end();

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
    if (pType == nullptr || !pType->isExpression())
    {
      throw error(tokColon, "type expression expected");
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
    if (pValue == nullptr || !pValue->isExpression())
    {
      throw error(tokColon, "expression expected");
    }
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
    tokLet = match(Token::KwLet, ErrorStrategy::DefaultErrorMessage);
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
    throw error(tokLet, "let statement cannot be empty");
  }

  match(Token::Semicolon, ErrorStrategy::DefaultErrorMessage);

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

  Node::SPtr pType = nullptr;
  if (next(Token::Colon))
  {
    match(Token::Colon);
    // TODO implement workaround when a = b expression will be implemented and this code will break
    pType = expression();
  }

  // TODO default error messages for operators
  Token const tokEq = match(Token::Operator, "assignement operator '=' expected");
  if (tokEq.text() != "=") // TODO better API
  {
    throw error(tokEq, "assignement operator '=' expected");
  }

  auto const pValue = expression();
  if (!pValue)
  {
    throw error(tokEq, "expression expected");
  }

  return LetStatement::Part::make_shared(start, tokSymbol.text(), pType, pValue);
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
    throw error(currentToken, errorMessage);
  }
  d_currentTokenIdx++;
  return currentToken;
}

Token Parser::match(Token::Tag tag, ErrorStrategy strategy)
{
  auto const errorMessage =
    (strategy == ErrorStrategy::Unreachable)
      ? "reached unreachable code in Parser::match()"
      : fmt::format("{} expected", tag);

  return match(tag, errorMessage);
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

Token Parser::lastMatchedToken()
{
  return d_tokens[std::min(d_currentTokenIdx, d_tokens.size() - 1)];
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
  // TODO commit clears the used tokens from the vector
  if (!d_rollbacks.empty())
  {
    d_rollbacks.pop_back();
  }
}

Error Parser::error(Token token, std::string const& message)
{
  return Error(d_tokenizer.sourcePath(), token.start(), token.end(), message);
}

Error Parser::error(ast::Node::SPtr pNode, std::string const& message)
{
  return Error(d_tokenizer.sourcePath(), pNode->start(), pNode->end(), message);
}