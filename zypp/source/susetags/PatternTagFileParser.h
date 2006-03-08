/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/PatternTagFileParser.h
 *
*/
#ifndef ZYPP_PARSER_TAGFILE_PATTERNTAGFILEPARSER_H
#define ZYPP_PARSER_TAGFILE_PATTERNTAGFILEPARSER_H

#include <iosfwd>
#include <set>
#include <map>
#include <list>

#include "zypp/parser/tagfile/TagFileParser.h"
#include "zypp/parser/tagfile/ParseException.h"
#include "zypp/Pattern.h"
#include "zypp/source/susetags/SuseTagsPatternImpl.h"

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
      //	CLASS NAME : PatternTagFileParser
      //
      /** Tagfile parser. */
      struct PatternTagFileParser : public zypp::parser::tagfile::TagFileParser
      {
        Pattern::Ptr result;
        detail::ResImplTraits<SuseTagsPatternImpl>::Ptr patImpl;

        PatternTagFileParser();
        virtual ~PatternTagFileParser()
        {}

        void consume( const SingleTag &tag );
        void consume( const MultiTag &tag );
        void endParse();
      };
      ///////////////////////////////////////////////////////////////////
      /**
       * returns null if parse fails
       */
      Pattern::Ptr parsePattern( const Pathname & file_r );
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
#endif //  ZYPP_PARSER_TAGFILE_PATTERNTAGFILEPPARSER_H
