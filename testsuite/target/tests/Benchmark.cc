
#include <iostream>

#include "Benchmark.h"

#include "zypp/base/Logger.h"

Benchmark::Benchmark( const std::string &name)
{
  _name = name;
  _time_start = clock();
}
  
Benchmark::~Benchmark()
{
  _curr_time = clock() - _time_start;           // time in micro seconds
  MIL << _name << " completed in " << (double) _curr_time / CLOCKS_PER_SEC << " seconds" << std::endl;
}

