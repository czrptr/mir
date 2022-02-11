#include "Utils.h"

#include <algorithm>

size_t levenshteinDistance(std::string_view str1, std::string_view str2)
{
  if (str1.length() == 0)
  {
    return str2.length();
  }

  if (str2.length() == 0)
  {
    return str1.length();
  }

  std::string_view const
    tail1 = str1.substr(1),
    tail2 = str2.substr(1);

  size_t const levTailTail = levenshteinDistance(tail1, tail2);

  if (str1[0] == str2[0])
  {
    return levTailTail;
  }

  return 1 + std::min({
    levenshteinDistance(tail1, str2),
    levenshteinDistance(str1, tail2),
    levTailTail
  });
}