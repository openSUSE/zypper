#include <iostream>
#include <ctime>

#include <fstream>
#include <list>
#include <string>
#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>
#include <zypp/base/String.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/PtrTypes.h>

#include <zypp/CapFactory.h>
#include <zypp/CapSet.h>

using namespace std;
using namespace zypp;

// work around flaw in y2logview
template<class _Tp>
  void printOnHack( const _Tp & obj )
  {
    MIL << obj << endl;
  };

///////////////////////////////////////////////////////////////////
// Just for the stats
struct Measure
{
  time_t _begin;
  Measure()
  : _begin( time(NULL) )
  {
    USR << "START MEASURE..." << endl;
  }
  ~Measure()
  {
    USR << "DURATION: " << (time(NULL)-_begin) << " sec." << endl;
  }
};

///////////////////////////////////////////////////////////////////
// Print stream status
ostream & operator<<( ostream & str, const istream & obj ) {
  return str
  << (obj.good() ? 'g' : '_')
  << (obj.eof()  ? 'e' : '_')
  << (obj.fail() ? 'F' : '_')
  << (obj.bad()  ? 'B' : '_');
}

///////////////////////////////////////////////////////////////////
// Fits forEachLine. A simple 'do' function
void collect( const std::string & linre_r )
{
  DBG << linre_r << endl;
}
///////////////////////////////////////////////////////////////////
// Fits forEachLine. A simple 'do' functor counting lines as base.
/*
 Line processing is prepared by providing 'virtual void doConsume(std::string)'.
 That's what a derived Collector will overload.
 */
struct Collector : public std::unary_function<const std::string &, bool>
{
  unsigned _lineNo;
  Collector()
  : _lineNo( 0 )
  {}
  virtual ~Collector()
  {}
  virtual void doConsume( const std::string & line_r )
  {}
  bool operator()( const std::string & line_r )
  {
    ++_lineNo;
    if ( ! (_lineNo % 10000) )
      DBG << "Got " << _lineNo << " lines..." << endl;
    doConsume( line_r );
    return true;
  }
};
///////////////////////////////////////////////////////////////////
// Fits forEachLine. An impatient collector ;)
struct ImpatientCollector : public Collector
{
  virtual void doConsume( const std::string & line_r )
  {
    if ( _lineNo == 1234 )
      ZYPP_THROW( Exception("takes to long") );
  }
};
///////////////////////////////////////////////////////////////////
// Fits forEachLine. Almost usefull collector.
/*
 Note that it's still a functor 'void operator()( const std::string & )'.

 On every invocation the string is parsed into a Capability, and the
 Capability is stored in a CapSet.

 Exceptions building the Capability are caught and collected in a
 Failure list. It's a matter of taste whether to immediately abort,
 or to parse to the end check for collected errors then. Room for
 improvement.

 see enhacedCollectorUsage().
*/
struct EnhacedCollector : public Collector
{
  // Stores line number, original string and Exception
  struct Failure
  {
    unsigned _lineNo;
    std::string _line;
    Exception _excpt;
    Failure( unsigned lineNo_r,
             const std::string & line_r, const Exception & excpt_r )
    : _lineNo( lineNo_r ), _line( line_r ), _excpt( excpt_r )
    {}
  };

  typedef std::list<Failure> FailedList;

  CapFactory  _factory;
  unsigned    _capLines;
  CapSet      _caps;
  FailedList  _failures;


  EnhacedCollector()
  : _capLines( 0 )
  {}

  void makeCap( const string & line_r )
  {
    ++_capLines; // count attempts
    try
      {
        // bulid Package deps.
        _caps.insert( _factory.parse( ResTraits<Package>::kind, line_r ) );
      }
    catch( Exception & excpt_r )
      {
        _failures.push_back( Failure(_lineNo, line_r, excpt_r) );
      }
  }

  virtual void doConsume( const std::string & line_r )
  {
    makeCap( line_r );
  }
};

// Print a Failure
ostream & operator<<( ostream & str, const EnhacedCollector::Failure & obj )
{
  return str << str::form( "[%u] \"%s\" ==> %s",
                           obj._lineNo,
                           obj._line.c_str(),
                           obj._excpt.asString().c_str() );
}

// Print EnhacedCollector stats
ostream & operator<<( ostream & str, const EnhacedCollector & obj )
{
  str << "Lines parsed : " << obj._lineNo << endl;
  str << "Cap lines    : " << obj._capLines << endl;
  str << "Capabilites  : " << obj._caps.size() << endl;
  str << "Parse errors : " << obj._failures.size() << endl;
  if ( obj._failures.size() )
    {
      copy( obj._failures.begin(), obj._failures.end(),
            ostream_iterator<EnhacedCollector::Failure>(ERR,"\n") );
      //-something-we-should-not-do-unless-....---------^^^
    }
  return str;
}

///////////////////////////////////////////////////////////////////
// Fits forEachLine.
/*
 Within a packages file, not every line defines a Capability. So
 EnhacedCollector is refined to turn Capability collection on and
 off, as appropriate. A dumb version simply storing all Capabilities
 defined somewhere in the packages file.
*/
struct PackageParseCollector : public EnhacedCollector
{
  static std::string _rxStrDeps;
  static str::regex  _rxDepOn;
  static str::regex  _rxDepOff;

  str::smatch _what;
  bool        _consume;

  PackageParseCollector()
  : _consume( false )
  {}

  bool matches( const string & line_r, const str::regex & rx_r )
  {
    return str::regex_match( line_r.begin(), line_r.end(), rx_r );
  }

  virtual void doConsume( const std::string & line_r )
  {
    if ( _consume )
      {
        if ( matches( line_r, _rxDepOff ) )
          {
            _consume = false;
          }
        else
          {
            EnhacedCollector::doConsume( line_r );
          }
      }
    else if ( matches( line_r, _rxDepOn ) )
      {
        _consume = true;
      }
  }
};

std::string PackageParseCollector::_rxStrDeps( "(Req|Prq|Prv|Con|Obs)" );
str::regex  PackageParseCollector::_rxDepOn ( str::form( "\\+%s:", _rxStrDeps.c_str() ) );
str::regex  PackageParseCollector::_rxDepOff( str::form( "-%s:", _rxStrDeps.c_str() ) );

/******************************************************************
**
**      FUNCTION NAME : enhacedCollectorUsage
**      FUNCTION TYPE :
*/
void enhacedCollectorUsage()
{
  // EnhacedCollector: Simply collect strings which are expected to
  // be Capabilities. Error handling is delayed by collecting failures.
  EnhacedCollector collector;
  collector( "" );
  collector( "foo baa kaa" );
  collector( "/bin/sh" );
  collector( "/bin/sh" );
  collector( "/bin/sh" );
  collector( "/bin/sh" );

  MIL << collector << endl;
  MIL << "Capabilities found:" << endl;
  for_each( collector._caps.begin(), collector._caps.end(),
            printOnHack<Capability> );
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  --argc;
  ++argv;
  if ( ! argc )
    {
      cerr << "Usage: Example.createCapabilities <packages file>" << endl;
      return 1;
    }
  string file( argv[0] );

  INT << "===[START]==========================================" << endl;

  // dump PackageParseCollector: open the file, and build Capabilities
  // from each appropriate line. Collecting failures.

  ifstream str( file.c_str() );
  (str?DBG:ERR) << file << ": " << str << endl;
  shared_ptr<Measure> duration( new Measure );

  PackageParseCollector datacollect;
  try
    {
      iostr::forEachLine( str, datacollect );
    }
  catch ( Exception & excpt_r )
    {
      // Note: Exceptions building a Capability are caught. So this is
      // something different. Maybe a bored ImpatientCollector.
      ZYPP_CAUGHT( excpt_r );
      ERR << "Parse error at line " << datacollect._lineNo << ": " << excpt_r << endl;
      return 1;
    }

  duration.reset();
  DBG << file << ": " << str << endl;

  MIL << datacollect << endl;
  MIL << "Capabilities found:" << endl;
  for_each( datacollect._caps.begin(), datacollect._caps.end(),
            printOnHack<Capability> );

  INT << "===[END]============================================" << endl;
  return 0;
}
