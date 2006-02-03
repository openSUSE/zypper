/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/ProductMetadataParser.h
 *
*/
#ifndef ZYPP_SOURCE_SUSETAGS_PRODUCTMETADATAPARSER_H
#define ZYPP_SOURCE_SUSETAGS_PRODUCTMETADATAPARSER_H

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
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ProductMetadataParser
      //
      /** Tagfile parser. */
      struct ProductMetadataParser
      {
        struct ProductEntry {
          std::string name;
          std::string version;
          std::string dist;
          std::string dist_version;
          std::string base_product;
          std::string base_version;
          std::string you_type;
          std::string you_path;
          std::string you_url;
          std::string vendor;
          std::string release_notes_url;
          std::map< std::string, std::list<std::string> > arch;
          std::string default_base;
          std::list<std::string> requires;
          std::list<std::string> languages;
          std::map< std::string, std::string > label;
          std::string description_dir;
          std::string data_dir;
          std::list<std::string> flags;
          std::string language;
          std::string timezone;
        };

        virtual ~ProductMetadataParser()
        {}

        /* Parse file and invoke consume on each tag found.
         * \throw ParseException
         * \todo more doc on Ecaptions.
        */
        void parse( const Pathname & file_r, ProductEntry &entry_r );
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
#endif //  ZYPP_SOURCE_SUSETAGS_PRODUCTMETADATAPARSER_H
