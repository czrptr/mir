#pragma once

#include <parsing/Position.h>
#include <parsing/Error.h>
#include <parsing/Token.h>

#include <sstream>
#include <fstream>
#include <functional>

#include <string>

struct Tokenizer final
{
private:
  // Types
	enum State
	{
		Start = -1,
		Symbol = 0,
		NumberLiteral = 1,
		StringLiteral = 2,
		Operator = 3,
		NumberLiteralFaction,
		StringLiteralEscape,
    StringLiteralEnd,
		LineComment,
		BlockComment,
	};

  // keep Token::Tag and Tokenizer::State in sync
  static_assert(static_cast<int>(State::Symbol) == static_cast<int>(Token::Symbol));
  static_assert(static_cast<int>(State::NumberLiteral) == static_cast<int>(Token::NumberLiteral));
  static_assert(static_cast<int>(State::StringLiteral) == static_cast<int>(Token::StringLiteral));
  static_assert(static_cast<int>(State::Operator) == static_cast<int>(Token::Operator));

  // Data
  std::istream& d_inputStream;

	std::string const d_sourcePath;

  bool d_leftOver = false;
  char d_currentChar;
  char d_nextChar;

  Position d_currentPos;
	Position d_nextPos;

  std::string d_currentTokenText;

  State d_currentState;
	Token d_currentToken;

public:
  // Constructors
  Tokenizer(
    std::istream& input,
    std::string const& sourcePath);

  // Methods
	Token next();
  std::string const& sourcePath() const;

private:
  bool inputStreamFinished() const;
  char inputPeek();
  char inputNext();

	void advance();

	void tokenStart(Token::Tag tag); // change into macro?
	[[nodiscard]] Token tokenEnd();
	[[nodiscard]] Token token(Token::Tag tag);

  Error error(std::string const& message) const;
};