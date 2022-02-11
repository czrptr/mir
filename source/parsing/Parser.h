#pragma once

#include <parsing/Error.h>
#include <parsing/Token.h>
#include <parsing/Tokenizer.h>
#include <parsing/ast/Nodes.h>

// TODO move tokenizer into CurrentParsing somehow

struct Parser
{
private:
  // maybe commit and rollback methods on Parser were better?
  struct CurrentParsing
  {
    friend struct Parser;

  private:
    Parser* d_pParser;
    size_t const d_prevTokenIdx;
    bool d_commit = false;

  public:
    CurrentParsing(Parser* pParser)
      : d_pParser(pParser)
      , d_prevTokenIdx(pParser->d_currentTokenIdx)
    {}

    bool next(Token::Tag tag);
    Token match(Token::Tag tag, std::string const& errorMessage = "UNREACHABLE");
    bool skip(Token::Tag tag);

    void commit() { d_commit = true; }
    Position end() const { return d_pParser->d_tokens.back().end(); }

    ~CurrentParsing();
  };

private:
  Tokenizer d_tokenizer;
  std::vector<Token> d_tokens;
  size_t d_currentTokenIdx;

public:
  Parser(std::string const& sourceText)
    : d_tokenizer(sourceText)
    , d_currentTokenIdx(0)
  {}

  ast::Root::SPtr parse();

public:
  // Nodes
  ast::Statement::SPtr statement();
  ast::LetStatement::SPtr letStatement();
  ast::LetStatement::Part::SPtr letStatementPart();
  ast::Expression::SPtr expression();
};