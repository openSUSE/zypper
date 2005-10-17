/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ResolvableImpl.cc
 *
*/
#include <iostream>

#include "zypp/detail/ResolvableImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ResolvableImpl::ResolvableImpl
    //	METHOD TYPE : Ctor
    //
    ResolvableImpl::ResolvableImpl( const ResKind & kind_r,
                                    const ResName & name_r,
                                    const Edition & edition_r,
                                    const Arch & arch_r )
    : _kind( kind_r )
    , _name( name_r )
    , _edition( edition_r )
    , _arch( arch_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ResolvableImpl::~ResolvableImpl
    //	METHOD TYPE : Dtor
    //
    ResolvableImpl::~ResolvableImpl()
    {}

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const ResolvableImpl & obj )
    {
      str << '[' << obj.kind() << ']' << obj.name() << '-' << obj.edition() << '.' << obj.arch();
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
