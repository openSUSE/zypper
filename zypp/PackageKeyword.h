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

#include "zypp/base/UniqueString.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PackageKeyword
  //
  /** Package keywords. */
  struct PackageKeyword : public base::UniqueString<PackageKeyword>
  {
    PackageKeyword()
    {}

    PackageKeyword( const std::string & name_r )
      : base::UniqueString<PackageKeyword>( name_r )
    {}
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PACKAGEKEYWORD_H
