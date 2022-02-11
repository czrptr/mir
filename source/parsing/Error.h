#pragma once

#include <parsing/Position.h>

#include <exception>
#include <string>
#include <vector>
#include <fmt/format.h>

struct Error : public std::exception
{
public:
  enum class Type
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
    Type d_type;
    std::string d_message;

  public:
    Part(std::string const& filepath, Position start, Position end, Type type, std::string const& message) noexcept
      : d_filepath("<file>") // TODO remove default value
      , d_start(start)
      , d_end(end)
      , d_type(type)
      , d_message(message)
    {}

    std::string const& filePath() const noexcept { return d_filepath; }
    Position start() const noexcept { return d_start; }
    Position end() const noexcept { return d_end; }
    Type type() const noexcept { return d_type; }
    std::string const& message() const noexcept { return d_message; }
  };

private:
  std::vector<Part> d_parts;

public:
  Error(std::string const& filepath, Position start, Position end, Type type, std::string const& message)
  {
    d_parts.emplace_back(filepath, start, end, type, message);
  }

  Error(std::vector<Part>&& parts)
    : d_parts(std::move(parts))
  {}

  std::vector<Part> const& parts() const { return d_parts; }
};

template<>
struct fmt::formatter<Error::Type> : formatter<string_view>
{
  template<typename FormatContext>
  auto format(Error::Type tag, FormatContext& ctx)
  {
    std::string_view name = "ERROR_TYPE_INVALID";
    switch (tag)
    {
    case Error::Type::Error: name = "error"; break;
    case Error::Type::Note: name = "note"; break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};

template<>
struct fmt::formatter<Error::Part> : formatter<string_view>
{
  template<typename FormatContext>
  auto format(Error::Part const& part, FormatContext& ctx)
  {
    return fmt::format_to(
      ctx.out(),
      "{0}:{1}: {2}: {3}",
      part.filePath(), part.start(), part.type(), part.message()
    );
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