/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/Parser.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/parser/tagfile/Parser.h"
#include "zypp/parser/tagfile/Grammar.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace tagfile
    { /////////////////////////////////////////////////////////////////

      // translate operator= into Parser::consume calls
      struct ParserConnect
      {
        ParserConnect( Parser & consumer_r )
        : _consumer( consumer_r )
        {}

        const ParserConnect & operator=( const STag & stag_r ) const
        { _consumer.consume( stag_r ); return *this; }

        const ParserConnect & operator=( const MTag & mtag_r ) const
        { _consumer.consume( mtag_r ); return *this; }

        Parser & _consumer;
      };

      static void doParse( iterator_t begin_r, iterator_t end_r,
                           const ParserConnect & consumer_r );

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Parser::parse
      //	METHOD TYPE : void
      //
      void Parser::parse( const Pathname & file_r )
      {
        // Create a file iterator for this file
        fiterator_t first( file_r.asString() );
        if (!first)
          {
            ZYPP_THROW( ParseException( std::string("Unable to open file ")
                                        + file_r.asString() ) );
          }
        // Create an EOF iterator
        fiterator_t last = first.make_end();

        // Create position iterators
        iterator_t begin( first, last, file_r.asString() );
        iterator_t end;

        // go
        parseBegin();
        doParse( begin, end, *this );
        parseEnd();
      }

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

      ///////////////////////////////////////////////////////////////////
      //
      //	doParse
      //
      ///////////////////////////////////////////////////////////////////

      static void doParse( iterator_t begin_r,
                           iterator_t end_r,
                           const ParserConnect & consumer_r )
      {
        SingleTag stag;
        MultiTag  mtag;

        rule_t file =   end_p
                      | ( stag [var(consumer_r)=arg1]
                        | mtag [var(consumer_r)=arg1]
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

        shared_ptr<Measure> duration( new Measure );
        parse_info<iterator_t> info = parse( begin_r, end_r, file );
        duration.reset();

        if ( info.full )
          USR << "Parse succeeded!\n";
        else
          {
            if ( info.hit )
              {
                ERR << "Parse partial!\n";
                ERR << " at pos " << info.length << endl;
              }
            else
              {
                ERR << "Parse failed!\n";
                ERR << " at pos " << info.length << endl;
              }
            ZYPP_THROW( ParseException("Parse failed") );
          }
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
