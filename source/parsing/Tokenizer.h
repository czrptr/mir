#pragma once

#include <parsing/Position.h>
#include <parsing/Token.h>

#include <string>

struct Tokenizer
{
private:
  // Types
	enum State
	{
		Start = -2,
		Finished = -1,
		Symbol = 0,
		Operator = 1,
		NumberLiteral = 2,
		StringLiteral = 3,
		NumberLiteralFaction,
		StringLiteralEscape,
		SingleLineComment,
		MultiLineComment,
		TokenEnd,
	};
	// TODO enum equality static asserts

  // Data
	std::string d_sourceText;
	size_t d_currentIdx;
	size_t d_currentTokenStartIdx;
	Position d_currentPos;
	Position d_previousPos;
	State d_currentState;
	Token d_currentToken;

public:
  // Constructors
	explicit Tokenizer(std::string const& sourceText);

  // Methods
	Token next();

private:
	void advance();

	void token(Token::Tag tag);

	void tokenStart(Token::Tag tag);
	void tokenEnd();
};