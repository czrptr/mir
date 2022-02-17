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
    CanBeStatement = 1 << 1,
    Destructuring = 1 << 2,
  };

  enum State
  {
    FunctionReturnType,
    NotFunctionReturnType,
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
    d_stateStack.push(NotFunctionReturnType);
  }

public:
  ast::TokenExpression::SPtr tokenExpression();

  ast::TypeExpression::SPtr typeExpression(bool isRoot = false);

  ast::FunctionExpression::SPtr functionExpression();

  ast::LetStatement::SPtr letStatement();

  ast::BlockExpression::SPtr blockExpression();

  ast::IfExpression::SPtr ifExpression();

  ast::Node::SPtr expression();

private:
  ast::Part::SPtr part();

  ast::Node::SPtr expressionOrField(); // use only in typeExpression()

  template<typename NodeT = ast::Node>
  std::shared_ptr<NodeT> expression(
    size_t args,
    std::string const& errorMessage,
    Position fallback = Position::invalid())
  {
    static constexpr auto StatementAndDestructuring = CanBeStatement | Destructuring;
    assert((args & StatementAndDestructuring) != StatementAndDestructuring);

    bool const
      optional = args & Optional,
      canBeStatement = args & CanBeStatement;
      // TODO destrcuturing = args & Destructuring;

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
  Token match(Token::Tag tag, std::string const& errorMessage);
  Token match(Token::Tag tag, ErrorStrategy strategy = ErrorStrategy::Unreachable);
  bool skip(Token::Tag tag);

  bool next(Operator::Tag tag);
  // TODO template and construct Token or Operator
  Token match(Operator::Tag tag, std::string const& errorMessage);
  Token match(Operator::Tag tag, ErrorStrategy strategy = ErrorStrategy::Unreachable);
  bool skip(Operator::Tag tag);

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
};