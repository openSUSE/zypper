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

#include "zypp/parser/tagfile/ParseException.h"

#include "zypp/CheckSum.h"
#include "zypp/Pathname.h"
#include "zypp/Product.h"
#include "zypp/source/susetags/SuseTagsProductImpl.h"
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
        Product::Ptr result;
        detail::ResImplTraits<SuseTagsProductImpl>::Ptr prodImpl;
        ProductMetadataParser();
        virtual ~ProductMetadataParser()
        {}

        /* Parse file and invoke consume on each tag found.
         * \throw ParseException
         * \todo more doc on Ecaptions.
        */
        void parse( const Pathname & file_r, Source_Ref source_r);
        /* Parse a key.modifier (std::list of std::strings)
         * That means, translatable tag with multiple values
         * the default modifier will get the modifier of default (LABEL.de, LABEL as LANGUAGE.default)
        */
        void parseLine( const std::string &key, const std::string &modif, const std::string &value, std::map< std::string, std::list<std::string> > &container);

        void parseLine( const std::string &key, const std::string &lang, const std::string &value, TranslatedText &container);
        /*
         * same as above, but the value is a single std::string, this means, translatable tags, with only 1 value
        */
        void parseLine( const std::string &key,const std::string &modif, const std::string &value, std::map< std::string, std::string > &container);
        /*
         * Non translatable tag with multiple values
         */
        void parseLine( const std::string &key, const std::string &value, std::list<std::string> &container);
        /*
         * Dependency (REQUIRES, PROVIDES, CONFLICTS, ...) capabilites  line
         */
        void parseDependencies( const std::string &key, const std::string &value, zypp::Dependencies & deps, zypp::Dep deptag );
        /*
         */
        void parseFileCheckSum( const std::string &key, const std::string &value, std::map<std::string, CheckSum> &container);

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
