#ifndef ZYPPER_GETOPT_H_
#define ZYPPER_GETOPT_H_

#include <map>
#include <list>
#include <vector>
#include <string>

#include <getopt.h>
#include <zypp/TriBool.h>

using zypp::TriBool;

typedef std::map<std::string, std::list<std::string> > parsed_opts;

// longopts.flag must be NULL
parsed_opts parse_options( int argc, char **argv, const struct option *longopts );

class Zypper;

//! Parse a single string to an array of char* usable for getopt_long.
class Args
{
public:
  Args( const std::string & s );

  ~Args ()
  { clear_argv(); }

  int argc() const
  { return _args.size(); }

  char** argv()
  { clear_argv(); make_argv(); return _argv; }

private:
  // output iterator for split
  class OIter
  {
    Args * _dest;		//!< the destination object
  public:
    OIter (Args * dest) : _dest (dest) {}

    OIter& operator++()	//<  do nothing, operator = is enough
    { return *this; }

    OIter& operator*()	//<  the iterator works as the proxy object too
    { return *this; }

    OIter & operator=( const std::string & s )	//< do the real work
    { _dest->_args.push_back( s ); return *this; }
  };
  
  friend class OIter;

  void clear_argv();
  void make_argv();

  // for parsing
  std::vector<std::string> _args;
  // holds the argv style args
  char** _argv;
};

#endif /*ZYPPER_GETOPT_H_*/
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
