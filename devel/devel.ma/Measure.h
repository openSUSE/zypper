#ifndef MA_MEASURE_H
#define MA_MEASURE_H

#include <iostream>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
// Just for the stats
struct Measure
{
  struct Run
  {
    Run( const std::string & msg_r )
    : _msg( msg_r )
    , _begin( time(NULL) )
    {
      USR << "START MEASURE..." << _msg << std::endl;
    }
    ~Run()
    {
      USR << "DURATION: " << (time(NULL)-_begin) << " sec. " << _msg << std::endl;
    }
    std::string _msg;
    time_t      _begin;
  };

  Measure( const std::string & msg_r = std::string() )
  : _run( new Run( msg_r ) )
  {}

  void stop()
  { _run.reset(); }

  void start( const std::string & msg_r = std::string() )
  { _run.reset(); _run.reset( new Run( msg_r ) ); }

  private:
    zypp::shared_ptr<Run> _run;
};

///////////////////////////////////////////////////////////////////
#endif // MA_MEASURE_H
