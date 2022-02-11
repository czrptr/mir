#pragma once

#include <tuple>
#include <vector>

// TODO: make constexpr
template <typename InputT, typename ResultT>
struct Switch
{
private:
  // Types
  using Self = Switch<InputT, ResultT>;
  using PairT = std::tuple<InputT, ResultT>;

  // Data
  InputT d_value;
  std::vector<PairT> d_cases;

public:
  // Constructors
  Switch(InputT const& value)
    : d_value(value)
  {}

  // Methods
  Self& Case(InputT const& value, ResultT const& result)
  {
    d_cases.push_back({value, result});
    return *this;
  }

  ResultT Default(ResultT const& fallback)
  {
    for (auto const& [value, result] : d_cases)
    {
      if (d_value == value) return result;
    }
    return fallback;
  }
};