#include "parsing/Parser.h"

// TODO better error messages
// TODO? error on "[else] if ()", "switch ()", "loop ()", "enum ()"

using namespace ast;

namespace Runes
{

static constexpr Operator::Tag Bar = Operator::BitOr;

}

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

  Token tokTag { Token::KwStruct, Position(0, 0), Position(0, 0), "" };
  TypeExpression::Tag tag = TypeExpression::Struct;
  Node::SPtr pUnderlyingType = nullptr;

  if (!isRoot)
  {
    if (next(Token::KwStruct))
    {
      tokTag = match(Token::KwStruct);
    }
    else if (next(Token::KwEnum))
    {
      tokTag = match(Token::KwEnum);
      tag = TypeExpression::Enum;
    }
    else if (next(Token::KwUnion))
    {
      tokTag = match(Token::KwUnion);
      tag = TypeExpression::Union;
    }
    else
    {
      return nullptr;
    }

    // TODO error for struct and variant: "{}s don't have underlying types"
    if (tag == TypeExpression::Enum && !next(Token::LBrace))
    {
      pUnderlyingType = expression();
      // TODO error pos need to be after tokTag, not on it
      throwErrorIfNullOrNotExpression(pUnderlyingType, tokTag, "type expression expected");
    }
    match(Token::LBrace, ErrorStrategy::DefaultErrorMessage);
  }

  std::vector<Field::SPtr> fields;
  std::vector<LetStatement::SPtr>
    declsPre, declsPost, *decls = &declsPre;

  size_t commaCount = 0;
  while (auto pExpr = expressionOrField())
  {
    if (pExpr->is<LetStatement>())
    {
      decls->push_back(pExpr->as<LetStatement>());
      match(Token::Semicolon, ErrorStrategy::DefaultErrorMessage);
    }
    else if (pExpr->is<Field>())
    {
      auto pField = pExpr->as<Field>();
      decls = &declsPost;

      if (!fields.empty() && !declsPost.empty())
      {
        throw error(pField, fmt::format("{:field}s must be grouped together", tag));
      }

      if (tag == TypeExpression::Enum)
      {
        if (pField->hasType())
        {
          throw error(pField, "enum variants cannot have type annotations")
            .note("all enum variants share the same underlying type");
        }
      }
      else // struct or union
      {
        if (!pField->hasType())
        {
          throw error(pField, fmt::format("{:field}s must have type annotations", tag));
        }
        if (pField->hasValue())
        {
          throw error(pField, fmt::format("{:field}s cannot have default values", tag));
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
    else
    {
      throw error(pExpr,
        fmt::format("{}s can only contain declarations (let statements), fields and a comptime block", tag));
    }
  }

  Position end = (!isRoot)
    ? match(Token::RBrace, ErrorStrategy::DefaultErrorMessage).end()
    : lastMatchedToken().end();

  return TypeExpression::make_shared(
    tag, tokTag.start(), end, std::move(fields), std::move(declsPre), std::move(declsPost), pUnderlyingType);
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

  size_t commaCount = 0;
  std::vector<FunctionExpression::Parameter> parameters;
  while(next(Token::Symbol))
  {
    Token tokName = match(Token::Symbol);

    Token tokColon = match(Token::Colon, ErrorStrategy::DefaultErrorMessage);

    auto pType = expression();
    throwErrorIfNullOrNotExpression(pType, tokColon, "type expression expected");

    // force commas between parameters
    if (commaCount != parameters.size())
    {
      auto const errPos = parameters.back().type->end();
      throw Error(d_tokenizer.sourcePath(), errPos, errPos, "',' expected");
    }
    parameters.push_back({tokName, pType});
    commaCount += static_cast<size_t>(skip(Token::Comma));
  }

  match(Token::RParen, ErrorStrategy::DefaultErrorMessage);

  pushState(FunctionReturnType);
  auto pReturnType = expression();
  assert(popState() == FunctionReturnType);

  // TODO expression<>(errorMessage)
  auto pBody = expression();
  auto pBlock = std::dynamic_pointer_cast<BlockExpression>(pBody);

  if (pBody != nullptr)
  {
    if (!pBody->is<BlockExpression>())
    {
      throw error(pBody, "block expected");
    }
    if (pBody->as<BlockExpression>()->isLabeled())
    {
      throw error(pBlock, "function blocks cannot be labeled");
    }
  }
  return FunctionExpression::make_shared(tokFn, std::move(parameters), pReturnType, pBody->as<BlockExpression>());
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
    Token const tokEq = match(Operator::Eq, ErrorStrategy::DefaultErrorMessage);
    pValue = expression();
    throwErrorIfNullOrNotExpression(pValue, tokEq, "expression expected");
  }

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

ast::IfExpression::SPtr Parser::ifExpression()
{
  if (!next(Token::KwIf) && !next(Token::KwElse))
  {
    return nullptr;
  }

  std::vector<IfExpression::Clause> clauses;
  bool hasIfClause = false;
  while (next(Token::KwIf) || next(Token::KwElse))
  {
    Token tokStart;
    Token tokBeforeCondition;
    IfExpression::Clause::Tag tag;

    if (next(Token::KwIf))
    {
      if (hasIfClause)
      {
        // this is the beginning of another if expression
        break;
      }
      tokStart = match(Token::KwIf);
      tokBeforeCondition = tokStart;
      tag = IfExpression::Clause::If;
      hasIfClause = true;
    }
    else // next(Token::KwElse)
    {
      tokStart = match(Token::KwElse);
      if (next(Token::KwIf))
      {
        tokBeforeCondition = match(Token::KwIf);
        tag = IfExpression::Clause::ElseIf;
      }
      else
      {
        tag = IfExpression::Clause::Else;
      }
    }

    Node::SPtr pClauseCondition = nullptr;
    if (tag != IfExpression::Clause::Else)
    {
      pClauseCondition = expression();
      throwErrorIfNullOrNotExpression(pClauseCondition, tokBeforeCondition, "expression expected");
    }

    Node::SPtr pClauseCapture = nullptr;
    if (skip(Runes::Bar))
    {
      pClauseCapture = expression();
      // TODO throwErrorIfNotDestructureExpression
      throwErrorIfNotExpression(pClauseCapture, "expression expected");

      match(Runes::Bar, "'|' expected");
    }

    auto pClauseBody = expression();
    if (!pClauseBody->is<BlockExpression>())
    {
      throw error(pClauseBody, "block expected");
    }
    if (pClauseBody->as<BlockExpression>()->isLabeled())
    {
      // TODO add note: move the label here <first clause>
      throw error(pClauseBody, "label the whole if expression");
    }
    clauses.push_back({tag, tokStart, pClauseCondition, pClauseCapture, pClauseBody->as<BlockExpression>()});

    if (tag == IfExpression::Clause::Else)
    {
      // this is the end of this if expression
      break;
    }
  }

  if (!hasIfClause)
  {
    auto const& clause = clauses.front();
    throw error(clause.tokStart, fmt::format("'{}' missing preceding 'if'", clause.tag));
  }

  return IfExpression::make_shared(std::move(clauses));
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
  else if (auto pTemp = ifExpression(); pTemp != nullptr)
  {
    pRes = pTemp;
  }
  else
  {
    pRes = tokenExpression();
  }

  // TODO add if, loop check
  auto pBlock = std::dynamic_pointer_cast<BlockExpression>(pRes);
  auto pIf = std::dynamic_pointer_cast<IfExpression>(pRes);

  if (pBlock == nullptr && pIf == nullptr && isLabeled)
  {
    throw error(toklabel, "only block, ifs and loops can be labeled");
  }
  if (pBlock != nullptr && isLabeled)
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

bool Parser::next(Operator::Tag tag)
{
  if (!next(Token::Operator))
  {
    return false;
  }
  Token const tokOp = match(Token::Operator);
  d_currentTokenIdx -= 1; // we just want to peek
  return tokOp.text() == fmt::to_string(tag);
}

Token Parser::match(Operator::Tag tag, std::string const& errorMessage)
{
  Token const tokOp = match(Token::Operator, errorMessage);
  if (tokOp.text() != fmt::to_string(tag))
  {
    throw error(tokOp, errorMessage);
  }
  return tokOp;
}

Token Parser::match(Operator::Tag tag, ErrorStrategy strategy)
{
  auto const errorMessage =
    (strategy == ErrorStrategy::Unreachable)
      ? "reached unreachable code in Parser::match()"
      : fmt::format("operator '{}' expected", tag);

  return match(tag, errorMessage);
}

bool Parser::skip(Operator::Tag tag)
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
  // TODO check for node type and add notes
  //   ex: ranges are not expressions, let statements are not expressions, etc

  if (pNode != nullptr && !pNode->isExpression())
  {
    throw error(pNode, message);
  }
}

void Parser::throwErrorIfNullOrNotExpression(Node::SPtr pNode, Token fallbackToken, std::string const& message)
{
  // TODO check for node type and add notes
  //   ex: ranges are not expressions, let statements are not expressions, etc

  if (pNode == nullptr)
  {
    throw Error(d_tokenizer.sourcePath(), fallbackToken.end(), fallbackToken.end(), message);
  }
  if (!pNode->isExpression())
  {
    throw error(pNode, message);
  }
}

Parser::State Parser::currentState() const
{
  return d_stateStack.top();
}

Parser::State Parser::popState()
{
  auto const state = d_stateStack.top();
  if (d_stateStack.size() > 1)
  {
    d_stateStack.pop();
  }
  return state;
}

void Parser::pushState(State state)
{
  d_stateStack.push(state);
}