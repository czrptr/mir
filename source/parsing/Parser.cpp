#include "parsing/Parser.h"

// TODO better error messages

using namespace ast;

namespace Runes
{

static constexpr Operator::Tag Bar = Operator::BitOr;

}

TypeExpression::SPtr Parser::root()
{
  auto pRoot = typeExpression(true);
  match(Token::Eof, ErrorStrategy::DefaultErrorMessage);
  return pRoot;
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

    if (tag == TypeExpression::Enum && !next(Token::LBrace))
    {
      pUnderlyingType = expression("type expression or block expected", tokTag.end().nextColumn());
    }

    Position const fallback = (pUnderlyingType != nullptr)
      ? pUnderlyingType->end().nextColumn()
      : tokTag.end().nextColumn();

    match(Token::LBrace, "block expected", fallback);
  }

  std::vector<Part::SPtr> fields;
  std::vector<LetStatement::SPtr>
    declsPre, declsPost, *decls = &declsPre;

  size_t commaCount = 0;
  while (auto pExpr = expressionOrPart())
  {
    if (pExpr->is<LetStatement>())
    {
      decls->push_back(pExpr->as<LetStatement>());
      matchStatementEnder();
    }
    else if (pExpr->is<Part>())
    {
      auto const pField = pExpr->as<Part>();
      decls = &declsPost;

      if (pExpr->isComptime())
      {
        throw error(pField, fmt::format("{:field}s cannot be comptime", tag));
      }

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

  // TODO make end actual end of the expression and not the Eof
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
  std::vector<Part::SPtr> parameters;
  while(auto pExpr = expressionOrPart())
  {
    // TODO check for default values, i.e. all params with default values
    //   must be grouped at the end
    if (!pExpr->is<Part>())
    {
      throw error(pExpr, "parameter definition expected");
    }

    // force commas between parameters
    if (commaCount != parameters.size())
    {
      throw error(parameters.back()->end(), "',' expected");
    }
    parameters.push_back(pExpr->as<Part>());
    commaCount += static_cast<size_t>(skip(Token::Comma));
  }

  Token const tokRParen = match(Token::RParen, ErrorStrategy::DefaultErrorMessage);

  pushState(State::FunctionReturnType);
  auto const pReturnType = expression("type expression expected", tokRParen.end());

  auto const state = popState();
  assert(state == State::FunctionReturnType);

  auto const pBody = expression<BlockExpression>(Optional, "block expected");
  if (pBody != nullptr && pBody->isLabeled())
  {
    throw error(pBody->label().start(), "function blocks cannot be labeled");
  }

  if (pBody != nullptr)
  {
    // this is a function definition
    for (auto const& param : parameters)
    {
      if (!param->hasType())
      {
        throw error(param, "function parameters must have type annotations");
      }
    }
  }
  else
  {
    // this is a function type
    for (auto const& param : parameters)
    {
      if (param->hasValue())
      {
        throw error(param, "function type parameters cannot have default values");
      }
    }
  }

  return FunctionExpression::make_shared(tokFn, std::move(parameters), pReturnType, pBody);
}

LetStatement::SPtr Parser::letStatement()
{
  // TODO parse comptime here
  //  "pub comptime let" not "comptime pub let"

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

  if (next(Token::KwComptime))
  {
    throw error(match(Token::KwComptime), "move the 'comptime' keyword before the 'let'");
  }

  if (skip(Token::KwMut))
  {
    isMut = true;
  }

  size_t commaCount = 0;
  std::vector<Part::SPtr> parts;
  while (auto pExpr = expressionOrPart())
  {
    if (!pExpr->is<Part>())
    {
      throw error(pExpr, "let statement part expected");
    }
    auto const pPart = pExpr->as<Part>();

    if (pPart->isComptime())
    {
      throw error(pPart->tokComptime(), "individual let statement parts cannot be comptime")
        .note(tokLet.start(), "mark the whole statement as comptime here");
    }

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
  // TODO implement workaround when a = b expression will be implemented and this code will break

  auto const [tokComptime, isComptime] = comptime();

  Node::SPtr pAsign = nullptr;

  setRollbackPoint();
  auto const [tokLabel, isLabeled] = label();
  if (isLabeled)
  {
    rollback();
    pAsign = tokenExpression(); // garanteed SymbolExpression
  }
  else
  {
    commit();

    setRollbackPoint();
    pAsign = expression();
    if (!isDestructuringExpression(pAsign))
    {
      rollback();
      return nullptr;
    }
    commit();
  }

  if (pAsign == nullptr)
  {
    return nullptr;
  }

  Node::SPtr pType = nullptr;
  if (next(Token::Colon))
  {
    Token const tokColon = match(Token::Colon);
    pType = expression("type expression expected", tokColon.end());
  }

  Node::SPtr pValue = nullptr;
  if (next(Token::Operator))
  {
    Token const tokEq = match(Operator::Eq, ErrorStrategy::DefaultErrorMessage);
    pValue = expression("expression expected", tokEq.end());
  }

  auto pRes = Part::make_shared(pAsign, pType, pValue);
  if (isComptime)
  {
    pRes->setIsComptime(tokComptime);
  }
  return pRes;
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
    // errors on unassigned function results is done in sema

    // TODO error if the statement isn't a
    //   let, assignment, function call, block, if, loop or switch

    statements.push_back(pStmt);
    if (!pStmt->is<BlockExpression>()
      && !pStmt->is<IfExpression>()
      && !pStmt->is<LoopExpression>()
      && !pStmt->is<SwitchExpression>()
      && !(pStmt->is<DeferStatement>()
        && pStmt->as<DeferStatement>()->target()
          ->is<BlockExpression>()))
    {
      matchStatementEnder(pStmt->end());
    }
  }
  Token const tokRBrace = match(Token::RBrace, ErrorStrategy::DefaultErrorMessage);

  return BlockExpression::make_shared(
    tokLBrace.start(), tokRBrace.end(), std::move(statements));
}

IfExpression::SPtr Parser::ifExpression()
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
      pushState(State::IfExpression);
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
      // TODO the error should be "expression expected"
      //   and the current error should be a note
      pClauseCondition = expression(fmt::format("{} clauses must have conditions", tag), tokBeforeCondition.end().nextColumn());
    }

    auto const [pClauseCapture, tokClosingCapture] = capture();

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

  if (currentState() != State::IfExpression && !hasIfClause)
  {
    // fixes: if cond else {} being parsed as if cond (else {})
    // i.e. the else {} is parsed as the if's block

    // TODO else clauses must have a preceding if or loop clause
    auto const& clause = clauses.front();
    throw error(clause.tokStart, fmt::format("{} clause must have a preceding if clause", clause.tag));
  }

  if (hasIfClause)
  {
    auto const state = popState();
    assert(state == State::IfExpression);
  }

  return IfExpression::make_shared(std::move(clauses));
}

LoopExpression::SPtr Parser::loopExpression()
{
  if (!next(Token::KwLoop))
  {
    return nullptr;
  }
  Token const tokLoop = match(Token::KwLoop);

  // TODO add note: loops must have conditions
  auto const pCondition = expression("expression expected", tokLoop.end().nextColumn());

  auto const [pCapture, tokClosingCapture] = capture();

  Position fallback = (pCapture != nullptr)
    ? tokClosingCapture.end().nextColumn()
    : pCondition->end().nextColumn();

  pushState(State::IfExpression);
  // prevents loop a else to error with "else missing if"
  auto pBody = expression<BlockExpression>("block expected", fallback);
  if (pBody->isLabeled())
  {
    throw error(pBody->label().start(), "individual clause blocks cannot be labeled")
      .note(tokLoop.start(), fmt::format("place the label '{}:' here", pBody->labelName()));
  }
  auto const state = popState();
  assert(state == State::IfExpression);

  Token tokElse;
  Node::SPtr pElseCapture = nullptr;
  BlockExpression::SPtr pElseBody = nullptr;

  if (next(Token::KwElse))
  {
    tokElse = match(Token::KwElse);

    auto const [pElseCaptureAux, tokClosingCapture] = capture();
    pElseCapture = pElseCaptureAux;

    Position fallback = (pElseCapture != nullptr)
      ? tokClosingCapture.end().nextColumn()
      : tokElse.end().nextColumn();

    pElseBody = expression<BlockExpression>("block expected", fallback);
    if (pElseBody->isLabeled())
    {
      throw error(pElseBody->label().start(), "individual clause blocks cannot be labeled")
        .note(tokLoop.start(), fmt::format("place the label '{}:' here", pElseBody->labelName()));
    }
  }

  return LoopExpression::make_shared(
    tokLoop, pCondition, pCapture, pBody, tokElse, pElseCapture, pElseBody);
}

SwitchExpression::SPtr Parser::switchExpression()
{
  if (!next(Token::KwSwitch))
  {
    return nullptr;
  }
  Token const tokSwitch = match(Token::KwSwitch);

  auto const pValue = expression("expression expected", tokSwitch.end().nextColumn());

  auto const [tokLabel, isLabeled] = label();
  if (isLabeled)
  {
    throw error(tokLabel, "switch blocks cannot be labeled");
  }

  Token const tokLBrace = match(Token::LBrace, "block expected", pValue->end().nextColumn());

  std::vector<SwitchExpression::Case> cases;
  size_t commaCount = 0;
  while (true)
  {
    auto const pCaseValue = expression();
    if (pCaseValue == nullptr)
    {
      break;
    }
    auto const [pCapture, tokClosingCapture] = capture();

    Position const fallback = (pCapture != nullptr)
      ? tokClosingCapture.end().nextColumn()
      : pCaseValue->end().nextColumn();

    Token const tokArrow = match(Token::ThickArrow, ErrorStrategy::DefaultErrorMessage, fallback);

    auto const pResult = expression("expression expected", tokArrow.end().nextColumn());

    // force commas between fields
    if (commaCount != cases.size())
    {
      throw error(cases.back().result->end(), "',' expected");
    }
    cases.push_back({pCaseValue, pCapture, pResult});
    commaCount += static_cast<size_t>(skip(Token::Comma));
  }

  if (cases.empty())
  {
    throw error(tokLBrace.end(), "switch must be exhaustive");
  }

  Token const tokRBrace = match(Token::RBrace, ErrorStrategy::DefaultErrorMessage, cases.back().result->end().nextColumn());

  return SwitchExpression::make_shared(tokSwitch, pValue, std::move(cases), tokRBrace.end());
}

Node::SPtr Parser::expression()
{
  auto const [tokComptime, isComptime] = comptime();

  auto const [tokLabel, isLabeled] = label();

  auto pRes = pred15();

  if (isComptime)
  {
    if (pRes != nullptr)
    {
      if (pRes->is<LetStatement>())
      {
        // TODO add note with let location
        throw error(tokComptime, "move the 'comptime' keyword before the 'let'");
      }
      pRes->setIsComptime(tokComptime);
    }
    else
    {
      throw error(tokComptime.end().nextColumn(), "expression expected");
    }
  }

  if (isLabeled)
  {
    if (pRes != nullptr)
    {
      if (!pRes->is<LabeledNode>())
      {
        // TODO? error for labeled nothing (pRes == nullptr)?
        throw error(tokLabel, "only block, ifs and loops can be labeled");
      }
      pRes->as<LabeledNode>()->setLabel(tokLabel);
    }
    else
    {
      throw error(tokLabel.end().nextColumn(), "expression expected");
    }
  }

  return pRes;
}

Node::SPtr Parser::pred15()
{
  if (next(Operator::Return))
  {
    Token const tokBreak = match(Operator::Return);

    auto const [tokLabel, isLabeled] = jumpLabel();
    if (isLabeled)
    {
      Position pos = tokLabel.start();
      pos.column -= 1;
      throw error(pos, "return statements don't take labels");
    }

    auto const pTarget = atomic();

    return ReturnStatement::make_shared(tokBreak, pTarget);
  }
  if (next(Operator::Break))
  {
    Token const tokBreak = match(Operator::Break);
    auto const [tokLabel, isLabeled] = jumpLabel();
    auto const pTarget = atomic();

    return BreakStatement::make_shared(tokBreak, isLabeled, tokLabel, pTarget);
  }
  if (next(Operator::Continue))
  {
    Token const tokContinue = match(Operator::Continue);
    auto const [tokLabel, isLabeled] = jumpLabel();

    return std::make_shared<ContinueStatement>(tokContinue, isLabeled, tokLabel);
  }
  if (next(Operator::Defer))
  {
    Token const tokDefer = match(Operator::Defer);

    auto const [tokLabel, isLabeled] = jumpLabel();
    if (isLabeled)
    {
      throw error(tokLabel, "defer statements don't take labels");
    }

    auto const pTarget = atomic();
    if (pTarget == nullptr)
    {
      throw error(tokDefer.end().nextColumn(), "expression expected");
    }
    return DeferStatement::make_shared(tokDefer, pTarget);
  }
  return atomic();
}

Node::SPtr Parser::atomic()
{
  if (auto pRes = typeExpression(); pRes != nullptr)
  {
    return pRes;
  }
  if (auto pRes = functionExpression(); pRes != nullptr)
  {
    return pRes;
  }
  if (auto pRes = letStatement(); pRes != nullptr)
  {
    return pRes;
  }
  if (auto pRes = blockExpression(); pRes != nullptr)
  {
    return pRes;
  }
  if (auto pRes = ifExpression(); pRes != nullptr)
  {
    return pRes;
  }
  if (auto pRes = loopExpression(); pRes != nullptr)
  {
    return pRes;
  }
  if (auto pRes = switchExpression(); pRes != nullptr)
  {
    return pRes;
  }
  return tokenExpression();
}

Node::SPtr Parser::expressionOrPart()
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

std::tuple<Token, bool> Parser::comptime()
{
  if (!next(Token::KwComptime))
  {
    return {Token(), false};
  }
  return {match(Token::KwComptime), true};
}

std::tuple<Token, bool> Parser::label()
{
  std::tuple<Token, bool> const fail = {Token(), false};

  if (!next(Token::Symbol))
  {
    return fail;
  }

  setRollbackPoint();
  Token const toklabel = match(Token::Symbol);

  if (!next(Token::Colon))
  {
    rollback();
    return fail;
  }

  Token const tokColon = match(Token::Colon);
  if (toklabel.end() != tokColon.start())
  {
    // marking labels must be symbols suffixed with ':'
    throw error(tokColon, fmt::format("did you mean '{}:'", toklabel.text()));
  }

  commit();
  return {toklabel, true};
}

std::tuple<Token, bool> Parser::jumpLabel()
{
  std::tuple<Token, bool> const fail = {Token(), false};

  if (!next(Token::Colon))
  {
    return fail;
  }

  setRollbackPoint();
  Token const tokColon = match(Token::Colon);

  if (!next(Token::Symbol))
  {
    rollback();
    return fail;
  }

  Token const tokLabel = match(Token::Symbol);
  if (tokColon.end() != tokLabel.start())
  {
    // jumping labels must be symbols prefixed with ':'
    throw error(tokLabel, fmt::format("did you mean ':{}'", tokLabel.text()));
  }

  commit();
  return {tokLabel, true};
}

std::tuple<Node::SPtr, Token> Parser::capture()
{
  Node::SPtr pCapture = nullptr;
  Token tokClosingBar;
  if (next(Runes::Bar))
  {
    Token const tokBar = match(Runes::Bar);
    pCapture = expression("destructuring expression expected", tokBar.end());
    if (!isDestructuringExpression(pCapture))
    {
      throw error(pCapture, "destructuring expression expected");
    }
    tokClosingBar = match(Runes::Bar, "'|' expected", pCapture->end());
  }
  return {pCapture, tokClosingBar};
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

void Parser::matchStatementEnder(Position fallback)
{
  match(Token::Semicolon, ErrorStrategy::DefaultErrorMessage, fallback);
  if (next(Token::Semicolon))
  {
    Token const tokSemicolon = match(Token::Semicolon);
    throw error(tokSemicolon, "remove superfluous ';'");
  }
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

bool Parser::isDestructuringExpression(ast::Node::SPtr expression)
{
  // TODO tuple, array
  return expression->is<SymbolExpression>();
}