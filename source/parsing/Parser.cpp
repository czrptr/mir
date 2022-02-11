#include "parsing/Parser.h"

// TODO better error messages

using namespace ast;

TokenExpression::SPtr Parser::tokenExpression()
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
    return std::make_shared<NumberExpression>(match(Token::NumberLiteral));
  }
  return nullptr;
}

TypeExpression::SPtr Parser::typeExpression(bool isRoot)
{
  using Field = TypeExpression::Field;

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

    // TODO remove required parens: enum(u8) {} -> enum u8 {}
    if (tag == TypeExpression::Enum && next(Token::LParen))
    {
      Token const tokLParen = match(Token::LParen);
      pUnderlyingType = expression();
      throwErrorIfNullOrNotExpression(pUnderlyingType, tokLParen, "type expression expected");

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

  size_t commaCount = 0;
  while (auto pExpr = expressionOrField())
  {
    auto pDecl = std::dynamic_pointer_cast<LetStatement>(pExpr);
    auto pField = std::dynamic_pointer_cast<Field>(pExpr);
    if (pDecl == nullptr && pField == nullptr)
    {
      throw error(pExpr,
        fmt::format("{}s can only contain declarations (let statements), fields and a comptime block", tag));
    }
    if (pDecl != nullptr)
    {
      // is this a decl after a field?
      declsSection = true && fieldsSection;
      decls->push_back(pDecl);
      match(Token::Semicolon, ErrorStrategy::DefaultErrorMessage);
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
        // TODO add note
        throw error(pExpr, fmt::format("{}s must be grouped together", fieldsString));
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

      // force commas between fields
      if (commaCount != fields.size())
      {
        auto const errPos = fields.back()->end();
        throw Error(d_tokenizer.sourcePath(), errPos, errPos, "',' expected");
      }
      fields.push_back(pField);
      commaCount += static_cast<size_t>(skip(Token::Comma));
    }
  }

  Position end = (!isRoot)
    ? match(Token::RBrace, ErrorStrategy::DefaultErrorMessage).end()
    : lastMatchedToken().end();

  return TypeExpression::make_shared(
    tag, start, end, std::move(fields), std::move(declsPre), std::move(declsPost), pUnderlyingType);
}

FunctionExpression::SPtr Parser::functionExpression()
{
  Token tokFn;
  if (!next(Token::KwFn))
  {
    return nullptr;
  }
  tokFn = match(Token::KwFn);

  match(Token::LParen, ErrorStrategy::DefaultErrorMessage);

  std::vector<FunctionExpression::Parameter> parameters;
  while(next(Token::Symbol))
  {
    Token tokName = match(Token::Symbol);

    Token tokColon = match(Token::Colon, ErrorStrategy::DefaultErrorMessage);

    auto pType = expression();
    throwErrorIfNullOrNotExpression(pType, tokColon, "type expression expected");

    parameters.push_back({tokName, pType});
    skip(Token::Comma);
  }

  match(Token::RParen, ErrorStrategy::DefaultErrorMessage);
  auto pReturnType = expression();

  // TODO treat
  //   case: fn (...) label: {}
  //   case: fn (...) <expression> label: {}

  auto pBody = blockExpression();

  return FunctionExpression::make_shared(tokFn, std::move(parameters), pReturnType, pBody);
}

LetStatement::SPtr Parser::letStatement()
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

  if (skip(Token::KwMut))
  {
    isMut = true;
  }

  size_t commaCount = 0;
  std::vector<Part::SPtr> parts;
  while (auto pPart = part())
  {
    if (!pPart->hasValue())
    {
      if (isMut)
      {
        throw error(pPart, "assign 'undefined' to leave variables uninitialzed");
      }
      else
      {
        throw error(pPart, "constants must be initialized");
      }
    }
    if (!isMut && std::dynamic_pointer_cast<UndefinedExpression>(pPart->value()) != nullptr)
    {
      // TODO add note
      throw error(pPart, "constants must be initialized with a proper value");
    }

    // force commas between parts
    if (commaCount != parts.size())
    {
      auto const errPos = parts.back()->end();
      throw Error(d_tokenizer.sourcePath(), errPos, errPos, "',' expected");
    }
    parts.push_back(pPart);
    commaCount += static_cast<size_t>(skip(Token::Comma));
  }

  if (parts.empty())
  {
    throw error(tokLet, "let statement cannot be empty");
  }

  return LetStatement::make_shared(start, isPub, isMut, std::move(parts));
}

Part::SPtr Parser::part()
{
  if (!next(Token::Symbol))
  {
    return nullptr;
  }

  // setRollbackPoint();
  Token const tokSymbol = match(Token::Symbol);

  // TODO implement workaround when a = b expression will be implemented and this code will break

  Node::SPtr pType = nullptr;
  if (skip(Token::Colon))
  {
    pType = expression();
    throwErrorIfNotExpression(pType, "type expression expected");
  }

  Node::SPtr pValue = nullptr;
  if (next(Token::Operator))
  {
    Token const tokEq = match(Token::Operator, "assignement operator '=' expected");
    if (tokEq.text() != "=")
    {
      // rollback();
      // return nullptr;
      throw error(tokEq, "assignement operator '=' expected");
    }
    pValue = expression();
    throwErrorIfNullOrNotExpression(pValue, tokEq, "expression expected");
  }

  // commit();
  return Part::make_shared(tokSymbol, pType, pValue);
}

BlockExpression::SPtr Parser::blockExpression()
{
  Token tokLBrace;
  if (!next(Token::LBrace))
  {
    return nullptr;
  }
  tokLBrace = match(Token::LBrace);

  std::vector<Node::SPtr> statements;
  while (auto pStmt = expression())
  {
    statements.push_back(pStmt);
    while (skip(Token::Semicolon));
  }

  Token const tokRBrace = match(Token::RBrace, ErrorStrategy::DefaultErrorMessage);

  return BlockExpression::make_shared(
    tokLBrace.start(), tokRBrace.start(), std::string_view(), std::move(statements));
}

Node::SPtr Parser::expression()
{
  // TODO parse comptime here
  // add marked comptime field to Node?

  bool isLabeled = false;
  Token toklabel;

  setRollbackPoint();
  if (next(Token::Symbol))
  {
    toklabel = match(Token::Symbol);
    if (next(Token::Colon))
    {
      Token tokColon = match(Token::Colon);
      if (toklabel.end() != tokColon.start())
      {
        // marking labels must be symbols suffixed with ':'
        throw error(tokColon, fmt::format("did you mean '{}:'", toklabel.text()));
      }
      isLabeled = true;
      commit();
    }
    else
    {
      rollback();
    }
  }

  Node::SPtr pRes = nullptr;
  if (auto pTemp = typeExpression(); pTemp != nullptr)
  {
    pRes = pTemp;
  }
  else if (auto pTemp = functionExpression(); pTemp != nullptr)
  {
    pRes = pTemp;
  }
  else if (auto pTemp = letStatement(); pTemp != nullptr)
  {
    pRes = pTemp;
  }
  else if (auto pTemp = blockExpression(); pTemp != nullptr)
  {
    pRes = pTemp;
  }
  else
  {
    pRes = tokenExpression();
  }

  // TODO add if, loop check
  auto pBlock = std::dynamic_pointer_cast<BlockExpression>(pRes);

  if (pBlock == nullptr && isLabeled)
  {
    throw error(toklabel, "only block, ifs and loops can be labeled");
  }
  if (pBlock != nullptr)
  {
    pBlock->setLabel(toklabel.text());
  }
  return pRes;
}

Node::SPtr Parser::expressionOrField()
{
  /*
    this fixes: a: b = c,
      being parsed with two recursive calls to part()
    expression()
    ->part() a:
      ->expression()
        ->part() a = c
  */
  if (auto pRes = part(); pRes != nullptr)
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

Error Parser::error(Node::SPtr pNode, std::string const& message)
{
  return Error(d_tokenizer.sourcePath(), pNode->start(), pNode->end(), message);
}

void Parser::throwErrorIfNotExpression(Node::SPtr pNode, std::string const& message)
{
  if (pNode != nullptr && !pNode->isExpression())
  {
    throw error(pNode, message);
  }
}

void Parser::throwErrorIfNullOrNotExpression(Node::SPtr pNode, Token fallbackToken, std::string const& message)
{
  if (pNode == nullptr)
  {
    throw Error(d_tokenizer.sourcePath(), fallbackToken.end(), fallbackToken.end(), message);
  }
  if (!pNode->isExpression())
  {
    throw error(pNode, message);
  }
}