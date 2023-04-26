#pragma once

#include <iostream>
#include <vector>

#include "cell.hpp"
#include "grid.hpp"

#if DEBUG
#define BREAKPOINT __builtin_trap()
#else
#define BREAKPOINT
#endif


// pretty print vectors
template <typename T>
std::ostream &operator<<(std::ostream &stream, const std::vector<T> &vec) {
  stream << "[ ";
  for (const auto &item : vec)
    stream << item << ", ";
  stream << "\b\b ]\n";
  return stream;
}

// pretty print tuples
template <typename... Ts, uint32_t... Is>
std::ostream &operator<<(std::ostream &stream, const std::tuple<Ts...> &tuple) {
  static_assert(sizeof...(Is) == sizeof...(Ts), "invalid tuple");
  static_assert(sizeof...(Ts) > 0, "empty tuple");

  const auto last = sizeof...(Ts);
  stream << "( ";
  ((stream << std::get<Is>(tuple) << ", "), ...);
  stream << "\b\b )";

  return stream;
}

// pretty print context
std::ostream &operator<<(std::ostream &, const simulake::CellType);

// pretty print cell context
std::ostream &operator<<(std::ostream &, const simulake::BaseCell::context_t);

// pretty print grid
std::ostream &operator<<(std::ostream &, const simulake::Grid &);

// pretty print cell type
std::ostream &operator<<(std::ostream &, const simulake::CellType);

// pretty print context
// TODO(vir): 8 bytes for now, might change later so update this accordingly
std::ostream &operator<<(std::ostream &, const simulake::BaseCell::context_t);

namespace simulake {

#if DEBUG
#define PROFILE_SCOPE(name) ::simulake::scope_timer_t timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__PRETTY_FUNCTION__)
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#endif

// scope timer
struct scope_timer_t {
  const std::chrono::time_point<std::chrono::high_resolution_clock> start;
  const std::string_view title;

  scope_timer_t(const char*);
  ~scope_timer_t();
};

} // namespace simulake