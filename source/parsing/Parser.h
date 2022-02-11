#pragma once

#include <parsing/Error.h>
#include <parsing/Token.h>
#include <parsing/Tokenizer.h>
#include <parsing/ast/Nodes.h>

struct Parser final
{
private:
  using ExpressionParser = ast::Expression::SPtr (Parser::*)();

private:
  Tokenizer d_tokenizer;
  std::vector<Token> d_tokens;
  std::vector<size_t> d_rollbacks;
  size_t d_currentTokenIdx;

public:
  template<typename T>
    requires std::is_same_v<T, Tokenizer>
  Parser(T&& tokenizer)
    : d_tokenizer(std::forward<T>(tokenizer))
    , d_currentTokenIdx(0)
  {}

  ast::Root::SPtr parse();

public: // TODO make private:
  // Nodes
  ast::Statement::SPtr statement();
  ast::LetStatement::SPtr letStatement();
  ast::LetStatement::Part::SPtr letStatementPart();

  [[gnu::always_inline]] inline
  ast::Expression::SPtr expression() { return expr00(); }

  ast::Expression::SPtr atomicExpression();


  [[gnu::always_inline]] inline
  ast::Expression::SPtr binaryExpression(
    ExpressionParser nextParser,
    // TODO these contain chainability and asociativity info?
    std::vector<Operator::Tag> operators);

  // TODO generate these based on Operator::Tag
  ast::Expression::SPtr expr00();
  ast::Expression::SPtr expr01();
  ast::Expression::SPtr expr02();

private:
  bool next(Token::Tag tag);
  Token match(Token::Tag tag, std::string const& errorMessage = "UNREACHABLE");
  bool skip(Token::Tag tag);

  void setRollbackPoint();
  void rollback();
  void commit();
};