/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/Tags.h
 *
*/
#ifndef ZYPP_PARSER_TAGFILE_TAGS_H
#define ZYPP_PARSER_TAGFILE_TAGS_H

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/parser/Spirit.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace tagfile
    { /////////////////////////////////////////////////////////////////

      using namespace spirit;
      using namespace phoenix;

      typedef char                           char_t;
      typedef file_iterator<char_t>          fiterator_t;
      typedef position_iterator<fiterator_t, file_position_and_offset> iterator_t;

      ////////////////////////////////////////////////////////////////////////////
      //
      //  Data types
      //
      ////////////////////////////////////////////////////////////////////////////

      struct Range
      {
        std::streamoff start;
        unsigned       length;

        Range()
        : start( 0 ), length( 0 )
        {}

        Range( const iterator_t & first, const iterator_t & last )
        : start( first.get_position().offset ), length( last.get_position().offset - start )
        {}
      };

      /** \relates Range Stream output.*/
      std::ostream & operator<<( std::ostream & str, const Range & obj );

      struct Tag
      {
        Range       range;
        std::string ident;
        std::string ext;

        bool isPlain() const
        { return ext.empty(); }

        bool hasLang() const
        { return !isPlain(); }

        bool isPlain( const std::string & val_r ) const
        { return isPlain() && ident == val_r; }

        bool isLang( const std::string & val_r ) const
        { return hasLang() && ident == val_r; }
      };

      /** \relates Tag Stream output.*/
      std::ostream & operator<<( std::ostream & str, const Tag & obj );

      struct STag
      {
        Tag         stag;
        Range       data;
        std::string value;
      };

      /** \relates STag Stream output.*/
      std::ostream & operator<<( std::ostream & str, const STag & obj );

      struct MTag
      {
        Tag         stag;
        Tag         etag;
        Range       data;
        std::list<std::string> value;
      };

      /** \relates MTag Stream output.*/
      std::ostream & operator<<( std::ostream & str, const MTag & obj );

      /////////////////////////////////////////////////////////////////
    } // namespace tagfile
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_TAGFILE_TAGS_H
