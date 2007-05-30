/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/MediaPatchesMetadataParser.h
 *
*/
#ifndef ZYPP_PARSER_TAGFILE_MediaPatchesMetadataPARSER_H
#define ZYPP_PARSER_TAGFILE_MediaPatchesMetadataPARSER_H

#include <iosfwd>
#include <set>
#include <map>
#include <list>

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

      /*
        Location  /media.1/ directory
        Content  one line of ASCII as follows
        <directory> <whitespace> <optional comment>
        zero or more lines specifying exclusive products: <productname>-<productversion>
      */

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : MediaPatchesMetadataParser
      //
      /** Tagfile parser. */
      struct MediaPatchesMetadataParser
      {
        struct MediaPatchesEntry
        {
          Pathname dir;
          std::string comment;
          // set of pairs (productname, version)
          std::set< std::pair<std::string, std::string> > products;
        };

        virtual ~MediaPatchesMetadataParser()
      {}

        /* Parse file and invoke consume on each tag found.
         * \throw ParseException
         * \todo more doc on Ecaptions.
        */
        void parse( const Pathname & file_r, MediaPatchesEntry &entry_r );
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
#endif //  ZYPP_PARSER_TAGFILE_MediaPatchesMetadataPPARSER_H
