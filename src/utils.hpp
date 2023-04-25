#pragma once

#include <iostream>
#include <vector>

#include "cell.hpp"
#include "grid.hpp"

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
std::ostream &operator<<(std::ostream &, const simulake::BaseCell::context_t);
