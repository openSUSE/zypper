/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/TagFileParser.h
 *
*/
#ifndef ZYPP_PARSER_TAGFILE2_H
#define ZYPP_PARSER_TAGFILE2_H

#include <iosfwd>
#include <map>
#include <list>

#include <boost/regex.hpp>

#include "zypp/parser/ParserProgress.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace tagfile
    { /////////////////////////////////////////////////////////////////

      void dumpRegexpResults( const boost::smatch &what );
      void dumpRegexpResults2( const boost::smatch &what );
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : TagFileParser
      //
      /** Tagfile parser. */
      class TagFileParser
      { 
        public:
        struct MultiTag
        {
          std::string name;
          std::string modifier;
          std::list<std::string> values;
        };

        struct SingleTag
        {
          std::string name;
          std::string modifier;
          std::string value;
        };

        TagFileParser( ParserProgress::Ptr progress );
        virtual ~TagFileParser()
        {}

        /* Parse file and invoke consume on each tag found.
         * \throw ParseException
         * \todo more doc on Ecaptions.
        */
        virtual void parse( const Pathname & file_r);

        /*
         * Called when start parsing
         */ 
        virtual void beginParse();
        /*
         * Called when a single tag is found
         */ 
        virtual void consume( const SingleTag &tag );
        /*
         * Called when a multiple tag is found
         */ 
        virtual void consume( const MultiTag &tag );
        /*
         * Called when the parse is done
         */ 
        virtual void endParse();
        
        protected:
          ParserProgress::Ptr _progress;
          Pathname _file_r;
          int _file_size;
          int _line_number;
      };
      /////////////////////////////////////////////////////////////////
    } // namespace parser
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace tagfile
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
//
#endif //  ZYPP_PARSER_TAGFILE_SelectionSelFilePPARSER_H
