#include "parsing/Parser.h"

// TODO better error messages

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
      pUnderlyingType = expression("type expression or enum body (block) expected", tokTag.end().nextColumn());
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
      auto const pField = pExpr->as<Field>();
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
        throw error(fields.back()->end(), "',' expected");
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

  Position const end = (!isRoot)
    ? match(Token::RBrace, ErrorStrategy::DefaultErrorMessage).end()
    : lastMatchedToken().end();

  return TypeExpression::make_shared(
    tag, tokTag.start(), end, std::move(fields), std::move(declsPre), std::move(declsPost), pUnderlyingType);
}

FunctionExpression::SPtr Parser::functionExpression()
{
  if (!next(Token::KwFn))
  {
    return nullptr;
  }
  Token const tokFn = match(Token::KwFn);

  match(Token::LParen, ErrorStrategy::DefaultErrorMessage);

  size_t commaCount = 0;
  std::vector<FunctionExpression::Parameter> parameters;
  while(next(Token::Symbol))
  {
    Token const tokName = match(Token::Symbol);
    Token const tokColon = match(Token::Colon, ErrorStrategy::DefaultErrorMessage);
    auto const pType = expression("type expression expected", tokColon.end());

    // force commas between parameters
    if (commaCount != parameters.size())
    {
      auto const errPos = parameters.back().type->end();
      throw Error(d_tokenizer.sourcePath(), errPos, errPos, "',' expected");
    }
    parameters.push_back({tokName, pType});
    commaCount += static_cast<size_t>(skip(Token::Comma));
  }

  Token const tokRParen = match(Token::RParen, ErrorStrategy::DefaultErrorMessage);

  pushState(FunctionReturnType);
  auto const pReturnType = expression("type expression expected", tokRParen.end());
  assert(popState() == FunctionReturnType);

  auto const pBody = expression<BlockExpression>(Optional, "block expected");
  if (pBody != nullptr && pBody->isLabeled())
  {
    throw error(pBody->label().start(), "function blocks cannot be labeled");
  }
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
      throw error(parts.back()->end(), "',' expected");
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
    pType = expression(Optional, "type expression expected");
  }

  Node::SPtr pValue = nullptr;
  if (next(Token::Operator))
  {
    Token const tokEq = match(Operator::Eq, ErrorStrategy::DefaultErrorMessage);
    pValue = expression("expression expected", tokEq.end());
  }

  return Part::make_shared(tokSymbol, pType, pValue);
}

BlockExpression::SPtr Parser::blockExpression()
{
  if (!next(Token::LBrace))
  {
    return nullptr;
  }
  Token const tokLBrace = match(Token::LBrace);

  std::vector<Node::SPtr> statements;
  while (auto pStmt = expression())
  {
    statements.push_back(pStmt);
    // FIXME this matches 0 or more, make it match 1 and then error on more then one
    while (skip(Token::Semicolon));
  }
  Token const tokRBrace = match(Token::RBrace, ErrorStrategy::DefaultErrorMessage);

  return BlockExpression::make_shared(
    tokLBrace.start(), tokRBrace.start(), std::move(statements));
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
      pClauseCondition = expression(fmt::format("{} clauses must have conditions", tag), tokBeforeCondition.end().nextColumn());
    }
    else
    {
      setRollbackPoint();
      pClauseCondition = expression(Optional, "expression expected");
      if (pClauseCondition != nullptr && !pClauseCondition->is<BlockExpression>())
      {
        throw error(pClauseCondition, "else clauses don't have conditions");
      }
      rollback();
    }

    Node::SPtr pClauseCapture = nullptr;
    Token tokClosingCapture;
    if (next(Runes::Bar))
    {
      Token const tokBar = match(Runes::Bar);
      pClauseCapture = expression(Destructuring, "destructuring expression expected", tokBar.end());
      tokClosingCapture = match(Runes::Bar, "'|' expected", pClauseCapture->end());
    }

    Position fallback;
    if (pClauseCapture != nullptr)
    {
      // nexColumn for a space "| {"
      fallback = tokClosingCapture.end().nextColumn();
    }
    else if (pClauseCondition != nullptr)
    {
      fallback = pClauseCondition->end().nextColumn();
    }
    else // no condition so this is 'else'
    {
      fallback = tokStart.end().nextColumn();
    }

    auto pClauseBody = expression<BlockExpression>("block expected", fallback);
    clauses.push_back({tag, tokStart, pClauseCondition, pClauseCapture, pClauseBody});

    if (pClauseBody->isLabeled())
    {
      throw error(pClauseBody->label().start(), "individual clause blocks cannot be labeled")
        .note(clauses.front().tokStart.start(), fmt::format("place the label '{}:' here", pClauseBody->labelName()));
    }

    if (tag == IfExpression::Clause::Else)
    {
      // this is the end of this if expression
      break;
    }
  }

  if (!hasIfClause)
  {
    auto const& clause = clauses.front();
    throw error(clause.tokStart, fmt::format("{} clause must have a preceding if clause", clause.tag));
  }

  return IfExpression::make_shared(std::move(clauses));
}

Node::SPtr Parser::expression()
{
  // TODO parse comptime here
  // add marked comptime field to Node?

  bool isLabeled = false;
  Token toklabel;

  if (next(Token::Symbol))
  {
    setRollbackPoint();
    toklabel = match(Token::Symbol);
    if (next(Token::Colon))
    {
      Token const tokColon = match(Token::Colon);
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

  if (isLabeled)
  {
    if (!pRes->is<LabeledNode>())
    {
      throw error(toklabel, "only block, ifs and loops can be labeled");
    }
    pRes->as<LabeledNode>()->setLabel(toklabel);
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

Token Parser::match(Token::Tag tag, std::string const& errorMessage, Position position)
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
    if (position.isValid())
    {
      throw error(position, errorMessage);
    }
    throw error(currentToken, errorMessage);
  }
  d_currentTokenIdx++;
  return currentToken;
}

Token Parser::match(Token::Tag tag, ErrorStrategy strategy, Position position)
{
  auto const errorMessage =
    (strategy == ErrorStrategy::Unreachable)
      ? "reached unreachable code in Parser::match()"
      : fmt::format("{} expected", tag);

  return match(tag, errorMessage, position);
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

Token Parser::match(Operator::Tag tag, std::string const& errorMessage, Position position)
{
  Token const tokOp = match(Token::Operator, errorMessage, position);
  if (tokOp.text() != fmt::to_string(tag))
  {
    if (position.isValid())
    {
      throw error(position, errorMessage);
    }
    throw error(tokOp, errorMessage);
  }
  return tokOp;
}

Token Parser::match(Operator::Tag tag, ErrorStrategy strategy, Position position)
{
  auto const errorMessage =
    (strategy == ErrorStrategy::Unreachable)
      ? "reached unreachable code in Parser::match()"
      : fmt::format("operator '{}' expected", tag);

  return match(tag, errorMessage, position);
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

Error Parser::error(Token token, std::string const& message) const
{
  return Error(d_tokenizer.sourcePath(), token.start(), token.end(), message);
}

Error Parser::error(Node::SPtr pNode, std::string const& message) const
{
  return Error(d_tokenizer.sourcePath(), pNode->start(), pNode->end(), message);
}

Error Parser::error(Position pos, std::string const& message) const
{
  return Error(d_tokenizer.sourcePath(), pos, pos, message);
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