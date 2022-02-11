#pragma once

#include <set>
#include <string>
#include <string_view>

struct Intern final
{
private:
  std::set<std::string> d_strings;

  static Intern& instance();

public:

  template<size_t N>
  static std::string_view string(char const cString[N])
  {
    // for string literals
    return std::string_view(&cString[0], N);
  }

  static std::string_view string(char const* cString);

  static std::string_view string(char const* ptr, size_t length);

  static std::string_view string(std::string const& string);

  static std::string_view string(std::string&& string);
};