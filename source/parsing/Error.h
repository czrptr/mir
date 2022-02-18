#pragma once

#include <parsing/Position.h>

#include <fmt/format.h>

#include <string>
#include <vector>

struct Error final
{
public:
  enum class Tag
  {
    Error,
    Note,
  };

  struct Part
  {
  private:
    std::string d_filepath;
    Position d_start;
    Position d_end;
    Tag d_tag;
    std::string d_message;

  public:
    Part(std::string const& filepath, Position start, Position end, Tag tag, std::string const& message) noexcept
      : d_filepath(filepath)
      , d_start(start)
      , d_end(end)
      , d_tag(tag)
      , d_message(message)
    {}

    std::string const& filePath() const noexcept { return d_filepath; }
    Position start() const noexcept { return d_start; }
    Position end() const noexcept { return d_end; }
    Tag tag() const noexcept { return d_tag; }
    std::string const& message() const noexcept { return d_message; }
  };

private:
  std::vector<Part> d_parts;

public:
  Error(std::string const& filepath, Position start, Position end, std::string const& message)
  {
    d_parts.emplace_back(filepath, start, end, Tag::Error, message);
  }

  Error(std::string const& filepath, Position position, std::string const& message)
    : Error(filepath, position, position.nextColumn(), message)
  {}

  Error(std::string const& filepath, std::string const& message)
    : Error(filepath, Position::invalid(), Position::invalid(), message)
  {}

  Error& note(std::string const& filepath, Position start, Position end, std::string const& message)
  {
    d_parts.emplace_back(filepath, start, end, Tag::Note, message);
    return *this;
  }

  Error& note(std::string const& filepath, Position position, std::string const& message)
  {
    d_parts.emplace_back(filepath, position, position.nextColumn(), Tag::Note, message);
    return *this;
  }

  Error& note(std::string const& filepath, std::string const& message)
  {
    return note(filepath, d_parts.back().start(), d_parts.back().end(), message);
  }

  Error& note(Position start, Position end, std::string const& message)
  {
    d_parts.emplace_back(d_parts.back().filePath(), start, end, Tag::Note, message);
    return *this;
  }

  Error& note(Position position, std::string const& message)
  {
    d_parts.emplace_back(d_parts.back().filePath(), position, position.nextColumn(), Tag::Note, message);
    return *this;
  }

  Error& note(std::string const& message)
  {
    return note(d_parts.back().start(), d_parts.back().end(), message);
  }

  std::vector<Part> const& parts() const { return d_parts; }
};

template<>
struct fmt::formatter<Error::Tag> : fmt::formatter<string_view>
{
  template<typename FormatContext>
  auto format(Error::Tag tag, FormatContext& ctx)
  {
    std::string_view name = "ERROR_TYPE_INVALID";
    switch (tag)
    {
    case Error::Tag::Error: name = "error"; break;
    case Error::Tag::Note: name = "note"; break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};

template<>
struct fmt::formatter<Error::Part> : fmt::formatter<fmt::string_view>
{
  template<typename FormatContext>
  auto format(Error::Part const& part, FormatContext& ctx)
  {
    if (!part.start().isValid() && !part.end().isValid())
    {
      return fmt::format_to(
        ctx.out(),
        "{0}: {1}",
        part.filePath(), part.message());
    }
    return fmt::format_to(
      ctx.out(),
      "{0}:{1}: {2}: {3}",
      part.filePath(), part.start(), part.tag(), part.message());
  }
};

template<>
struct fmt::formatter<Error>
{
  constexpr auto parse(fmt::format_parse_context& ctx)
  {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(Error const& error, FormatContext& ctx)
  {
    return fmt::format_to(ctx.out(), "{}", fmt::join(error.parts(), "\n"));
  }
};