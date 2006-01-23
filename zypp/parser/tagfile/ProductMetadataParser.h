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
#ifndef ZYPP_PARSER_TAGFILE_ProductMetadataPARSER_H
#define ZYPP_PARSER_TAGFILE_ProductMetadataPARSER_H

#include <iosfwd>
#include <set>

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
      //	CLASS NAME : ProductMetadataParser
      //
      /** Tagfile parser. */
      struct ProductMetadataParser
      {
        struct ProductEntry {
          Pathname    _dir;
          std::string _name;

          ProductEntry( const Pathname & dir_r = "/", const std::string & name_r = std::string() ){
            _dir  = dir_r;
            _name = name_r;
          }
          bool operator<( const ProductEntry & rhs ) const {
            return( _dir.asString() < rhs._dir.asString() );
          }
        };

        typedef std::set<ProductEntry> ProductSet;


        virtual ~ProductMetadataParser()
        {}

        /* Parse file and invoke consume on each tag found.
         * \throw ParseException
         * \todo more doc on Ecaptions.
        */
        void parse( const Pathname & file_r, ProductEntry &entry_r );
        
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
#endif // ZYPP_PARSER_TAGFILE_ProductMetadataPPARSER_H
