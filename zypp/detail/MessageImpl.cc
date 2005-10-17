/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/MessageImpl.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/detail/MessageImpl.h"
#include "zypp/Message.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MessageImpl
    //
    ///////////////////////////////////////////////////////////////////

    /** Default ctor */
    MessageImpl::MessageImpl( const ResName & name_r,
			      const Edition & edition_r,
			      const Arch & arch_r )
    : ResolvableImpl (ResKind ("message"),
		      ResName (name_r),
		      Edition (edition_r),
		      Arch (arch_r))
    {
    }
    /** Dtor */
    MessageImpl::~MessageImpl()
    {
    }

    std::string MessageImpl::type () const {
      return _type;
    }

    std::string MessageImpl::text () const {
      return _text;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
