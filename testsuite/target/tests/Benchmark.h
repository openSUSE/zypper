#ifndef TEST_BENCHMARK_H
#define TEST_BENCHMARK_H

#include <ctime>
#include <string>

struct Benchmark
{
  clock_t _time_start;
  clock_t _curr_time;
  std::string _name;
  
  Benchmark( const std::string &name);
  ~Benchmark();
};

#endif
