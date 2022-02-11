#include "parsing/Tokenizer.h"

#include <parsing/Intern.h>
#include <Utils.h>

#include <cassert>
#include <cstring>
#include <string_view>
#include <filesystem>

namespace fs = std::filesystem;

// TODO refactor
// preallocated a buffer to placement construct a string with the read text
// read chuncks in there while tokenizing

#define LETTER \
       'a' ... 'z': \
  case 'A' ... 'Z'

#define DIGIT_NO_ZERO \
  '1' ... '9'

#define DIGIT \
  '0' ... '9'

#define OPERATOR \
       '=': \
  case '<': \
  case '>': \
  case '+': \
  case '-': \
  case '*': \
  case '%': \
  case '!': \
  case '|': \
  case '^': \
  case '&': \
  case '~': \
  case '?': \
  case '.'

#define SWITCH_CURRENT_CHAR(state) case state: { switch (d_currentChar)

#define SWITCH_NEXT_CHAR() switch (d_nextChar)

#define END() } break

namespace {

void customDeleter(std::istream* pStream)
{
  if (auto* pStreamFile = dynamic_cast<std::ifstream*>(pStream); pStreamFile != nullptr)
  {
    pStreamFile->close();
  }
  delete pStream;
}

} // anonymous

// TODO support for utf8 unicode & better error messages
Tokenizer::Tokenizer(
    std::string const& text,
    std::string const& sourcePath)
  : d_sourcePath(sourcePath)
  , d_currentPos(0, 0)
  , d_nextPos(0, 0)
{
  if (text.empty())
  {
    throw Error::justMessage(d_sourcePath, "source text is empty");
  }
  d_pInputStream = InputStream(new std::istringstream(text, std::ios::in), customDeleter);
  d_pInputStream->exceptions(std::ios::failbit | std::ios::badbit);
}

Tokenizer::Tokenizer(std::string const& sourcePath)
  : d_sourcePath(sourcePath)
  , d_currentPos(0, 0)
  , d_nextPos(0, 0)
{
  fs::path filePath(sourcePath);
  if (!fs::exists(filePath))
  {
    throw Error::justMessage(d_sourcePath, "file doesn't exist");
  }

  if (fs::file_size(filePath) == 0)
  {
    throw Error::justMessage(d_sourcePath, "source text is empty");
  }

  d_pInputStream = InputStream(new std::ifstream(sourcePath, std::ios::in), customDeleter);
  d_pInputStream->exceptions(std::ios::failbit | std::ios::badbit);
}

Token Tokenizer::next()
{
  // setup
  int commentNestLevel = 0;
  d_currentState = Start;

  while (!inputStreamFinished())
  {
    d_currentPos = d_nextPos;

    if (!d_leftOver)
    {
      d_currentChar = inputNext(); // also append to d_currentTokenText
      d_nextChar = inputPeek(); // maybe get rid of this?
    }
    else
    {
      d_leftOver = false;
    }

    if (d_currentChar == '\n')
    {
      d_nextPos.line += 1;
      d_nextPos.column = 0;
    }
    else
    {
      d_nextPos.column += 1;
    }

    switch (d_currentState)
    {
    SWITCH_CURRENT_CHAR(Start)
    {
      case ' ':
      case '\t':
      case '\n':
      {
        // skip
        if (!d_currentTokenText.empty())
        {
          d_currentTokenText.pop_back();
        }
      }
      break;

      case ',':
      {
        return token(Token::Comma);
      }
      break;

      case ':':
      {
        return token(Token::Colon);
      }
      break;

      case ';':
      {
        return token(Token::Semicolon);
      }
      break;

      case '(':
      {
        return token(Token::LParen);
      }
      break;

      case ')':
      {
        return token(Token::RParen);
      }
      break;

      case '[':
      {
        return token(Token::LBracket);
      }
      break;

      case ']':
      {
        return token(Token::RBracket);
      }
      break;

      case '{':
      {
        return token(Token::LBrace);
      }
      break;

      case '}':
      {
        return token(Token::RBrace);
      }
      break;

      case '/':
      {
        if (d_nextChar == '/')
        {
          tokenStart(Token::Comment);
          advance();
          d_currentState = SingleLineComment;
        }
        else if (d_nextChar == '*')
        {
          tokenStart(Token::Comment);
          advance();
          commentNestLevel += 1;
          d_currentState = MultiLineComment;
        }
        else
        {
          tokenStart(Token::Operator);
        }
      }
      break;

      case '@':
      case LETTER:
      {
        tokenStart(Token::Symbol);
      }
      break;

      case OPERATOR:
      {
        tokenStart(Token::Operator);
      }
      break;

      case DIGIT:
      {
        tokenStart(Token::NumberLiteral);
      }
      break;

      case '"':
      {
        tokenStart(Token::StringLiteral);
      }
      break;

      default:
      {
        throw error(fmt::format("unexpected character '{}'", d_currentChar));
      }
      break;
    }
    END(); // case Start

    SWITCH_CURRENT_CHAR(Symbol)
    {
      case LETTER:
      case DIGIT:
      case '_':
      {
        // continue parsing
      }
      break;

      default:
      {
        return tokenEnd();
      }
      break;
    }
    END(); // case Symbol

    SWITCH_CURRENT_CHAR(Operator)
    {
      case OPERATOR:
      {
        // continue parsing
      }
      break;

      default:
      {
        return tokenEnd();
      }
      break;
    }
    END(); // Operator

    SWITCH_CURRENT_CHAR(NumberLiteral)
    {
      // TODO separators ('), bases(0b, 0o, 0x), trailing types (0i32, 3.14f64)

      case DIGIT:
      {
        // continue parsing
      }
      break;

      case '.':
      {
        SWITCH_NEXT_CHAR()
        {
          case DIGIT:
          {
            d_currentState = NumberLiteralFaction;
          }
          break;

          default:
          {
            return tokenEnd();
          }
          break;
        }
      }
      break;

      default:
      {
        return tokenEnd();
      }
      break;
    }
    END(); // NumberLiteral

    SWITCH_CURRENT_CHAR(NumberLiteralFaction)
    {
      case DIGIT:
      {
        // continue parsing
      }
      break;

      default:
      {
        return tokenEnd();
      }
      break;
    }
    END(); // NumberLiteralFaction

    SWITCH_CURRENT_CHAR(StringLiteral)
    {
      case '\t':
      case '\n':
      case '\r':
      {
        // TODO multiline string literals (see Swift)
        throw error("string literals cannot contain unescaped tabs, carriage returns or new lines");
      }
      break;

      case '\\':
      {
        d_currentState = StringLiteralEscape;
      }
      break;

      case '"':
      {
        d_currentState = StringLiteralEnd;
      }
      break;

      default:
      {
        // continue parsing
      }
      break;
    }
    END(); // StringLiteral

    SWITCH_CURRENT_CHAR(StringLiteralEscape)
    {
      case '\\':
      case '"':
      case 'r':
      case 't':
      case 'n':
      {
        d_currentState = StringLiteral;
      }
      break;

      default:
      {
        throw error(fmt::format("unknown escape sequence '\\{}'", d_currentChar));
      }
      break;
    }
    END(); // StringLiteralEscape

    SWITCH_CURRENT_CHAR(StringLiteralEnd)
    {
      default:
      {
        return tokenEnd();
      }
      break;
    }
    END(); // StringLiteralEscape

    SWITCH_CURRENT_CHAR(SingleLineComment)
    {
      case '\n':
      {
        // the new line will be processed on the next call so we
        // dont need to set newLine = true
        return tokenEnd();
      }
      break;

      default:
      {
        // continue parsing
      }
      break;
    }
    END(); // SingleLineComment

    SWITCH_CURRENT_CHAR(MultiLineComment)
    {
      case '/':
      {
        if (d_nextChar == '*')
        {
          advance();
          commentNestLevel += 1;
        }
      }
      break;

      case '*':
      {
        if (d_nextChar == '/')
        {
          advance();
          commentNestLevel -= 1;
          if (commentNestLevel == 0)
          {
            return tokenEnd();
          }
          else if (commentNestLevel < 0)
          {
            throw error("unpaired multiline comment ending");
          }
        }
      }
      break;

      default:
      {
        // continue parsing
      }
      break;
    }
    END(); // MultiLineComment
    }
  }
  // while (!inputStreamFinished());

  if (!d_currentTokenText.empty())
  {
    // compensate for trim in tokenEnd()
    d_currentTokenText += " ";
    // compensate for skipped position update;
    d_currentPos.column += 1;
    return tokenEnd();
  }

  auto const eofPos = d_currentPos.nextColumn();
  return Token(Token::Eof, eofPos, eofPos, "Eof");
}

bool Tokenizer::inputStreamFinished() const
{
  return d_pInputStream->eof() && !d_leftOver;
}

char Tokenizer::inputPeek()
{
  try
  {
    return d_pInputStream->peek();
  }
  catch (std::ios::failure const& err)
  {
    throw error(err.what());
  }
}

char Tokenizer::inputNext()
{
  try
  {
    char res;
    d_pInputStream->get(res);
    d_currentTokenText += res;
    return res;
  }
  catch (std::ios::failure const& err)
  {
    throw error(err.what());
  }
}

void Tokenizer::advance()
{
  d_currentPos = d_currentPos.nextColumn();
}

void Tokenizer::tokenStart(Token::Tag tag)
{
  d_currentToken.d_tag = tag;
  d_currentToken.d_start = d_currentPos;
  d_currentState = static_cast<State>(tag);
}

Token Tokenizer::token(Token::Tag tag)
{
  d_currentToken.d_tag = tag;
  d_currentToken.d_start = d_currentPos;
  d_currentToken.d_end = d_currentPos.nextColumn();
  d_currentToken.d_text = Intern::string(std::string(&d_currentChar, 1));
  if (!d_currentTokenText.empty())
  {
    d_currentTokenText.pop_back();
  }
  return d_currentToken;
}

Token Tokenizer::tokenEnd()
{
  d_leftOver = true;
  if (!d_currentTokenText.empty())
  {
    d_currentTokenText.pop_back();
  }

  d_currentToken.d_end = d_currentPos;
  d_currentToken.d_text = Intern::string(d_currentTokenText);
  d_currentTokenText.clear();

  d_nextPos = d_currentPos; // backtrack position

  if (d_currentToken.d_tag == Token::Symbol)
  {
    // TODO construct only once
    d_currentToken.d_tag = Switch<std::string_view, Token::Tag>(d_currentToken.d_text)
      // TODO order cases in order of observerd usage
      .Case("pub", Token::KwPub)
      .Case("let", Token::KwLet)
      .Case("mut", Token::KwMut)
      .Case("comptime", Token::KwComptime)
      .Case("struct", Token::KwStruct)
      .Case("enum", Token::KwEnum)
      .Case("union", Token::KwUnion)
      .Case("fn", Token::KwFn)
      .Case("try", Token::KwTry)
      .Case("catch", Token::KwCatch)
      .Case("orelse", Token::KwOrelse)
      .Case("if", Token::KwIf)
      .Case("else", Token::KwElse)
      .Case("switch", Token::KwSwitch)
      .Case("loop", Token::KwLoop)
      .Case("return", Token::KwReturn)
      .Case("break", Token::KwBreak)
      .Case("continue", Token::KwContinue)
      .Case("defer", Token::KwDefer)
      .Case("import", Token::KwImport)
      .Default(d_currentToken.d_tag);
  }

  return d_currentToken;
}

Error Tokenizer::error(std::string const& message)
{
  return Error(d_sourcePath, d_currentPos, d_currentPos.nextColumn(), Error::Type::Error, message);
}
