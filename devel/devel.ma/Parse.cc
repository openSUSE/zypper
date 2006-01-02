//http://www.boost.org/libs/libraries.htm
#include <iostream>
#include <list>
#include <string>

#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/parser/tagfile/Tags.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace tagfile
    { /////////////////////////////////////////////////////////////////

      void echoOn( std::ostream & str,
                   iterator_t first, const iterator_t last,
                   const char* s = "" )
      {
        //return;
        str << first.get_position() << last.get_position();
        str << s << ">>";
        while ( first != last )
          str << *first++;
        str << "<< " << std::endl;
      }

      void echo( iterator_t first, const iterator_t last, const char* s = "" )
      {
        echoOn( DBG, first, last, s );
      }
      void mecho( iterator_t first, const iterator_t last, const char* s = "" )
      {
        echoOn( MIL, first, last, s );
      }

      /////////////////////////////////////////////////////////////////
    } // namespace tagfile
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  Types
//
////////////////////////////////////////////////////////////////////////////
using std::endl;
using std::list;
using std::string;
using namespace zypp;
using namespace zypp::parser::tagfile;
typedef scanner<iterator_t>            scanner_t;
typedef rule<scanner_t>                rule_t;
////////////////////////////////////////////////////////////////////////////
//
//  Just for the stats
//
////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////
//
//  Main
//
////////////////////////////////////////////////////////////////////////////
int main( int argc, char* argv[] )
{
  INT << "===[START]==========================================" << endl;
  string infile( "p" );
  if (argc >= 2 )
    infile = argv[1];

  // Create a file iterator for this file
  fiterator_t first(infile);
  if (!first)
    {
      ERR << "Unable to open file!\n";
      return -1;
    }
  // Create an EOF iterator
  fiterator_t last = first.make_end();

  // Create position iterators
  iterator_t begin( first, last, infile );
  iterator_t end;

  // Result var
  SingleTag stag;
  MultiTag  mtag;
  STag      stagData;
  MTag      mtagData;

  rule_t file =   end_p
                | ( stag
                  | mtag
                  | ( *blank_p
                      >> !( ch_p('#')
                            >> *(anychar_p - eol_p)
                          )
                      >> (eol_p|end_p)
                    | error_report_p( "neither empty nor comment" )
                    )
                  )
                  >> file;

  // Parse
  shared_ptr<Measure> duration( new Measure );
  parse_info<iterator_t> info
    = parse( begin, end, file );
  duration.reset();

  // Check for fail...
  if ( info.full )
    USR << "Parse succeeded!\n";
  else if ( info.hit )
    {
      ERR << "Parse partial!\n";
      ERR << " at pos " << info.length << endl;
    }
  else
    {
      ERR << "Parse failed!\n";
      ERR << " at pos " << info.length << endl;
    }

  INT << "===[END]============================================" << endl;
  return 0;
}
