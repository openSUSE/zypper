/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/Parser.h
 *
*/
#ifndef ZYPP_PARSER_TAGFILE_SelectionSelFilePARSER_H
#define ZYPP_PARSER_TAGFILE_SelectionSelFilePARSER_H

#include <iosfwd>
#include <set>
#include <map>
#include <list>

#include "zypp/parser/tagfile/Tags.h"
#include "zypp/parser/tagfile/ParseException.h"
#include "zypp/Selection.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SelectionSelFileParser
      //
      /** Tagfile parser. */
      struct SelectionSelFileParser
      {
        std::list<Selection::Ptr> result;
        shared_ptr<detail::SelectionImpl> selImpl;

        struct MultiTag
        {
          std::string name;
          std::string modifier;
          std::set<std::string> values;
        };

        struct SingleTag
        {
          std::string name;
          std::string modifier;
          std::string value;
        };

        virtual ~SelectionSelFileParser()
        {}

        /* Parse file and invoke consume on each tag found.
         * \throw ParseException
         * \todo more doc on Ecaptions.
        */
        void parse( const Pathname & file_r);
        void consume( const SingleTag &tag );
        void consume( const MultiTag &tag );
      };
      ///////////////////////////////////////////////////////////////////
      std::list<Selection::Ptr> parseSelections( const Pathname & file_r );
      /////////////////////////////////////////////////////////////////
    } // namespace source
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace susetags
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
//
#endif //  ZYPP_PARSER_TAGFILE_SelectionSelFilePPARSER_H
