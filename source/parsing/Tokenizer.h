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
		Start = -2,
		Finished = -1,
		Symbol = 0,
		Operator = 1,
		NumberLiteral = 2,
		StringLiteral = 3,
		NumberLiteralFaction,
		StringLiteralEscape,
    StringLiteralEnd,
		SingleLineComment,
		MultiLineComment
	};
	// TODO enum equality static asserts

  using Deleter = std::function<void(std::istream*)>;
  using InputStream = std::unique_ptr<std::istream, Deleter>;

  // Data
  InputStream d_pInputStream;

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
    std::string const& text,
    std::string const& sourcePath);

  explicit Tokenizer(std::string const& sourcePath);

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