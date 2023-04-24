#include <iostream>
#include <benchmark/benchmark.h>
#include "include/bs_threadpool.hpp"

int main() {
  BS::thread_pool tp(10);
  return 0;
}
