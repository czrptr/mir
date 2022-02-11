#include "parsing/Operator.h"

#include <Utils.h>
#include <parsing/Error.h>

#include <map>
#include <limits>
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
    Try, 3,
    PtrTo, 4,
    OrOr_ErrorSet, 5,
    SubBar, 6,
    BitShlBar, 7,
    Catch, 8,
    LeEq, 9,
    And, 10,
    Or, 11,
    DotDot, 12,
    BitXorEq, 13,
    Defer, 14
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
  constexpr auto NoneBecauseUnchainable = Associativity::LeftToRight;

  switch (tag)
  {
  case Operator::Tag::Dot: return Associativity::LeftToRight;
  case Operator::Tag::Opt: return Associativity::LeftToRight;
  case Operator::Tag::PtrDeref: return Associativity::LeftToRight;
  case Operator::Tag::Not_ErrorUnion: return Associativity::RightToLeft; // !!
  case Operator::Tag::Try: return Associativity::RightToLeft; // !!
  case Operator::Tag::Not: return Associativity::RightToLeft;
  case Operator::Tag::UnaryMinus: return Associativity::RightToLeft;
  case Operator::Tag::UnaryMinusMod: return NoneBecauseUnchainable;
  case Operator::Tag::UnaryPlus: return Associativity::RightToLeft;
  case Operator::Tag::BitNot: return Associativity::RightToLeft;
  case Operator::Tag::PtrTo: return NoneBecauseUnchainable;
  case Operator::Tag::Mul: return Associativity::LeftToRight;
  case Operator::Tag::MulMod: return Associativity::LeftToRight;
  case Operator::Tag::MulBar: return Associativity::LeftToRight;
  case Operator::Tag::Div: return Associativity::LeftToRight;
  case Operator::Tag::Mod: return Associativity::LeftToRight;
  case Operator::Tag::OrOr_ErrorSet: return Associativity::LeftToRight;
  case Operator::Tag::Add: return Associativity::LeftToRight;
  case Operator::Tag::AddMod: return Associativity::LeftToRight;
  case Operator::Tag::AddBar: return Associativity::LeftToRight;
  case Operator::Tag::Sub: return Associativity::LeftToRight;
  case Operator::Tag::SubMod: return Associativity::LeftToRight;
  case Operator::Tag::SubBar: return Associativity::LeftToRight;
  case Operator::Tag::BitShr: return Associativity::LeftToRight;
  case Operator::Tag::BitRor: return Associativity::LeftToRight;
  case Operator::Tag::BitShl: return Associativity::LeftToRight;
  case Operator::Tag::BitRol: return Associativity::LeftToRight;
  case Operator::Tag::BitShlBar: return Associativity::LeftToRight;
  case Operator::Tag::BitAnd: return Associativity::LeftToRight;
  case Operator::Tag::BitOr: return Associativity::LeftToRight;
  case Operator::Tag::BitXor: return Associativity::LeftToRight;
  case Operator::Tag::Orelse: return Associativity::RightToLeft; // !!
  case Operator::Tag::Catch: return Associativity::RightToLeft;  // !!
  case Operator::Tag::EqEq: return NoneBecauseUnchainable;
  case Operator::Tag::NotEq: return NoneBecauseUnchainable;
  case Operator::Tag::Ge: return NoneBecauseUnchainable;
  case Operator::Tag::Le: return NoneBecauseUnchainable;
  case Operator::Tag::GeEq: return NoneBecauseUnchainable;
  case Operator::Tag::LeEq: return NoneBecauseUnchainable;
  case Operator::Tag::And: return Associativity::LeftToRight;
  case Operator::Tag::Or: return Associativity::LeftToRight;
  case Operator::Tag::DotDot: return NoneBecauseUnchainable;
  case Operator::Tag::Eq: return NoneBecauseUnchainable;
  case Operator::Tag::MulEq: return NoneBecauseUnchainable;
  case Operator::Tag::MulModEq: return NoneBecauseUnchainable;
  case Operator::Tag::MulBarEq: return NoneBecauseUnchainable;
  case Operator::Tag::MulDivEq: return NoneBecauseUnchainable;
  case Operator::Tag::ModEq: return NoneBecauseUnchainable;
  case Operator::Tag::AddEq: return NoneBecauseUnchainable;
  case Operator::Tag::AddModEq: return NoneBecauseUnchainable;
  case Operator::Tag::AddBarEq: return NoneBecauseUnchainable;
  case Operator::Tag::SubEq: return NoneBecauseUnchainable;
  case Operator::Tag::SubModEq: return NoneBecauseUnchainable;
  case Operator::Tag::SubBarEq: return NoneBecauseUnchainable;
  case Operator::Tag::BitShrEq: return NoneBecauseUnchainable;
  case Operator::Tag::BitRorEq: return NoneBecauseUnchainable;
  case Operator::Tag::BitShlEq: return NoneBecauseUnchainable;
  case Operator::Tag::BitRolEq: return NoneBecauseUnchainable;
  case Operator::Tag::BitShlBarEq: return NoneBecauseUnchainable;
  case Operator::Tag::BitAndEq: return NoneBecauseUnchainable;
  case Operator::Tag::BitOrEq: return NoneBecauseUnchainable;
  case Operator::Tag::BitXorEq: return NoneBecauseUnchainable;
  case Operator::Tag::Return: return NoneBecauseUnchainable;
  case Operator::Tag::Break: return NoneBecauseUnchainable;
  case Operator::Tag::Continue: return NoneBecauseUnchainable;
  case Operator::Tag::Defer: return NoneBecauseUnchainable;
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
  case Operator::Tag::Not: return false;
  case Operator::Tag::UnaryMinus: return false;
  case Operator::Tag::UnaryMinusMod: return false;
  case Operator::Tag::UnaryPlus: return false;
  case Operator::Tag::BitNot: return false;
  case Operator::Tag::PtrTo: return false;
  case Operator::Tag::Mul: return true;
  case Operator::Tag::MulMod: return true;
  case Operator::Tag::MulBar: return true;
  case Operator::Tag::Div: return true;
  case Operator::Tag::Mod: return true;
  case Operator::Tag::OrOr_ErrorSet: return true;
  case Operator::Tag::Add: return true;
  case Operator::Tag::AddMod: return true;
  case Operator::Tag::AddBar: return true;
  case Operator::Tag::Sub: return true;
  case Operator::Tag::SubMod: return true;
  case Operator::Tag::SubBar: return true;
  case Operator::Tag::BitShr: return true;
  case Operator::Tag::BitRor: return true;
  case Operator::Tag::BitShl: return true;
  case Operator::Tag::BitRol: return true;
  case Operator::Tag::BitShlBar: return true;
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
  case Operator::Tag::MulEq: return false;
  case Operator::Tag::MulModEq: return false;
  case Operator::Tag::MulBarEq: return false;
  case Operator::Tag::MulDivEq: return false;
  case Operator::Tag::ModEq: return false;
  case Operator::Tag::AddEq: return false;
  case Operator::Tag::AddModEq: return false;
  case Operator::Tag::AddBarEq: return false;
  case Operator::Tag::SubEq: return false;
  case Operator::Tag::SubModEq: return false;
  case Operator::Tag::SubBarEq: return false;
  case Operator::Tag::BitShrEq: return false;
  case Operator::Tag::BitRorEq: return false;
  case Operator::Tag::BitShlEq: return false;
  case Operator::Tag::BitRolEq: return false;
  case Operator::Tag::BitShlBarEq: return false;
  case Operator::Tag::BitAndEq: return false;
  case Operator::Tag::BitOrEq: return false;
  case Operator::Tag::BitXorEq: return false;
  case Operator::Tag::Return: return false;
  case Operator::Tag::Break: return false;
  case Operator::Tag::Continue: return false;
  case Operator::Tag::Defer: return false;
  }
  assert(false); // unreachable
}

std::string Operator::validate(std::string_view text)
{
  auto const
    beginTag = static_cast<size_t>(Tag::Dot),
    endTag = static_cast<size_t>(Tag::Defer);

  std::map<size_t, std::vector<Tag>> dist;
  auto minDistance = std::numeric_limits<size_t>::max();

  for (size_t i = beginTag; i <= endTag; i += 1)
  {
    auto const tag = static_cast<Tag>(i);
    auto const distance = levenshteinDistance(text, fmt::to_string(tag));
    if (distance == 0)
    {
      return std::string();
    }
    dist[distance].push_back(tag);
    minDistance = std::min({distance, minDistance});
  }

  if (minDistance > 3)
  {
    // if the distance is too large then too many alternatives
    // will be displayed, if the given operator is to dissimilar
    // to any other known operator than we just error without
    // any suggestions
    return fmt::format("unknown operator '{}'", text);
  }

  auto& alts = dist[minDistance];
  assert(alts.size() > 0);

  std::string res = fmt::format("'{}'", alts[0]);
  for (size_t i = 1; i < alts.size() - 1; i+= 1)
  {
    res += fmt::format(", '{}'", alts[i]);
  }
  if (alts.size() > 1)
  {
    res += fmt::format(" or '{}'", alts.back());
  }
  return fmt::format("unknown operator '{}', did you mean {}?", text, res);
}