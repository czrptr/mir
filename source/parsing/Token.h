#pragma once

#include <parsing/Position.h>

#include <compare>
#include <string_view>
#include <fmt/format.h>

struct Token final
{
  // Friends
  friend struct Tokenizer;

public:
  // Types
  enum Tag
  {
    Symbol = 0,
    Operator = 1,
    NumberLiteral = 2,
    StringLiteral = 3,
    // runes
    Dot,
    Comma,
    Colon,
    Semicolon,
    LParen,
    RParen,
    LBracket,
    RBracket,
    LBrace,
    RBrace,
    // keywords
    KwPub,
    KwLet,
    KwMut,
    KwComptime,
    KwStruct,
    KwEnum,
    KwUnion,
    KwFn,
    // KwNull,        // builtin value
    // KwTrue,        // builtin value
    // KwFalse,       // builtin value
    // KwUndefined,   // builtin value
    // KwUnreachable, // builtin value
    KwTry,
    KwCatch,
    KwOrelse,
    KwIf,
    KwElse,
    KwSwitch,
    KwLoop,
    KwReturn,
    KwBreak,
    KwContinue,
    KwDefer,
    KwImport,
    // KwUsing, // undecided
    // KwAs,    // undecided
    // KwIn,    // undecided
    // KwErrDefer,
    // KwInline,
    // KwAsync,
    // KwAwait,
    // KwSuspend,
    // KwResume,
    // KwNosuspend,
    // KwAsm,
    // KwAllowzero,
    // KwTest,
    // KwMacro,
    Comment,
    Eof
  };

private:
  // Data
  Tag d_tag;
  Position d_start;
  Position d_end;
  std::string_view d_text;

public:
  // Constructors
  constexpr Token() noexcept = default;

  constexpr Token(
    Tag tag,
    Position start,
    Position end,
    std::string_view text) noexcept
  : d_tag(tag)
  , d_start(start)
  , d_end(end)
  , d_text(text)
  {}

public:
  // Methods
  Tag tag() const noexcept { return d_tag; }
  Position start() const noexcept { return d_start; }
  Position end() const noexcept { return d_end; }
  std::string_view text() const noexcept { return d_text; }

  // Operators
  constexpr std::strong_ordering operator<=>(Token const&) const noexcept = default;
};

template<>
struct fmt::formatter<Token::Tag> : formatter<string_view>
{
  template<typename FormatContext>
  auto format(Token::Tag tag, FormatContext& ctx)
  {
    std::string_view name = "TOKEN_TAG_INVALID";
    switch (tag)
    {
    case Token::Tag::Symbol: name = "Symbol"; break;
    case Token::Tag::Operator: name = "Operator"; break;
    case Token::Tag::NumberLiteral: name = "NumberLiteral"; break;
    case Token::Tag::StringLiteral: name = "StringLiteral"; break;
    case Token::Tag::Dot: name = "Dot"; break;
    case Token::Tag::Comma: name = "Comma"; break;
    case Token::Tag::Colon: name = "Colon"; break;
    case Token::Tag::Semicolon: name = "Semicolon"; break;
    case Token::Tag::LParen: name = "LParen"; break;
    case Token::Tag::RParen: name = "RParen"; break;
    case Token::Tag::LBracket: name = "LBracket"; break;
    case Token::Tag::RBracket: name = "RBracket"; break;
    case Token::Tag::LBrace: name = "LBrace"; break;
    case Token::Tag::RBrace: name = "RBrace"; break;
    case Token::Tag::KwPub: name = "KwPub"; break;
    case Token::Tag::KwLet: name = "KwLet"; break;
    case Token::Tag::KwMut: name = "KwMut"; break;
    case Token::Tag::KwComptime: name = "KwComptime"; break;
    case Token::Tag::KwStruct: name = "KwStruct"; break;
    case Token::Tag::KwEnum: name = "KwEnum"; break;
    case Token::Tag::KwUnion: name = "KwUnion"; break;
    case Token::Tag::KwFn: name = "KwFn"; break;
    // case Token::Tag::KwNull: name = "KwNull"; break;
    // case Token::Tag::KwTrue: name = "KwTrue"; break;
    // case Token::Tag::KwFalse: name = "KwFalse"; break;
    // case Token::Tag::KwUndefined: name = "KwUndefined"; break;
    // case Token::Tag::KwUnreachable: name = "KwUnreachable"; break;
    case Token::Tag::KwTry: name = "KwTry"; break;
    case Token::Tag::KwCatch: name = "KwCatch"; break;
    case Token::Tag::KwOrelse: name = "KwOrelse"; break;
    case Token::Tag::KwIf: name = "KwIf"; break;
    case Token::Tag::KwElse: name = "KwElse"; break;
    case Token::Tag::KwSwitch: name = "KwSwitch"; break;
    case Token::Tag::KwLoop: name = "KwLoop"; break;
    case Token::Tag::KwReturn: name = "KwReturn"; break;
    case Token::Tag::KwBreak: name = "KwBreak"; break;
    case Token::Tag::KwContinue: name = "KwContinue"; break;
    case Token::Tag::KwDefer: name = "KwDefer"; break;
    case Token::Tag::KwImport: name = "KwImport"; break;
    // case Token::Tag::KwUsing: name = "KwUsing"; break;
    // case Token::Tag::KwAs: name = "KwAs"; break;
    // case Token::Tag::KwIn: name = "KwIn"; break;
    // case Token::Tag::KwErrDefer: name = "KwErrDefer"; break;
    // case Token::Tag::KwInline: name = "KwInline"; break;
    // case Token::Tag::KwAsync: name = "KwAsync"; break;
    // case Token::Tag::KwAwait: name = "KwAwait"; break;
    // case Token::Tag::KwSuspend: name = "KwSuspend"; break;
    // case Token::Tag::KwResume: name = "KwResume"; break;
    // case Token::Tag::KwNosuspend: name = "KwNosuspend"; break;
    // case Token::Tag::KwAsm: name = "KwAsm"; break;
    // case Token::Tag::KwAllowzero: name = "KwAllowzero"; break;
    // case Token::Tag::KwTest: name = "KwTest"; break;
    // case Token::Tag::KwMacro: name = "KwMacro"; break;
    case Token::Tag::Comment: name = "Comment"; break;
    case Token::Tag::Eof: name = "Eof"; break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};

template<>
struct fmt::formatter<Token>
{
  bool debug { false };
  bool shortened { false };

  constexpr auto parse(fmt::format_parse_context& ctx)
  {
    auto
      it = ctx.begin(),
      end = ctx.end();

    if (it != end && *it == 'd')
    {
      debug = true;
      it += 1;
    }

    if (it != end && *it == 's')
    {
      if (!debug) throw format_error("invalid format");

      shortened = true;
      it += 1;
    }

    if (it != end && *it != '}')
    {
      throw format_error("invalid format");
    }

    return it;
  }

  template<typename FormatContext>
  auto format(Token tok, FormatContext& ctx)
  {
    if (debug)
    {
      if (shortened)
      {
        return fmt::format_to(
          ctx.out(),
          "Token{{{0}, {1:ds}, {2:ds}, \"{3}\"}}",
          tok.tag(), tok.start(), tok.end(), tok.text());
      }
      return fmt::format_to(
        ctx.out(),
        "Token{{tag = {0}, start = {1:d}, end = {2:d}, text = \"{3}\"}}",
        tok.tag(), tok.start(), tok.end(), tok.text());
    }
    return fmt::format_to(
      ctx.out(),
      "{0}-{1}-{2}-{3}",
      tok.tag(), tok.start(), tok.end(), tok.text());
  }
};