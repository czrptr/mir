#include "parsing/Operator.h"

#include <cassert>

Operator::Operator(Token token)
  : d_start(token.start())
  , d_end(token.end())
  , d_text(token.text())
{}

size_t Operator::precedence() const
{
  return precedence(d_tag);
}

bool Operator::chainable() const
{
  return chainable(d_tag);
}

Operator::Associativity Operator::associativity() const
{
  return associativity(d_tag);
}

size_t Operator::precedence(Operator::Tag tag)
{
  auto const value = static_cast<size_t>(tag);

  // table of operator thresholds
  // and respective precedence values
  static constexpr size_t thresholds[] = {
    PtrDeref, 0,
    Not_ErrorUnion, 1,
    PtrTo, 3,
    OrOr_ErrorSet, 4,
    Sub, 5,
    BitShl, 6,
    Catch, 7,
    LeEq, 8,
    And, 9,
    Or, 10,
    Eq, 11,
  };
  // ensure table is well formed
  static_assert(sizeof(thresholds) % 2 == 0);
  static constexpr size_t tableLength = sizeof(thresholds) / 2 / sizeof(thresholds[0]);

  for (size_t i = 0; i < tableLength; i += 1)
  {
    if (value <= thresholds[2 * i])
    {
      return thresholds[2 * i + 1];
    }
  }
  assert(false); // unreachable
}

Operator::Associativity Operator::associativity(Operator::Tag tag)
{
  // use LeftToRight as None because parsing LeftToRight doesn't
  // require a recursive call
  constexpr auto None = Associativity::LeftToRight;

  switch (tag)
  {
  case Operator::Tag::Dot: return Associativity::LeftToRight;
  case Operator::Tag::Opt: return Associativity::LeftToRight;
  case Operator::Tag::PtrDeref: return Associativity::LeftToRight;
  case Operator::Tag::Not_ErrorUnion: return Associativity::RightToLeft; // !!
  case Operator::Tag::Not: return Associativity::RightToLeft;
  case Operator::Tag::UnaryMinus: return Associativity::RightToLeft;
  case Operator::Tag::UnaryPlus: return Associativity::RightToLeft;
  case Operator::Tag::BitNot: return Associativity::RightToLeft;
  case Operator::Tag::PtrTo: return None; // unchainable
  case Operator::Tag::Mul: return Associativity::LeftToRight;
  case Operator::Tag::Div: return Associativity::LeftToRight;
  case Operator::Tag::Mod: return Associativity::LeftToRight;
  case Operator::Tag::OrOr_ErrorSet: return Associativity::LeftToRight;
  case Operator::Tag::Add: return Associativity::LeftToRight;
  case Operator::Tag::Sub: return Associativity::LeftToRight;
  case Operator::Tag::BitShr: return Associativity::LeftToRight;
  case Operator::Tag::BitShl: return Associativity::LeftToRight;
  case Operator::Tag::BitAnd: return Associativity::LeftToRight;
  case Operator::Tag::BitOr: return Associativity::LeftToRight;
  case Operator::Tag::BitXor: return Associativity::LeftToRight;
  case Operator::Tag::Orelse: return Associativity::RightToLeft; // !!
  case Operator::Tag::Catch: return Associativity::RightToLeft;  // !!
  case Operator::Tag::EqEq: return None;   // unchainable
  case Operator::Tag::NotEq: return None;  // unchainable
  case Operator::Tag::Ge: return None;     // unchainable
  case Operator::Tag::Le: return None;     // unchainable
  case Operator::Tag::GeEq: return None;   // unchainable
  case Operator::Tag::LeEq: return None;   // unchainable
  case Operator::Tag::And: return Associativity::LeftToRight;
  case Operator::Tag::Or: return Associativity::LeftToRight;
  case Operator::Tag::DotDot: return None; // unchainable
  case Operator::Tag::Eq: return None;     // unchainable
  }
  assert(false); // unreachable
}

bool Operator::chainable(Operator::Tag tag)
{
    switch (tag)
  {
  case Operator::Tag::Dot: return true;
  case Operator::Tag::Opt: return true;
  case Operator::Tag::PtrDeref: return true;
  case Operator::Tag::Not_ErrorUnion: return true;
  case Operator::Tag::Not: return true;         // TODO make false
  case Operator::Tag::UnaryMinus: return true;  // TODO make false
  case Operator::Tag::UnaryPlus: return true;   // TODO make false
  case Operator::Tag::BitNot: return true;      // TODO make false
  case Operator::Tag::PtrTo: return false;
  case Operator::Tag::Mul: return true;
  case Operator::Tag::Div: return true;
  case Operator::Tag::Mod: return true;
  case Operator::Tag::OrOr_ErrorSet: return true;
  case Operator::Tag::Add: return true;
  case Operator::Tag::Sub: return true;
  case Operator::Tag::BitShr: return true;
  case Operator::Tag::BitShl: return true;
  case Operator::Tag::BitAnd: return true;
  case Operator::Tag::BitOr: return true;
  case Operator::Tag::BitXor: return true;
  case Operator::Tag::Orelse: return true;
  case Operator::Tag::Catch: return true;
  case Operator::Tag::EqEq: return false;
  case Operator::Tag::NotEq: return false;
  case Operator::Tag::Ge: return false;
  case Operator::Tag::Le: return false;
  case Operator::Tag::GeEq: return false;
  case Operator::Tag::LeEq: return false;
  case Operator::Tag::And: return true;
  case Operator::Tag::Or: return true;
  case Operator::Tag::DotDot: return false;
  case Operator::Tag::Eq: return false;
  }
  assert(false); // unreachable
}