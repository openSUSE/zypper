//http://www.boost.org/libs/libraries.htm
#include <iostream>
#include <list>
#include <string>

#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/parser/tagfile/Tags.h>

#include <zypp/NVRA.h>

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
      void xecho( const char * first, const char *const last, const char* s = "" )
      {
        DBG << ">>" << std::string(first,last) << "<< " << std::endl;
      }
      void mecho( iterator_t first, const iterator_t last, const char* s = "" )
      {
        echoOn( MIL, first, last, s );
      }

      ////////////////////////////////////////////////////////////////////////////
      //
      //  SingleTag Grammar
      //
      ////////////////////////////////////////////////////////////////////////////


      struct Merror_report_parser
      {
        Merror_report_parser( const char * msg_r )
        : msg( msg_r )
        {}

        typedef spirit::nil_t result_t;

        template <typename ScannerT>
          int operator()( const ScannerT & scan, result_t & /*result*/ ) const
          {
            SEC << scan.first.get_position() << ' ' << msg << std::endl;
            return -1; // Fail.
          }

        const char * msg;
      };

      typedef functor_parser<Merror_report_parser> Merror_report_p;




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

NVRA parseNVRA( const std::string & value )
{
  std::string n;
  std::string v;
  std::string r;
  std::string a;

  parse_info<> info = parse( value.c_str(),

       lexeme_d[(+~space_p)]                    [assign_a(n)]
       >> lexeme_d[(+(~space_p & ~ch_p('-')))]  [assign_a(v)]
       >> lexeme_d[(+(~space_p & ~ch_p('-')))]  [assign_a(r)]
       >> lexeme_d[(+~space_p)]                 [assign_a(a)]
       ,
                             space_p );

  NVRA data;
  if ( info.full )
    {
      data = NVRA( n, Edition(v,r), Arch(a) );
    }
  else
    {
      ERR << "parseNVRA failed on " << value << std::endl;
    }
  INT << data << endl;
  return data;
}


struct PConsume
{
  static bool isTag( const Tag & tag_r, const std::string & ident_r )
  {
    return tag_r.ident == ident_r && tag_r.ext.empty();
  }
  static bool isLangTag( const Tag & tag_r, const std::string & ident_r )
  {
    return tag_r.ident == ident_r && ! tag_r.ext.empty();
  }

  bool newPkg( const std::string & value )
  {
    NVRA data( parseNVRA( value ) );
    return true;
  }


  PConsume & operator=( const STag & stag_r )
  {
    if ( isTag( stag_r.stag, "Pkg" ) )
      {
        newPkg( stag_r.value );
        MIL << stag_r << endl;
      }
    return *this;
  }
  PConsume & operator=( const MTag & mtag_r )
  {
    return *this;
  }

  scoped_ptr<NVRA> _nvra;
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

  PConsume  consume;

#if 1
  rule_t file =   end_p
                | ( stag [var(consume)=arg1]
                  | mtag [var(consume)=arg1]
                  | ( *blank_p
                      >> !( ch_p('#')
                            >> *(anychar_p - eol_p)
                          )
                      >> (eol_p|end_p)
                    )
                    | error_report_p( "illegal line" )
                  )
                  >> file
                  ;
#else
  rule_t file =
            end_p
          | (+~space_p) [&echo]
            >> ( lazy_p(var(skip))
               | Merror_report_p( "lazy failed" )
               )
            >> file
          ;
#endif

  // Parse
  shared_ptr<Measure> duration( new Measure );
  parse_info<iterator_t> info
    = parse( begin, end,

             file

           );
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
