#pragma once

#include <parsing/Error.h>
#include <parsing/Token.h>
#include <parsing/Tokenizer.h>
#include <parsing/ast/Nodes.h>

struct Parser final
{
private:
  Tokenizer d_tokenizer;
  std::vector<Token> d_tokens;
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
  ast::Expression::SPtr atomicExpression();

private:
  bool next(Token::Tag tag);
  Token match(Token::Tag tag, std::string const& errorMessage = "UNREACHABLE");
  bool skip(Token::Tag tag);
  void commit();
};