/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PackageKeyword.h
 *
*/
#ifndef ZYPP_PACKAGEKEYWORD_H
#define ZYPP_PACKAGEKEYWORD_H

#include "zypp/IdStringType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PackageKeyword
  //
  /** Package keywords.
   * \see \ref IdStringType
   */
  class PackageKeyword : public IdStringType<PackageKeyword>
  {
    public:
      /** Default ctor: empty keyword */
      PackageKeyword() {}

      /** Ctor taking keyword as string. */
      explicit PackageKeyword( sat::detail::IdType id_r )  : _str( id_r ) {}
      explicit PackageKeyword( const IdString & idstr_r )  : _str( idstr_r ) {}
      explicit PackageKeyword( const std::string & str_r ) : _str( str_r ) {}
      explicit PackageKeyword( const char * cstr_r )       : _str( cstr_r ) {}

    private:
      friend class IdStringType<PackageKeyword>;
      IdString _str;
   };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PACKAGEKEYWORD_H
