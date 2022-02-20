#include "parsing/Operator.h"

#include <Utils.h>
#include <parsing/Error.h>

#include <fmt/color.h>
#include <fort.hpp>

#include <map>
#include <limits>
#include <cassert>

Operator::Operator(Token token)
  : d_start(token.start())
  , d_end(token.end())
  , d_text(token.text())
{}

Operator::Fix Operator::fix() const
{
  return fix(d_tag);
}

bool Operator::chainable() const
{
  return chainable(d_tag);
}

Operator::Associativity Operator::associativity() const
{
  return associativity(d_tag);
}

size_t Operator::precedence() const
{
  return precedence(d_tag);
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
    BitRol, 7,
    BitXor, 8,
    Catch, 9,
    LeEq, 10,
    And, 11,
    Or, 12,
    DotDot, 13,
    BitXorEq, 14,
    Defer, 15
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

Operator::Fix Operator::fix(Operator::Tag tag)
{
  switch (tag)
  {
  case Operator::Tag::Dot: return Fix::Infix;
  case Operator::Tag::Opt: return Fix::Postfix;
  case Operator::Tag::PtrDeref: return Fix::Postfix;
  case Operator::Tag::Not_ErrorUnion: return Fix::Infix;
  case Operator::Tag::Try: return Fix::Prefix;
  case Operator::Tag::Not: return Fix::Prefix;
  case Operator::Tag::UnaryMinus: return Fix::Prefix;
  case Operator::Tag::UnaryMinusMod: return Fix::Prefix;
  case Operator::Tag::UnaryPlus: return Fix::Prefix;
  case Operator::Tag::BitNot: return Fix::Prefix;
  case Operator::Tag::PtrTo: return Fix::Prefix;
  case Operator::Tag::Mul: return Fix::Infix;
  case Operator::Tag::MulMod: return Fix::Infix;
  case Operator::Tag::MulBar: return Fix::Infix;
  case Operator::Tag::Div: return Fix::Infix;
  case Operator::Tag::Mod: return Fix::Infix;
  case Operator::Tag::OrOr_ErrorSet: return Fix::Infix;
  case Operator::Tag::Add: return Fix::Infix;
  case Operator::Tag::AddMod: return Fix::Infix;
  case Operator::Tag::AddBar: return Fix::Infix;
  case Operator::Tag::Sub: return Fix::Infix;
  case Operator::Tag::SubMod: return Fix::Infix;
  case Operator::Tag::SubBar: return Fix::Infix;
  case Operator::Tag::BitShr: return Fix::Infix;
  case Operator::Tag::BitRor: return Fix::Infix;
  case Operator::Tag::BitShl: return Fix::Infix;
  case Operator::Tag::BitShlBar: return Fix::Infix;
  case Operator::Tag::BitRol: return Fix::Infix;
  case Operator::Tag::BitAnd: return Fix::Infix;
  case Operator::Tag::BitOr: return Fix::Infix;
  case Operator::Tag::BitXor: return Fix::Infix;
  case Operator::Tag::Orelse: return Fix::Infix;
  case Operator::Tag::Catch: return Fix::Infix;
  case Operator::Tag::EqEq: return Fix::Infix;
  case Operator::Tag::NotEq: return Fix::Infix;
  case Operator::Tag::Ge: return Fix::Infix;
  case Operator::Tag::Le: return Fix::Infix;
  case Operator::Tag::GeEq: return Fix::Infix;
  case Operator::Tag::LeEq: return Fix::Infix;
  case Operator::Tag::And: return Fix::Infix;
  case Operator::Tag::Or: return Fix::Infix;
  case Operator::Tag::DotDot: return Fix::Infix;
  case Operator::Tag::Eq: return Fix::Infix;
  case Operator::Tag::MulEq: return Fix::Infix;
  case Operator::Tag::MulModEq: return Fix::Infix;
  case Operator::Tag::MulBarEq: return Fix::Infix;
  case Operator::Tag::MulDivEq: return Fix::Infix;
  case Operator::Tag::ModEq: return Fix::Infix;
  case Operator::Tag::AddEq: return Fix::Infix;
  case Operator::Tag::AddModEq: return Fix::Infix;
  case Operator::Tag::AddBarEq: return Fix::Infix;
  case Operator::Tag::SubEq: return Fix::Infix;
  case Operator::Tag::SubModEq: return Fix::Infix;
  case Operator::Tag::SubBarEq: return Fix::Infix;
  case Operator::Tag::BitShrEq: return Fix::Infix;
  case Operator::Tag::BitRorEq: return Fix::Infix;
  case Operator::Tag::BitShlEq: return Fix::Infix;
  case Operator::Tag::BitShlBarEq: return Fix::Infix;
  case Operator::Tag::BitRolEq: return Fix::Infix;
  case Operator::Tag::BitAndEq: return Fix::Infix;
  case Operator::Tag::BitOrEq: return Fix::Infix;
  case Operator::Tag::BitXorEq: return Fix::Infix;
  case Operator::Tag::Return: return Fix::Prefix;
  case Operator::Tag::Break: return Fix::Prefix;
  case Operator::Tag::Continue: return Fix::Prefix;
  case Operator::Tag::Defer: return Fix::Prefix;
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
  case Operator::Tag::Try: return true;
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
  case Operator::Tag::BitShlBar: return true;
  case Operator::Tag::BitRol: return true;
  case Operator::Tag::BitAnd: return true;
  case Operator::Tag::BitOr: return true;
  case Operator::Tag::BitXor: return true;
  case Operator::Tag::Orelse: return true;
  case Operator::Tag::Catch: return true;
  case Operator::Tag::EqEq: return true;
  case Operator::Tag::NotEq: return true;
  case Operator::Tag::Ge: return true;
  case Operator::Tag::Le: return true;
  case Operator::Tag::GeEq: return true;
  case Operator::Tag::LeEq: return true;
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
  case Operator::Tag::BitShlBarEq: return false;
  case Operator::Tag::BitRolEq: return false;
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

Operator::Associativity Operator::associativity(Operator::Tag tag)
{
  if (fix(tag) == Fix::Prefix)
  {
    return Associativity::RightToLeft;
  }

  if (fix(tag) == Fix::Postfix)
  {
    return Associativity::LeftToRight;
  }

  switch (tag)
  {
  case Operator::Tag::Not_ErrorUnion: return Associativity::RightToLeft;
  case Operator::Tag::Orelse: return Associativity::RightToLeft;
  case Operator::Tag::Catch: return Associativity::RightToLeft;
  default: return Associativity::LeftToRight;
  }
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

static std::string tableText(Operator::Tag tag)
{
  switch (tag)
  {
    case Operator::Tag::Dot: return "a.b";
    case Operator::Tag::Opt: return "a?";
    case Operator::Tag::PtrDeref: return "a^";
    case Operator::Tag::Not_ErrorUnion: return "a!b";
    case Operator::Tag::Try: return "try a";
    case Operator::Tag::Not: return "not a";
    case Operator::Tag::UnaryMinus: return "-a";
    case Operator::Tag::UnaryMinusMod: return "-%a";
    case Operator::Tag::UnaryPlus: return "+a";
    case Operator::Tag::BitNot: return "!a";
    case Operator::Tag::PtrTo: return "^a";
    case Operator::Tag::Mul: return "a * b";
    case Operator::Tag::MulMod: return "a *% b";
    case Operator::Tag::MulBar: return "a *| b";
    case Operator::Tag::Div: return "a / b";
    case Operator::Tag::Mod: return "a % b";
    case Operator::Tag::OrOr_ErrorSet: return "a || b";
    case Operator::Tag::Add: return "a + b";
    case Operator::Tag::AddMod: return "a +% b";
    case Operator::Tag::AddBar: return "a +| b";
    case Operator::Tag::Sub: return "a - b";
    case Operator::Tag::SubMod: return "a -% b";
    case Operator::Tag::SubBar: return "a -| b";
    case Operator::Tag::BitShr: return "a >> b";
    case Operator::Tag::BitRor: return "a >% b";
    case Operator::Tag::BitShl: return "a << b";
    case Operator::Tag::BitShlBar: return "a <| b";
    case Operator::Tag::BitRol: return "a <% b";
    case Operator::Tag::BitAnd: return "a & b";
    case Operator::Tag::BitOr: return "a | b";
    case Operator::Tag::BitXor: return "a ~ b";
    case Operator::Tag::Orelse: return "a orelse b";
    case Operator::Tag::Catch: return "a catch b";
    case Operator::Tag::EqEq: return "a == b";
    case Operator::Tag::NotEq: return "a != b";
    case Operator::Tag::Ge: return "a > b";
    case Operator::Tag::Le: return "a < b";
    case Operator::Tag::GeEq: return "a >= b";
    case Operator::Tag::LeEq: return "a <= b";
    case Operator::Tag::And: return "a and b";
    case Operator::Tag::Or: return "a or b";
    case Operator::Tag::DotDot: return "a..b";
    case Operator::Tag::Eq: return "a = b";
    case Operator::Tag::MulEq: return "a *= b";
    case Operator::Tag::MulModEq: return "a *%= b";
    case Operator::Tag::MulBarEq: return "a *|= b";
    case Operator::Tag::MulDivEq: return "a /= b";
    case Operator::Tag::ModEq: return "a %= b";
    case Operator::Tag::AddEq: return "a += b";
    case Operator::Tag::AddModEq: return "a +%= b";
    case Operator::Tag::AddBarEq: return "a +|= b";
    case Operator::Tag::SubEq: return "a -= b";
    case Operator::Tag::SubModEq: return "a -%= b";
    case Operator::Tag::SubBarEq: return "a -|= b";
    case Operator::Tag::BitShrEq: return "a >>= b";
    case Operator::Tag::BitRorEq: return "a >%= b";
    case Operator::Tag::BitShlEq: return "a <<= b";
    case Operator::Tag::BitShlBarEq: return "a <|= b";
    case Operator::Tag::BitRolEq: return "a <%= b";
    case Operator::Tag::BitAndEq: return "a &= b";
    case Operator::Tag::BitOrEq: return "a |= b";
    case Operator::Tag::BitXorEq: return "a ~= b";
    case Operator::Tag::Return: return "return";
    case Operator::Tag::Break: return "break";
    case Operator::Tag::Continue: return "continue";
    case Operator::Tag::Defer: return "defer a";
  }
  assert(false); // unreachable
}

static std::string description(Operator::Tag tag)
{
  switch (tag)
  {
    case Operator::Tag::Dot: return "Member access";
    case Operator::Tag::Opt: return "Optional chaining";
    case Operator::Tag::PtrDeref: return "Pointer dereference";
    case Operator::Tag::Not_ErrorUnion: return "Error union";
    case Operator::Tag::Try: return "Try";
    case Operator::Tag::Not: return "Boolean not";
    case Operator::Tag::UnaryMinus: return "Negation";
    case Operator::Tag::UnaryMinusMod: return "Wrapping negation";
    case Operator::Tag::UnaryPlus: return "Unary plus";
    case Operator::Tag::BitNot: return "Bitwise not";
    case Operator::Tag::PtrTo: return "Pointer to (const)";
    case Operator::Tag::Mul: return "Multiplication";
    case Operator::Tag::MulMod: return "Wrapping multiplication";
    case Operator::Tag::MulBar: return "Saturating multiplication";
    case Operator::Tag::Div: return "Division";
    case Operator::Tag::Mod: return "Modulo / Remainder division";
    case Operator::Tag::OrOr_ErrorSet: return "Error set union";
    case Operator::Tag::Add: return "Addition";
    case Operator::Tag::AddMod: return "Wrapping addition";
    case Operator::Tag::AddBar: return "Saturating addition";
    case Operator::Tag::Sub: return "Subtraction";
    case Operator::Tag::SubMod: return "Wrapping subtraction";
    case Operator::Tag::SubBar: return "Saturating subtraction";
    case Operator::Tag::BitShr: return "Bit shift right";
    case Operator::Tag::BitRor: return "Bit rotate right";
    case Operator::Tag::BitShl: return "Bit shift left";
    case Operator::Tag::BitShlBar: return "Saturating bit shift left";
    case Operator::Tag::BitRol: return "Bit rotate left";
    case Operator::Tag::BitAnd: return "Bitwise and";
    case Operator::Tag::BitOr: return "Bitwise or";
    case Operator::Tag::BitXor: return "Bitwise xor";
    case Operator::Tag::Orelse: return "Orelse";
    case Operator::Tag::Catch: return "Catch";
    case Operator::Tag::EqEq: return "Equality";
    case Operator::Tag::NotEq: return "Negated equality";
    case Operator::Tag::Ge: return "Greater then";
    case Operator::Tag::Le: return "Lesser then";
    case Operator::Tag::GeEq: return "Greater then or equal to";
    case Operator::Tag::LeEq: return "Lesser then or equal to";
    case Operator::Tag::And: return "Boolean and";
    case Operator::Tag::Or: return "Boolean or";
    case Operator::Tag::DotDot: return "Range";
    case Operator::Tag::Eq: return "Assigment";
    case Operator::Tag::MulEq: return "a = a * b";
    case Operator::Tag::MulModEq: return "a = a *% b";
    case Operator::Tag::MulBarEq: return "a = a *| b";
    case Operator::Tag::MulDivEq: return "a = a / b";
    case Operator::Tag::ModEq: return "a = a % b";
    case Operator::Tag::AddEq: return "a = a + b";
    case Operator::Tag::AddModEq: return "a = a +% b";
    case Operator::Tag::AddBarEq: return "a = a +| b";
    case Operator::Tag::SubEq: return "a = a - b";
    case Operator::Tag::SubModEq: return "a = a -% b";
    case Operator::Tag::SubBarEq: return "a = a -| b";
    case Operator::Tag::BitShrEq: return "a = a >> b";
    case Operator::Tag::BitRorEq: return "a = a >% b";
    case Operator::Tag::BitShlEq: return "a = a << b";
    case Operator::Tag::BitShlBarEq: return "a = a <| b";
    case Operator::Tag::BitRolEq: return "a = a <% b";
    case Operator::Tag::BitAndEq: return "a = a & b";
    case Operator::Tag::BitOrEq: return "a = a | b";
    case Operator::Tag::BitXorEq: return "a = a ~ b";
    case Operator::Tag::Return: return "Return";
    case Operator::Tag::Break: return "Break";
    case Operator::Tag::Continue: return "Continue";
    case Operator::Tag::Defer: return "Defer execution";
  }
  assert(false); // unreachable
}

std::string Operator::tableWithInfo()
{
 struct Line
  {
    Tag tag;
    size_t pred;
    std::string text;
    std::string desc;
    bool chain;
    Associativity assoc;
  };

  fort::utf8_table table;
  table.set_border_style(FT_SOLID_STYLE);

  // Header
  table << "Precedence" << "Operator" << "Description" << "Chainable" << "Associativity";

  // Formatting
  table.column(0).set_cell_text_align(fort::text_align::center);
  table.column(0).set_cell_text_style(fort::text_style::bold);
  table.column(0).set_cell_content_fg_color(fort::color::light_whyte);
  table.column(3).set_cell_text_align(fort::text_align::center);
  table.column(4).set_cell_text_align(fort::text_align::center);

  table.row(0).set_cell_text_style(fort::text_style::bold);
  table.row(0).set_cell_content_fg_color(fort::color::light_yellow);
  table.row(0).set_cell_row_type(fort::row_type::header);
  table.cell(0, 0).set_cell_content_fg_color(fort::color::light_yellow);

  auto const
    first = static_cast<size_t>(Operator::Dot),
    last = static_cast<size_t>(Operator::Defer);

  std::vector<Line> lines;
  lines.reserve(last);

  for (size_t i = first; i <= last; i += 1)
  {
    auto const tag = static_cast<Operator::Tag>(i);
    Line const l = {
      tag, precedence(tag), tableText(tag), description(tag), chainable(tag), associativity(tag)};

    lines.push_back(l);
  }

  auto const indexOf = [&](Operator::Tag tag)
  {
    return std::find_if(lines.begin(), lines.end(), [=](Line const& l)
    {
      return l.tag == tag;
    });
  };

  lines.insert(indexOf(Operator::Not_ErrorUnion),
    {Operator::Dot, 0, "a()", "Funcion call", true, Associativity::LeftToRight});

  lines.insert(indexOf(Operator::Not_ErrorUnion),
    {Operator::Dot, 0, "a[]", "Array access / Span", true, Associativity::LeftToRight});

  lines.insert(indexOf(Operator::Try),
    {Operator::Dot, 2, "a{}", "Type initialization", false, Associativity::LeftToRight});

  lines.insert(indexOf(Operator::Mul),
    {Operator::Dot, 4, "^mut a", "Pointer to mutable", false, Associativity::RightToLeft});

  lines.insert(indexOf(Operator::EqEq),
    {Operator::Dot, 9, "a catch |err| b", "Catch with capture", true, Associativity::RightToLeft});

  lines.insert(indexOf(Operator::Eq),
    {Operator::Dot, 13, "a..", "Range (to the end)", false, Associativity::LeftToRight});

  lines.insert(indexOf(Operator::Break),
    {Operator::Dot, 15, "return a", "Return a value", false, Associativity::RightToLeft});

  lines.insert(indexOf(Operator::Continue),
    {Operator::Dot, 15, "break :label", "Break to (outer) label", false, Associativity::RightToLeft});

  lines.insert(indexOf(Operator::Continue),
    {Operator::Dot, 15, "break a", "Block return", false, Associativity::RightToLeft});

  lines.insert(indexOf(Operator::Continue),
    {Operator::Dot, 15, "break :label a", "Block return (explicit)", false, Associativity::RightToLeft});

  lines.insert(indexOf(Operator::Defer),
    {Operator::Dot, 15, "continue :label", "Continue to (outer) label", false, Associativity::RightToLeft});

  // Data
  table << fort::endr << "0" << "\"atomic\"" << "Literals\nControl statements" << "No" << "-";

  size_t prevPred = static_cast<size_t>(-1);
  for (auto const& l : lines)
  {
    if (l.pred != prevPred)
    {
      prevPred = l.pred;
      table
        << fort::endr << fort::separator
        << (l.pred + 1) << l.text << l.desc << (l.chain ? "Yes" : "No") << (l.chain ? fmt::to_string(l.assoc) : "-");
    }
    else
    {
      table
        << fort::endr
        << "" << l.text << l.desc << "" << "";
    }
  }

  // Footer
  table
    << fort::endr << fort::separator
    << "Precedence" << "Operator" << "Description" << "Chainable" << "Associativity";

  auto const idx = table.row_count() - 1;
  table.row(idx).set_cell_text_style(fort::text_style::bold);
  table.row(idx).set_cell_content_fg_color(fort::color::light_yellow);
  table.row(idx).set_cell_row_type(fort::row_type::header);
  table.cell(idx, 0).set_cell_content_fg_color(fort::color::light_yellow);

  return table.to_string();
}