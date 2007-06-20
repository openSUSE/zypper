/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/cache/Attribute.h
 *
*/
#ifndef ZYPP_CACHE_ATTRIBUTE_H
#define ZYPP_CACHE_ATTRIBUTE_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Attribute
    //
    /** Attribute klass and name definition.
     *
     * \ref Attribute defines the klass and name value \ref CacheStore
     * uses to write an attribute to the database. The same pair is
     * required to query the attributes value.
    */
    struct Attribute
    {
      Attribute( const std::string & klass_r, const std::string & name_r )
        : klass(klass_r), name(name_r)
      {}
      std::string klass;
      std::string name;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Attribute Stream output */
    std::ostream & operator<<( std::ostream & str, const Attribute & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CACHE_ATTRIBUTE_H
