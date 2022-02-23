#pragma once

#include <parsing/Error.h>
#include <parsing/Token.h>
#include <parsing/Tokenizer.h>
#include <parsing/ast/Nodes.h>

#include <stack>

struct Parser final
{
private:
  enum class ErrorStrategy
  {
    Unreachable,
    DefaultErrorMessage,
  };

  enum ExpressionArgs : size_t
  {
    Optional = 1 << 0,
    CanBeStatement = 1 << 1
  };

  enum class State
  {
    Base,
    FunctionReturnType,
    IfExpression,
  };

private:
  Tokenizer d_tokenizer;
  std::vector<Token> d_tokens;
  std::vector<size_t> d_rollbacks;
  std::stack<State> d_stateStack;
  size_t d_currentTokenIdx;

public:
  template<typename T>
    requires std::is_same_v<T, Tokenizer>
  Parser(T&& tokenizer)
    : d_tokenizer(std::forward<T>(tokenizer))
    , d_currentTokenIdx(0)
  {
    d_stateStack.push(State::Base);
  }

public:
  ast::TypeExpression::SPtr root();

  ast::TokenExpression::SPtr tokenExpression();

  ast::TypeExpression::SPtr typeExpression(bool isRoot = false);

  ast::FunctionExpression::SPtr functionExpression();

  ast::LetStatement::SPtr letStatement();

  ast::BlockExpression::SPtr blockExpression();

  ast::IfExpression::SPtr ifExpression();

  ast::LoopExpression::SPtr loopExpression();

  ast::SwitchExpression::SPtr switchExpression();

  ast::Node::SPtr pred15();

  ast::Node::SPtr atomic();

  inline ast::Node::SPtr expression() { return pred15(); }

private:
  ast::Part::SPtr part();

  ast::Node::SPtr expressionOrPart(); // use only in typeExpression()

  std::tuple<Token, bool> comptime();

  std::tuple<Token, bool> label();

  std::tuple<Token, bool> jumpLabel();

  std::tuple<ast::Node::SPtr, Token> capture();

  template<typename NodeT = ast::Node>
  std::shared_ptr<NodeT> expression(
    size_t args,
    std::string const& errorMessage,
    Position fallback = Position::invalid())
  {
    bool const
      optional = args & Optional,
      canBeStatement = args & CanBeStatement;

    if (!optional) assert(fallback.isValid());

    auto pRes = expression();

    if (pRes == nullptr)
    {
      if (optional)
      {
        return nullptr;
      }
      throw error(fallback, errorMessage);
    }
    if (!pRes->isExpression() && !canBeStatement)
    {
      // TODO add notes saying:
      //   ranges can only be used in array subscripts,
      //   let statements are not expressions,
      //   assigments are not expressions,
      //   etc.
      throw error(pRes, errorMessage);
    }
    if (!pRes->is<NodeT>())
    {
      throw error(pRes, errorMessage);
    }
    return pRes->as<NodeT>();
  }

  template<typename NodeT = ast::Node>
  std::shared_ptr<NodeT> expression(
    std::string const& errorMessage,
    Position fallback = Position::invalid())
  {
    return expression<NodeT>(0, errorMessage, fallback);
  }

private:
  bool next(Token::Tag tag);
  Token match(Token::Tag tag, std::string const& errorMessage, Position position = Position::invalid());
  Token match(Token::Tag tag, ErrorStrategy strategy = ErrorStrategy::Unreachable, Position position = Position::invalid());
  bool skip(Token::Tag tag);

  bool next(Operator::Tag tag);
  // TODO template and construct Token or Operator
  Token match(Operator::Tag tag, std::string const& errorMessage, Position position = Position::invalid());
  Token match(Operator::Tag tag, ErrorStrategy strategy = ErrorStrategy::Unreachable, Position position = Position::invalid());
  bool skip(Operator::Tag tag);

  void matchStatementEnder(Position fallback = Position::invalid());

  Token lastMatchedToken();

  void setRollbackPoint();
  void rollback();
  void commit();

  [[nodiscard]] Error error(Token token, std::string const& message) const;
  [[nodiscard]] Error error(ast::Node::SPtr pNode, std::string const& message) const;
  [[nodiscard]] Error error(Position pos, std::string const& message) const;

  State currentState() const;
  State popState();
  void pushState(State state);

  static bool isDestructuringExpression(ast::Node::SPtr expression);
};