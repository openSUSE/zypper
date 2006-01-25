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

#include "zypp/parser/tagfile/Tags.h"
#include "zypp/parser/tagfile/ParseException.h"

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

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : MediaMetadataParser
      //
      /** Tagfile parser. */
      struct MediaMetadataParser
      {
        struct MediaEntry {
          Pathname    dir;
          std::string vendor;
          std::string timestamp;
          unsigned int count;
          std::set<std::string> flags;
          // map media number to ( map language -> description string )
          // entry.alternate_names[1]["de"] -> "SUSE Linux"
          std::map< unsigned int, std::map<std::string, std::string> > alternate_names;
              
          MediaEntry( const Pathname & dir_r = "/" ){
            dir  = dir_r;
          }
          bool operator<( const MediaEntry & rhs ) const {
            return( dir.asString() < rhs.dir.asString() );
          }
        };

        typedef std::set<MediaEntry> MediaSet;


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
        void parseLine( const std::string &key, const std::string &modif, const std::string &value, std::map< std::string, std::list<std::string> > &container);
        /*
         * same as above, but the value is a single std::string, this means, translatable tags, with only 1 value
        */
        void parseLine( const std::string &key,const std::string &modif, const std::string &value, std::map< std::string, std::string > &container);
        /*
         * Non translatable tag with multiple values
         */
        void parseLine( const std::string &key, const std::string &value, std::list<std::string> &container);
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
