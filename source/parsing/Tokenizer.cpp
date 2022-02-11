#include "parsing/Tokenizer.h"

#include <parsing/TokenizerCases.h>
#include <parsing/Intern.h>
#include <parsing/Operator.h>
#include <Utils.h>

#include <string_view>
#include <filesystem>
#include <map>
#include <cassert>
#include <cstring>

namespace fs = std::filesystem;

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

std::map<std::string_view, Token::Tag> const keywords
{
  {"pub", Token::KwPub},
  {"let", Token::KwLet},
  {"mut", Token::KwMut},
  {"comptime", Token::KwComptime},
  {"struct", Token::KwStruct},
  {"enum", Token::KwEnum},
  {"union", Token::KwUnion},
  {"fn", Token::KwFn},
  {"if", Token::KwIf},
  {"else", Token::KwElse},
  {"switch", Token::KwSwitch},
  {"loop", Token::KwLoop},
  {"import", Token::KwImport}, // make reserved symbol?
  {"return", Token::Operator},
  {"break", Token::Operator},
  {"continue", Token::Operator},
  {"defer", Token::Operator},
  {"try", Token::Operator},
  {"catch", Token::Operator},
  {"orelse", Token::Operator},
  {"not", Token::Operator},
  {"and", Token::Operator},
  {"or", Token::Operator}
  // {"infer", Token::Operator} // TODO
};

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
    throw Error(d_sourcePath, "source text is empty");
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
    throw Error(d_sourcePath, "file doesn't exist");
  }

  if (fs::file_size(filePath) == 0)
  {
    throw Error(d_sourcePath, "source text is empty");
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
    advance();

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
          SWITCH_NEXT_CHAR()
         {
          case '/':
          {
            tokenStart(Token::Comment);
            advance();
            d_currentState = LineComment;
          }
          break;

          case '*':
          {
            tokenStart(Token::Comment);
            advance();
            commentNestLevel += 1;
            d_currentState = BlockComment;
          }
          break;

          default:
          {
            tokenStart(Token::Operator);
          }
          break;
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

      case '@':
      {
        throw error("'@' can only be used as a prefix to denote compiler builtin functions");
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
      case '\n':
      case '\t':
      case '\r':
      {
        // TODO multiline string literals (see Swift)
        throw error("string literals cannot contain unescaped new lines, tabs or carriage returns");
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

    SWITCH_CURRENT_CHAR(LineComment)
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
    END(); // LineComment

    SWITCH_CURRENT_CHAR(BlockComment)
    {
      case '/':
      {
        SWITCH_NEXT_CHAR()
        {
          case '*':
          {
            advance();
            commentNestLevel += 1;
          }
          break;

          default:
          {
            // continue parsing
          }
          break;
        }
      }
      break;

      case '*':
      {
        SWITCH_NEXT_CHAR()
        {
          case '/':
          {
            advance();
            commentNestLevel -= 1;
          }
          break;

          default:
          {
            // continue parsing
          }
          break;
        }
      }
      break;

      default:
      {
        if (commentNestLevel == 0)
        {
          return tokenEnd();
        }
        else
        {
          // continue parsing
        }
      }
      break;
    }
    END(); // BlockComment
    }
  }

  if (d_currentState == StringLiteral)
  {
    throw Error(d_sourcePath, d_nextPos, "string literal missing terminating '\"'")
      .note(d_currentToken.d_start, "string literal starts here");
  }

  if (d_currentState == BlockComment && commentNestLevel > 0)
  {
    throw Error(d_sourcePath, d_nextPos, "block comment missing terminating '*/'")
      .note(d_currentToken.d_start, "block comment starts here");
  }

  if (!d_currentTokenText.empty())
  {
    // compensate for trim in tokenEnd()
    d_currentTokenText += " ";
    // compensate for skipped position update;
    d_currentPos.column += 1;
    Token const res = tokenEnd();
    d_leftOver = false; // tokenEnd() always assumes a char is left over
    return res;
  }

  return Token(Token::Eof, d_nextPos, d_nextPos, "");
}

std::string const& Tokenizer::sourcePath() const
{
  return d_sourcePath;
}

bool Tokenizer::inputStreamFinished() const
{
  return d_pInputStream->eof() && !d_leftOver;
}

char Tokenizer::inputPeek()
{
  // TODO return \0 on Eof
  try
  {
    return static_cast<char>(d_pInputStream->peek());
  }
  catch (std::ios::failure const& err)
  {
    throw error(err.what());
  }
}

char Tokenizer::inputNext()
{
  // TODO return \0 on Eof
  try
  {
    char res;
    d_pInputStream->get(res);
    return res;
  }
  catch (std::ios::failure const& err)
  {
    throw error(err.what());
  }
}

void Tokenizer::advance()
{
  d_currentPos = d_nextPos;

  if (!d_leftOver)
  {
    d_currentChar = inputNext();
    d_nextChar = inputPeek(); // maybe get rid of this?
  }
  else
  {
    d_leftOver = false;
  }
  d_currentTokenText += d_currentChar;

  if (d_currentChar == '\n')
  {
    d_nextPos.line += 1;
    d_nextPos.column = 0;
  }
  else
  {
    d_nextPos.column += 1;
  }
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
  // TODO don't intern keywords or operators
  // because it cannot vary, you can "calculate" its string representation
  d_currentToken.d_text = Intern::string(d_currentTokenText);
  d_currentTokenText.clear();

  // backtrack position
  d_nextPos = d_currentPos;

  if (d_currentToken.d_tag == Token::Symbol)
  {
    if (keywords.contains(d_currentToken.d_text))
    {
      d_currentToken.d_tag = keywords.at(d_currentToken.d_text);
    }
  }

  // TODO errors for a and= b, a not= b, etc...
  if (d_currentToken.d_tag == Token::Operator)
  {
    auto const errMsg = Operator::validate(d_currentToken.d_text);
    if (!errMsg.empty())
    {
      throw Error(d_sourcePath, d_currentToken.d_start, d_currentToken.d_end, errMsg);
    }
  }

  return d_currentToken;
}

Error Tokenizer::error(std::string const& message) const
{
  return Error(d_sourcePath, d_currentPos, d_currentPos.nextColumn(), message);
}
