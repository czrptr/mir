#pragma once

#include <parsing/Token.h>
#include <fmt/format.h>

// TODO use this to print operator precedence table with command line option
struct Operator final
{
  friend struct Parser;

public:

  // TODO
  //  add wrapping and saturating arithmetic
  //  add all assignment operators *=, += etc
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

    Not,        // !a
    UnaryMinus, // -a
    UnaryPlus,  // +a
    BitNot,     // ~!a
    PtrTo,      // ^a

    /* ============ Precedence 4 ============ */

    Mul,           // a * b
    Div,           // a / b
    Mod,           // a % b
    OrOr_ErrorSet, // a || b

    /* ============ Precedence 5 ============ */

    Add, // a + b
    Sub, // a - b

    /* ============ Precedence 6 ============ */

    BitShr, // a ~> b
    BitShl, // a ~< b

    /* ============ Precedence 7 ============ */

    BitAnd, // a ~& b
    BitOr,  // a ~| b
    BitXor, // a ~^ b
    Orelse, // a orelse b
    Catch,  // a catch [|err|] b

    /* ============ Precedence 8 ============ */

    EqEq,  // a == b
    NotEq, // a != b
    Ge,    // a > b
    Le,    // a < b
    GeEq,  // a >= b
    LeEq,  // a <= b

    /* ============ Precedence 9 ============ */

    AndAnd, // a && b

    /* ============ Precedence 10 ============ */

    OrOr, // a || b

    /* ============ Precedence 11 ============ */

    DotDot, // a .. b
    Eq,     // a = b
  };

  enum class Associativity
  {
    None,
    LeftToRight,
    RightToLeft,
  };
  // TODO precedence, chainability, associativity

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
  bool chainability() const;

  static size_t precedence(Operator::Tag tag);
  static Associativity associativity(Operator::Tag tag);
  static bool chainability(Operator::Tag tag);
};

template<>
struct fmt::formatter<Operator::Tag>
{
  bool asString { false };
  fmt::formatter<fmt::string_view> underlying_formatter;

  constexpr auto parse(fmt::format_parse_context& ctx)
  {
    auto
      it = ctx.begin(),
      end = ctx.end();

    if (it != end)
    {
      if (*it == 's')
      {
        asString = true;
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
    if (asString)
    {
      switch (tag)
      {
      case Operator::Tag::Dot: name = "Dot"; break;
      case Operator::Tag::Opt: name = "Opt"; break;
      case Operator::Tag::PtrDeref: name = "PtrDeref"; break;
      case Operator::Tag::Not_ErrorUnion: name = "Not_ErrorUnion"; break;
      case Operator::Tag::Not: name = "Not"; break;
      case Operator::Tag::UnaryMinus: name = "UnaryMinus"; break;
      case Operator::Tag::UnaryPlus: name = "UnaryPlus"; break;
      case Operator::Tag::BitNot: name = "BitNot"; break;
      case Operator::Tag::PtrTo: name = "PtrTo"; break;
      case Operator::Tag::Mul: name = "Mul"; break;
      case Operator::Tag::Div: name = "Div"; break;
      case Operator::Tag::Mod: name = "Mod"; break;
      case Operator::Tag::OrOr_ErrorSet: name = "OrOr_ErrorSet"; break;
      case Operator::Tag::Add: name = "Add"; break;
      case Operator::Tag::Sub: name = "Sub"; break;
      case Operator::Tag::BitShr: name = "BitShr"; break;
      case Operator::Tag::BitShl: name = "BitShl"; break;
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
      case Operator::Tag::AndAnd: name = "AndAnd"; break;
      case Operator::Tag::OrOr: name = "OrOr"; break;
      case Operator::Tag::DotDot: name = "DotDot"; break;
      case Operator::Tag::Eq: name = "Eq"; break;
      }
      return underlying_formatter.format(name, ctx);
    }

    switch (tag)
    {
      case Operator::Tag::Dot: name = "."; break;
      case Operator::Tag::Opt: name = "?"; break;
      case Operator::Tag::PtrDeref: name = "^"; break;
      case Operator::Tag::Not_ErrorUnion: name = "!"; break;
      case Operator::Tag::Not: name = "!"; break;
      case Operator::Tag::UnaryMinus: name = "-"; break;
      case Operator::Tag::UnaryPlus: name = "+"; break;
      case Operator::Tag::BitNot: name = "~!"; break;
      case Operator::Tag::PtrTo: name = "^"; break;
      case Operator::Tag::Mul: name = "*"; break;
      case Operator::Tag::Div: name = "/"; break;
      case Operator::Tag::Mod: name = "%"; break;
      case Operator::Tag::OrOr_ErrorSet: name = "||"; break;
      case Operator::Tag::Add: name = "+"; break;
      case Operator::Tag::Sub: name = "-"; break;
      case Operator::Tag::BitShr: name = "~>"; break;
      case Operator::Tag::BitShl: name = "~<"; break;
      case Operator::Tag::BitAnd: name = "~&"; break;
      case Operator::Tag::BitOr: name = "~|"; break;
      case Operator::Tag::BitXor: name = "~^"; break;
      case Operator::Tag::Orelse: name = "orelse"; break;
      case Operator::Tag::Catch: name = "catch"; break;
      case Operator::Tag::EqEq: name = "=="; break;
      case Operator::Tag::NotEq: name = "=!"; break;
      case Operator::Tag::Ge: name = ">"; break;
      case Operator::Tag::Le: name = "<"; break;
      case Operator::Tag::GeEq: name = ">="; break;
      case Operator::Tag::LeEq: name = "<="; break;
      case Operator::Tag::AndAnd: name = "&&"; break;
      case Operator::Tag::OrOr: name = "||"; break;
      case Operator::Tag::DotDot: name = ".."; break;
      case Operator::Tag::Eq: name = "="; break;
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
    case Operator::Associativity::None: name = "None"; break;
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