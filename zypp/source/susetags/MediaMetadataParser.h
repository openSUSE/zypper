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
#ifndef ZYPP_PARSER_TAGFILE_MediaMetadataPARSER_H
#define ZYPP_PARSER_TAGFILE_MediaMetadataPARSER_H

#include <iosfwd>
#include <set>
#include <map>
#include <list>

#include "zypp/parser/tagfile/ParseException.h"

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
      //	CLASS NAME : MediaMetadataParser
      //
      /** Tagfile parser. */
      struct MediaMetadataParser
      {
        struct MediaEntry {
          std::string vendor;
          std::string timestamp;
          unsigned int count;
          std::set<std::string> flags;
          // map media number to ( map language -> description string )
          // entry.alternate_names[1]["de"] -> "SUSE Linux"
          std::map< unsigned int, std::map<std::string, std::string> > alternate_names;
        };

        virtual ~MediaMetadataParser()
        {}

        /* Parse file and invoke consume on each tag found.
         * \throw ParseException
         * \todo more doc on Ecaptions.
        */
        void parse( const Pathname & file_r, MediaEntry &entry_r );
        /* Parse a key.modifier (std::list of std::strings)
         * That means, translatable tag with multiple values
         * the default modifier will get the modifier of default (LABEL.de, LABEL as LANGUAGE.default)
         */ 
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace tagfile
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
//
#endif //  ZYPP_PARSER_TAGFILE_MediaMetadataPPARSER_H
