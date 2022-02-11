#pragma once

#include <parsing/Token.h>
#include <fmt/format.h>

// TODO
//   use this to print operator precedence table with command line option
//   make constexpr
struct Operator final
{
  friend struct Parser;

public:

  // TODO infer, comptime?
  enum Tag
  {
    /* === Precedence 0 ============ */

    Dot,           // a.b
    Opt,           // a?
    PtrDeref,      // a^
    // FnCall      // a()
    // ArrayAccess // a[]

    /* ============ Precedence 1 ============ */

    Not_ErrorUnion, // a!b

    /* ============ Precedence 2 ============ */

    // TypeInit // a{}

    /* ============ Precedence 3 ============ */

    Try, // try a

    /* ============ Precedence 4 ============ */

    Not,           // not a
    UnaryMinus,    // -a
    UnaryMinusMod, // -%a
    UnaryPlus,     // +a
    BitNot,        // !a
    PtrTo,         // ^a

    /* ============ Precedence 5 ============ */

    Mul,           // a * b
    MulMod,        // a *% b
    MulBar,        // a *| b
    Div,           // a / b
    Mod,           // a % b
    OrOr_ErrorSet, // a || b

    /* ============ Precedence 6 ============ */

    Add,    // a + b
    AddMod, // a +% b
    AddBar, // a +| b
    Sub,    // a - b
    SubMod, // a -% b
    SubBar, // a -| b

    /* ============ Precedence 7 ============ */

    BitShr,    // a >> b
    BitRor,    // a >% b
    BitShl,    // a << b
    BitRol,    // a <% b
    BitShlBar, // a <| b

    /* ============ Precedence 8 ============ */

    BitAnd, // a & b
    BitOr,  // a | b
    BitXor, // a ~ b
    Orelse, // a orelse b             // TODO move
    Catch,  // a catch [|err|] b      //   these

    /* ============ Precedence 9 ============ */

    EqEq,  // a == b
    NotEq, // a != b
    Ge,    // a > b
    Le,    // a < b
    GeEq,  // a >= b
    LeEq,  // a <= b

    /* ============ Precedence 10 ============ */

    And, // a and b

    /* ============ Precedence 11 ============ */

    Or, // a or b

    /* ============ Precedence 12 ============ */

    DotDot, // a..[b]

    /* ============ Precedence 13 ============ */

    Eq,          // a = b
    MulEq,       // a *= b
    MulModEq,    // a *%= b
    MulBarEq,    // a *|= b
    MulDivEq,    // a /= b
    ModEq,       // a %= b
    AddEq,       // a += b
    AddModEq,    // a +%= b
    AddBarEq,    // a +|= b
    SubEq,       // a -= b
    SubModEq,    // a -%= b
    SubBarEq,    // a -|= b
    BitShrEq,    // a >>= b
    BitRorEq,    // a >%= b
    BitShlEq,    // a <<= b
    BitRolEq,    // a <%= b
    BitShlBarEq, // a <|= b
    BitAndEq,    // a &= b
    BitOrEq,     // a |= b
    BitXorEq,    // a ~= b

    /* ============ Precedence 14 ============ */

    Return,   // return [a]
    Break,    // break [:lbl] [a]
    Continue, // continue [:lbl]
    Defer,    // defer a
  };

  enum class Associativity
  {
    LeftToRight,
    RightToLeft,
  };

private:
  Tag d_tag;
  Position d_start;
  Position d_end;
  std::string_view d_text;

  // to be used in parser
  void setTag(Operator::Tag tag)
  {
    d_tag = tag;
  }

public:
  explicit Operator(Token token);

  Tag tag() const noexcept { return d_tag; }
  Position start() const noexcept { return d_start; }
  Position end() const noexcept { return d_end; }
  std::string_view text() const noexcept { return d_text; }

  size_t precedence() const;
  Associativity associativity() const;
  bool chainable() const;

  static size_t precedence(Operator::Tag tag);
  static Associativity associativity(Operator::Tag tag);
  static bool chainable(Operator::Tag tag);

  static std::string validate(std::string_view text);
};

template<>
struct fmt::formatter<Operator::Tag>
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
  auto format(Operator::Tag tag, FormatContext& ctx)
  {
    std::string_view name = "OPERATOR_TAG_INVALID";
    if (debug)
    {
      switch (tag)
      {
      case Operator::Tag::Dot: name = "Dot"; break;
      case Operator::Tag::Opt: name = "Opt"; break;
      case Operator::Tag::PtrDeref: name = "PtrDeref"; break;
      case Operator::Tag::Not_ErrorUnion: name = "Not_ErrorUnion"; break;
      case Operator::Tag::Try: name = "Try"; break;
      case Operator::Tag::Not: name = "Not"; break;
      case Operator::Tag::UnaryMinus: name = "UnaryMinus"; break;
      case Operator::Tag::UnaryMinusMod: name = "UnaryMinusMod"; break;
      case Operator::Tag::UnaryPlus: name = "UnaryPlus"; break;
      case Operator::Tag::BitNot: name = "BitNot"; break;
      case Operator::Tag::PtrTo: name = "PtrTo"; break;
      case Operator::Tag::Mul: name = "Mul"; break;
      case Operator::Tag::MulMod: name = "MulMod"; break;
      case Operator::Tag::MulBar: name = "MulBar"; break;
      case Operator::Tag::Div: name = "Div"; break;
      case Operator::Tag::Mod: name = "Mod"; break;
      case Operator::Tag::OrOr_ErrorSet: name = "OrOr_ErrorSet"; break;
      case Operator::Tag::Add: name = "Add"; break;
      case Operator::Tag::AddMod: name = "AddMod"; break;
      case Operator::Tag::AddBar: name = "AddBar"; break;
      case Operator::Tag::Sub: name = "Sub"; break;
      case Operator::Tag::BitShr: name = "BitShr"; break;
      case Operator::Tag::BitRor: name = "BitRor"; break;
      case Operator::Tag::BitShl: name = "BitShl"; break;
      case Operator::Tag::BitRol: name = "BitRol"; break;
      case Operator::Tag::BitShlBar: name = "BitShlBar"; break;
      case Operator::Tag::BitAnd: name = "BitAnd"; break;
      case Operator::Tag::BitOr: name = "BitOr"; break;
      case Operator::Tag::BitXor: name = "BitXor"; break;
      case Operator::Tag::Orelse: name = "Orelse"; break;
      case Operator::Tag::Catch: name = "Catch"; break;
      case Operator::Tag::EqEq: name = "EqEq"; break;
      case Operator::Tag::NotEq: name = "NotEq"; break;
      case Operator::Tag::Ge: name = "Ge"; break;
      case Operator::Tag::Le: name = "Le"; break;
      case Operator::Tag::GeEq: name = "GeEq"; break;
      case Operator::Tag::LeEq: name = "LeEq"; break;
      case Operator::Tag::And: name = "AndAnd"; break;
      case Operator::Tag::Or: name = "OrOr"; break;
      case Operator::Tag::DotDot: name = "DotDot"; break;
      case Operator::Tag::Eq: name = "Eq"; break;
      case Operator::Tag::MulEq: name = "MulEq"; break;
      case Operator::Tag::MulModEq: name = "MulModEq"; break;
      case Operator::Tag::MulBarEq: name = "MulBarEq"; break;
      case Operator::Tag::MulDivEq: name = "MulDivEq"; break;
      case Operator::Tag::ModEq: name = "ModEq"; break;
      case Operator::Tag::AddEq: name = "AddEq"; break;
      case Operator::Tag::AddModEq: name = "AddModEq"; break;
      case Operator::Tag::AddBarEq: name = "AddBarEq"; break;
      case Operator::Tag::SubEq: name = "SubEq"; break;
      case Operator::Tag::SubModEq: name = "SubModEq"; break;
      case Operator::Tag::SubBarEq: name = "SubBarEq"; break;
      case Operator::Tag::BitShrEq: name = "BitShrEq"; break;
      case Operator::Tag::BitRorEq: name = "BitRorEq"; break;
      case Operator::Tag::BitShlEq: name = "BitShlEq"; break;
      case Operator::Tag::BitRolEq: name = "BitRolEq"; break;
      case Operator::Tag::BitShlBarEq: name = "BitShlBarEq"; break;
      case Operator::Tag::BitAndEq: name = "BitAndEq"; break;
      case Operator::Tag::BitOrEq: name = "BitOrEq"; break;
      case Operator::Tag::BitXorEq: name = "BitXorEq"; break;
      case Operator::Tag::Return: name = "Return"; break;
      case Operator::Tag::Break: name = "Break"; break;
      case Operator::Tag::Continue: name = "Continue"; break;
      case Operator::Tag::Defer: name = "Defer"; break;
      }
      return underlying_formatter.format(name, ctx);
    }

    switch (tag)
    {
      case Operator::Tag::Dot: name = "."; break;
      case Operator::Tag::Opt: name = "?"; break;
      case Operator::Tag::PtrDeref: name = "^"; break;
      case Operator::Tag::Not_ErrorUnion: name = "!"; break;
      case Operator::Tag::Try: name = "try"; break;
      case Operator::Tag::Not: name = "not"; break;
      case Operator::Tag::UnaryMinus: name = "-"; break;
      case Operator::Tag::UnaryMinusMod: name = "-%"; break;
      case Operator::Tag::UnaryPlus: name = "+"; break;
      case Operator::Tag::BitNot: name = "!"; break;
      case Operator::Tag::PtrTo: name = "^"; break;
      case Operator::Tag::Mul: name = "*"; break;
      case Operator::Tag::MulMod: name = "*%"; break;
      case Operator::Tag::MulBar: name = "*|"; break;
      case Operator::Tag::Div: name = "/"; break;
      case Operator::Tag::Mod: name = "%"; break;
      case Operator::Tag::OrOr_ErrorSet: name = "||"; break;
      case Operator::Tag::Add: name = "+"; break;
      case Operator::Tag::AddMod: name = "+%"; break;
      case Operator::Tag::AddBar: name = "+|"; break;
      case Operator::Tag::Sub: name = "-"; break;
      case Operator::Tag::SubMod: name = "-%"; break;
      case Operator::Tag::SubBar: name = "-|"; break;
      case Operator::Tag::BitShr: name = ">>"; break;
      case Operator::Tag::BitRor: name = ">%"; break;
      case Operator::Tag::BitShl: name = "<<"; break;
      case Operator::Tag::BitRol: name = "<%"; break;
      case Operator::Tag::BitShlBar: name = "<|"; break;
      case Operator::Tag::BitAnd: name = "&"; break;
      case Operator::Tag::BitOr: name = "|"; break;
      case Operator::Tag::BitXor: name = "~"; break;
      case Operator::Tag::Orelse: name = "orelse"; break;
      case Operator::Tag::Catch: name = "catch"; break;
      case Operator::Tag::EqEq: name = "=="; break;
      case Operator::Tag::NotEq: name = "!="; break;
      case Operator::Tag::Ge: name = ">"; break;
      case Operator::Tag::Le: name = "<"; break;
      case Operator::Tag::GeEq: name = ">="; break;
      case Operator::Tag::LeEq: name = "<="; break;
      case Operator::Tag::And: name = "and"; break;
      case Operator::Tag::Or: name = "or"; break;
      case Operator::Tag::DotDot: name = ".."; break;
      case Operator::Tag::Eq: name = "="; break;
      case Operator::Tag::MulEq: name = "*="; break;
      case Operator::Tag::MulModEq: name = "*%="; break;
      case Operator::Tag::MulBarEq: name = "*|="; break;
      case Operator::Tag::MulDivEq: name = "/="; break;
      case Operator::Tag::ModEq: name = "%="; break;
      case Operator::Tag::AddEq: name = "+="; break;
      case Operator::Tag::AddModEq: name = "+%="; break;
      case Operator::Tag::AddBarEq: name = "+|="; break;
      case Operator::Tag::SubEq: name = "-="; break;
      case Operator::Tag::SubModEq: name = "-%="; break;
      case Operator::Tag::SubBarEq: name = "-|="; break;
      case Operator::Tag::BitShrEq: name = ">>="; break;
      case Operator::Tag::BitRorEq: name = ">%="; break;
      case Operator::Tag::BitShlEq: name = "<<="; break;
      case Operator::Tag::BitRolEq: name = "<%="; break;
      case Operator::Tag::BitShlBarEq: name = "<|="; break;
      case Operator::Tag::BitAndEq: name = "&="; break;
      case Operator::Tag::BitOrEq: name = "|="; break;
      case Operator::Tag::BitXorEq: name = "~="; break;
      case Operator::Tag::Return: name = "return"; break;
      case Operator::Tag::Break: name = "break"; break;
      case Operator::Tag::Continue: name = "continue"; break;
      case Operator::Tag::Defer: name = "defer"; break;
    }
    return underlying_formatter.format(name, ctx);
  }
};

template<>
struct fmt::formatter<Operator::Associativity> : fmt::formatter<fmt::string_view>
{
  template<typename FormatContext>
  auto format(Operator::Associativity assoc, FormatContext& ctx)
  {
    std::string_view name = "OPERATOR_ASSOCIATIVITY_INVALID";
    switch (assoc)
    {
    case Operator::Associativity::LeftToRight: name = "LeftToRight"; break;
    case Operator::Associativity::RightToLeft: name = "RightToLeft"; break;
    }
    return fmt::formatter<string_view>::format(name, ctx);
  }
};

template<>
struct fmt::formatter<Operator>
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
  auto format(Operator op, FormatContext& ctx)
  {
    if (debug)
    {
      // TODO print precedence, associativity, chainability
      if (shortened)
      {
        return fmt::format_to(
          ctx.out(),
          "Operator{{{0:s}, {1:ds}, {2:ds}, \"{3}\"}}",
          op.tag(), op.start(), op.end(), op.text());
      }
      return fmt::format_to(
        ctx.out(),
        "Operator{{tag = {0:s}, start = {1:d}, end = {2:d}, text = \"{3}\"}}",
        op.tag(), op.start(), op.end(), op.text());
    }
    return fmt::format_to(
      ctx.out(),
      "{0}",
      op.tag());
  }
};