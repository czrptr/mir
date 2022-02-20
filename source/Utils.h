#pragma once

#include <string_view>
#include <fmt/core.h>

size_t levenshteinDistance(std::string_view str1, std::string_view str2);