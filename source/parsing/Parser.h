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
  ast::Part::SPtr part();

  ast::BlockExpression::SPtr blockExpression();

  ast::Node::SPtr expression();

  ast::Node::SPtr expressionOrField(); // use only int typeExpression()

private:
  bool next(Token::Tag tag);
  Token match(Token::Tag tag, std::string const& errorMessage);
  Token match(Token::Tag tag, ErrorStrategy strategy = ErrorStrategy::Unreachable);
  bool skip(Token::Tag tag);
  Token lastMatchedToken();

  void setRollbackPoint();
  void rollback();
  void commit();

  // TODO make these const
  [[nodiscard]] Error error(Token token, std::string const& message);
  [[nodiscard]] Error error(ast::Node::SPtr pNode, std::string const& message);
  void throwErrorIfNotExpression(ast::Node::SPtr pNode, std::string const& message);
  void throwErrorIfNullOrNotExpression(ast::Node::SPtr pNode, Token fallbackToken, std::string const& message);

  State currentState() const;
  State popState();
  void pushState(State state);
};