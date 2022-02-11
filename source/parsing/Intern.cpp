#include "parsing/Intern.h"

Intern& Intern::instance()
{
  static Intern inst;
  return inst;
}

std::string_view Intern::string(char const* cString)
{
  std::string string(cString);
  auto const [it, _] = instance().d_strings.insert(std::move(string));
  return std::string_view(it->c_str(), it->length());
}

std::string_view Intern::string(char const* ptr, size_t length)
{
  std::string string(ptr, length);
  auto const [it, _] = instance().d_strings.insert(std::move(string));
  return std::string_view(it->c_str(), it->length());
}

std::string_view Intern::string(std::string const& string)
{
  auto const [it, _] = instance().d_strings.insert(string);
  return std::string_view(it->c_str(), it->length());

}

std::string_view Intern::string(std::string&& string)
{
  auto const [it, _] = instance().d_strings.insert(std::move(string));
  return std::string_view(it->c_str(), it->length());
}

// auto& inst = instance();

// auto const [it, inserted] = inst.d_strings.insert(string);

// if (it != inst.d_strings.end())
// {
//   return std::string_view(string.c_str(), string.length());
// }