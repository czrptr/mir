#pragma once

#include <compare>
#include <fmt/format.h>

struct Position final
{
  // Data
  size_t line;
  size_t column;

  // Constructors
  constexpr Position() noexcept = default;

  constexpr Position(size_t line, size_t column) noexcept
    : line(line), column(column)
  {}

  // Methods
  constexpr Position nextColumn() const noexcept { return Position(line, column + 1); }
  constexpr bool isValid() const noexcept { return *this != invalid(); }

  // Operators
  constexpr std::strong_ordering operator<=>(Position const&) const noexcept = default;

  static constexpr Position invalid()
  {
    constexpr auto pos = static_cast<size_t>(-1);
    return Position(pos, pos);
  };
};

template<>
struct fmt::formatter<Position>
{
  bool debug { false };
  bool shortened { false };
  bool user { false };

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

    if (it != end && *it == '+' && *(it + 1) == '1')
    {
      user = true;
      it += 2;
    }

    if (it != end && *it != '}')
    {
      throw format_error("invalid format");
    }

    return it;
  }

  template<typename FormatContext>
  auto format(Position pos, FormatContext& ctx)
  {
    if (user)
    {
      pos.line += 1;
      pos.column += 1;
    }

    if (debug)
    {
      if (shortened)
      {
        return fmt::format_to(ctx.out(), "Position{{{0}, {1}}}", pos.line, pos.column);
      }
      return fmt::format_to(ctx.out(), "Position{{line = {0}, column = {1}}}", pos.line, pos.column);
    }
    return fmt::format_to(ctx.out(), "{0}:{1}", pos.line, pos.column);
  }
};