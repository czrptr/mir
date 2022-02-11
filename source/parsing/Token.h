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
    NumberLiteral = 1,
    StringLiteral = 2,
    Operator = 3,
    // "runes"
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
    KwInfer,
    KwTry,
    // KwCatch,  // Operator
    // KwOrelse, // Operator
    KwIf,
    KwElse,
    KwSwitch,
    KwLoop,
    KwReturn,
    KwBreak,
    KwContinue,
    KwDefer,
    KwImport,
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
struct fmt::formatter<Token::Tag>
{
  bool debug { false };
  fmt::formatter<fmt::string_view> underlying_formatter;

  constexpr auto parse(fmt::format_parse_context& ctx)
  {
    auto
      it = ctx.begin(),
      end = ctx.end();

    if (it != end)
    {
      if (*it == 'd')
      {
        debug = true;
        it += 1;
        ctx.advance_to(it);
        return underlying_formatter.parse(ctx);
      }

      return it;
    }
    return it;
  }

  template<typename FormatContext>
  auto format(Token::Tag tag, FormatContext& ctx)
  {
    std::string_view name = "TOKEN_TAG_INVALID";
    if (debug)
    {
      switch (tag)
      {
      case Token::Tag::Symbol: name = "Symbol"; break;
      case Token::Tag::Operator: name = "Operator"; break;
      case Token::Tag::NumberLiteral: name = "NumberLiteral"; break;
      case Token::Tag::StringLiteral: name = "StringLiteral"; break;
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
      case Token::Tag::KwInfer: name = "KwInfer"; break;
      case Token::Tag::KwTry: name = "KwTry"; break;
      case Token::Tag::KwIf: name = "KwIf"; break;
      case Token::Tag::KwElse: name = "KwElse"; break;
      case Token::Tag::KwSwitch: name = "KwSwitch"; break;
      case Token::Tag::KwLoop: name = "KwLoop"; break;
      case Token::Tag::KwReturn: name = "KwReturn"; break;
      case Token::Tag::KwBreak: name = "KwBreak"; break;
      case Token::Tag::KwContinue: name = "KwContinue"; break;
      case Token::Tag::KwDefer: name = "KwDefer"; break;
      case Token::Tag::KwImport: name = "KwImport"; break;
      case Token::Tag::Comment: name = "Comment"; break;
      case Token::Tag::Eof: name = "Eof"; break;
      }
      return underlying_formatter.format(name, ctx);
    }

    switch (tag)
    {
      case Token::Tag::Symbol: name = "identifier"; break;
      case Token::Tag::Operator: name = "operator"; break;
      case Token::Tag::NumberLiteral: name = "number literal"; break;
      case Token::Tag::StringLiteral: name = "string literal"; break;
      case Token::Tag::Comma: name = "','"; break;
      case Token::Tag::Colon: name = "':'"; break;
      case Token::Tag::Semicolon: name = "';'"; break;
      case Token::Tag::LParen: name = "'('"; break;
      case Token::Tag::RParen: name = "')'"; break;
      case Token::Tag::LBracket: name = "'['"; break;
      case Token::Tag::RBracket: name = "']'"; break;
      case Token::Tag::LBrace: name = "'{'"; break;
      case Token::Tag::RBrace: name = "'}'"; break;
      case Token::Tag::KwPub: name = "keyword 'pub'"; break;
      case Token::Tag::KwLet: name = "keyword 'let'"; break;
      case Token::Tag::KwMut: name = "keyword 'mut'"; break;
      case Token::Tag::KwComptime: name = "keyword 'comptime'"; break;
      case Token::Tag::KwStruct: name = "keyword 'struct'"; break;
      case Token::Tag::KwEnum: name = "keyword 'enum'"; break;
      case Token::Tag::KwUnion: name = "keyword 'union'"; break;
      case Token::Tag::KwFn: name = "keyword 'fn'"; break;
      case Token::Tag::KwInfer: name = "keyword 'infer'"; break;
      case Token::Tag::KwTry: name = "keyword 'try'"; break;
      case Token::Tag::KwIf: name = "keyword 'if'"; break;
      case Token::Tag::KwElse: name = "keyword 'else'"; break;
      case Token::Tag::KwSwitch: name = "keyword 'switch'"; break;
      case Token::Tag::KwLoop: name = "keyword 'loop'"; break;
      case Token::Tag::KwReturn: name = "keyword 'return'"; break;
      case Token::Tag::KwBreak: name = "keyword 'break'"; break;
      case Token::Tag::KwContinue: name = "keyword 'continue'"; break;
      case Token::Tag::KwDefer: name = "keyword 'defer'"; break;
      case Token::Tag::KwImport: name = "keyword 'import'"; break;
      case Token::Tag::Comment: name = "comment"; break;
      case Token::Tag::Eof: name = "end of file"; break;
    }
    return underlying_formatter.format(name, ctx);
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
      if (!debug) throw fmt::format_error("invalid format");

      shortened = true;
      it += 1;
    }

    if (it != end && *it != '}')
    {
      throw fmt::format_error("invalid format");
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
          "Token{{{0:d}, {1:ds}, {2:ds}, \"{3}\"}}",
          tok.tag(), tok.start(), tok.end(), tok.text());
      }
      return fmt::format_to(
        ctx.out(),
        "Token{{tag = {0:d}, start = {1:d}, end = {2:d}, text = \"{3}\"}}",
        tok.tag(), tok.start(), tok.end(), tok.text());
    }
    return fmt::format_to(
      ctx.out(),
      "{0}-{1}-{2}-{3}",
      tok.tag(), tok.start(), tok.end(), tok.text());
  }
};