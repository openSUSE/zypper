#ifndef ZYPPER_GETOPT_H_
#define ZYPPER_GETOPT_H_

#include <map>
#include <list>
#include <vector>
#include <string>

#include <getopt.h>

#include <boost/logic/tribool.hpp> //! \todo replace with zypp's tribool

typedef std::map<std::string, std::list<std::string> > parsed_opts;

// longopts.flag must be NULL
parsed_opts parse_options (int argc, char **argv,
			   const struct option *longopts);

class Zypper;

/**
 * 
 */
boost::tribool get_boolean_option(
    Zypper & zypper,
    const std::string & pname,
    const std::string & nname );


//! Parse a single string to an array of char* usable for getopt_long.
class Args {
public:
  Args (const std::string& s);

  ~Args () {
    clear_argv ();
  }

  int argc () const {
    return _args.size ();
  }
  char ** argv () {
    clear_argv ();
    make_argv ();
    return _argv;
  }

private:
  // output iterator for split
  class OIter {
    Args * _dest;		//!< the destination object
  public:
    OIter (Args * dest): _dest (dest) {}

    OIter& operator ++ () {
      return *this;		// do nothing, operator = is enough
    }
    OIter& operator * () {
      return *this;	 // the iterator works as the proxy object too
    }
    OIter& operator = (const std::string& s) {
      _dest->_args.push_back (s); // do the real work
      return *this;
    }
  };
  
  friend class OIter;

  void clear_argv ();
  void make_argv ();

  // for parsing
  std::vector<std::string> _args;
  // holds the argv style args
  char ** _argv;
};

extern parsed_opts copts; // command options

#endif /*ZYPPER_GETOPT_H_*/
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
