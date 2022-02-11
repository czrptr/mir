#include "parsing/Tokenizer.h"

#include <cstring>
#include <string_view>

#include <parsing/Intern.h>
#include <parsing/Error.h>
#include <Utils.h>

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
	case '?'

#define SWITCH_CURRENT_CHAR(state) case state: { switch (currentChar)

#define END } break

// TODO support for utf8 unicode & better error messages
Tokenizer::Tokenizer(std::string const& sourceText)
	: d_sourceText(sourceText + " ") // FIXME token ending on EOF
	, d_currentIdx(0)
	, d_currentTokenStartIdx(0)
	, d_currentPos(0, 0)
	, d_previousPos(0, 0)
{}

Token Tokenizer::next()
{
	// setup
	int commentNestLevel = 0;
	d_currentState = Start;

	while (d_currentIdx < d_sourceText.length())
	{
		char const currentChar = d_sourceText[d_currentIdx];
		// will be '\0' on last iteration
		char const nextChar = d_sourceText[d_currentIdx + 1];
		bool newLine = false;

		switch (d_currentState)
		{
		SWITCH_CURRENT_CHAR(Start)
		{
			case ' ':
			case '\t':
			{
				// skip
			}
			break;

			case '\n':
			{
				newLine = true;
			}
			break;

			case '.':
			{
				token(Token::Dot);
			}
			break;

			case ',':
			{
				token(Token::Comma);
			}
			break;

			case ':':
			{
				token(Token::Colon);
			}
			break;

			case ';':
			{
				token(Token::Semicolon);
			}
			break;

			case '(':
			{
				token(Token::LParen);
			}
			break;

			case ')':
			{
				token(Token::RParen);
			}
			break;

			case '[':
			{
				token(Token::LBracket);
			}
			break;

			case ']':
			{
				token(Token::RBracket);
			}
			break;

			case '{':
			{
				token(Token::LBrace);
			}
			break;

			case '}':
			{
				token(Token::RBrace);
			}
			break;

			case '/':
			{
				if (nextChar == '/')
				{
					tokenStart(Token::Comment);
					advance();
					d_currentState = SingleLineComment;
				}
				else if (nextChar == '*')
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
			// case '$':
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

			// carriage return is stupid
			// TODO add this to every multichar token
			// case '\r':
			// {
			// 	throw Error("", d_currentPos, d_currentPos.nextColumn(), Error::Type::Error, "illigal carriage return");
			// }
			// break;

			default:
			{
				throw Error("", d_currentPos, d_currentPos.nextColumn(), Error::Type::Error, "unexpected character");
			}
			break;
		}
		END; // case Start

		case Finished:
		{
			return d_currentToken;
		}
		break; // Finished

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
				tokenEnd();
			}
			break;
		}
		END; // Symbol

		SWITCH_CURRENT_CHAR(Operator)
		{
			case OPERATOR:
			{
				// continue parsing
			}
			break;

			default:
			{
				tokenEnd();
			}
			break;
		}
		END; // Operator

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
				d_currentState = NumberLiteralFaction;
			}
			break;

			default:
			{
				tokenEnd();
			}
			break;
		}
		END; // NumberLiteral

		SWITCH_CURRENT_CHAR(NumberLiteralFaction)
		{
			case DIGIT:
			{
				// continue parsing
			}
			break;

			default:
			{
				tokenEnd();
			}
			break;
		}
		END; // NumberLiteralFaction

		SWITCH_CURRENT_CHAR(StringLiteral)
		{
			case '\t':
			case '\n':
			{
				// TODO multiline string literals (see Swift)
				throw Error("", d_currentPos, d_currentPos.nextColumn(), Error::Type::Error, "string literals cannot contain new lines or tabs");
			}
			break;

			case '\\':
			{
				d_currentState = StringLiteralEscape;
			}
			break;

			case '"':
			{
				d_currentState = TokenEnd;
			}
			break;

			default:
			{
				// continue parsing
			}
			break;
		}
		END; // StringLiteral

		SWITCH_CURRENT_CHAR(StringLiteralEscape)
		{
			case '\\':
			case '"':
			case 't':
			case 'n':
			{
				throw Error("", d_currentPos, d_currentPos.nextColumn(), Error::Type::Error, "unknown escape sequence");
			}
			break;

			default:
			{
				d_currentState = StringLiteral;
			}
			break;
		}
		END; // StringLiteralEscape

		SWITCH_CURRENT_CHAR(SingleLineComment)
		{
			case '\n':
			{
				// the new line will be processed on the next call so we
				// dont need to set newLine = true
				tokenEnd();
			}
			break;

			default:
			{
				// continue parsing
			}
			break;
		}
		END; // SingleLineComment

		SWITCH_CURRENT_CHAR(MultiLineComment)
		{
			case '\n':
			{
				newLine = true;
			}
			break;

			case '/':
			{
				if (nextChar == '*')
				{
					advance();
					commentNestLevel += 1;
				}
			}
			break;

			case '*':
			{
				if (nextChar == '/')
				{
					advance();
					commentNestLevel -= 1;
					if (commentNestLevel == 0)
					{
						d_currentState = TokenEnd;
					}
					else if (commentNestLevel < 0)
					{
						throw Error("", d_currentPos, d_currentPos.nextColumn(), Error::Type::Error, "unpaired multiline comment ending");
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
		END; // MultiLineComment

		case TokenEnd:
		{
			tokenEnd();
		}
		break; // TokenEnd
		} // switch (d_currentState)

		d_currentIdx += 1;
		// reverting the line info aware position in the case where
		// we ar at the begging of a line is non trivial since we
		// don't know how long the previous line was
		// we store the previous position in order to revert to it
		d_previousPos = d_currentPos;
		if (newLine)
		{
			d_currentPos.line += 1;
			d_currentPos.column = 0;
		}
		else
		{
			d_currentPos.column += 1;
		}
	}

	d_currentToken.d_tag = Token::Eof;
	d_currentToken.d_start = d_currentPos;
	d_currentToken.d_end = d_currentPos;
  // d_currentToken.d_start.column -= 1; // compensate for trailing " "
  // d_currentToken.d_end.column -= 1;
	d_currentToken.d_text = "Eof";
	return d_currentToken;
}

void Tokenizer::advance()
{
	d_currentIdx += 1;
	d_currentPos = d_currentPos.nextColumn();
}

void Tokenizer::tokenStart(Token::Tag tag)
{
	d_currentTokenStartIdx = d_currentIdx;

	d_currentToken.d_tag = tag;
	d_currentToken.d_start = d_currentPos;
	d_currentState = static_cast<State>(tag);
}

void Tokenizer::token(Token::Tag tag)
{
	d_currentToken.d_tag = tag;
	d_currentToken.d_start = d_currentPos;
	d_currentToken.d_end = d_currentPos.nextColumn();
	d_currentToken.d_text = Intern::string(d_sourceText.c_str() + d_currentIdx, 1);
	d_currentState = Finished;
}

void Tokenizer::tokenEnd()
{
	d_currentToken.d_end = d_currentPos;
	d_currentToken.d_text = Intern::string(d_sourceText.c_str() + d_currentTokenStartIdx, d_currentIdx - d_currentTokenStartIdx);

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
			// .Case("null", Token::KwNull)
			// .Case("true", Token::KwTrue)
			// .Case("false", Token::KwFalse)
			// .Case("undefined", Token::KwUndefined)
			// .Case("unreachable", Token::KwUnreachable)
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
			// .Case("using", Token::KwUsing)
			// .Case("as", Token::KwAs)
			// .Case("in", Token::KwIn)
			.Default(d_currentToken.d_tag);
	}

	if (d_currentToken.d_tag == Token::Operator)
	{
		// TODO checks and suggestions with lexicographic distance
	}

	d_currentIdx -= 1;
	d_currentPos = d_previousPos;
	d_currentState = Finished;
}