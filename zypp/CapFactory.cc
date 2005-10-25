/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapFactory.cc
 *
*/
#include <iostream>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/CapFactory.h"

#include "zypp/capability/FileCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CapFactoryImpl
    //
    /** CapFactory implementation */
    struct CapFactoryImpl : public base::ReferenceCounted, private base::NonCopyable
    {
      /** Default ctor*/
      CapFactoryImpl();
      /** Dtor */
      ~CapFactoryImpl();
    };
    ///////////////////////////////////////////////////////////////////
    IMPL_PTR_TYPE(CapFactoryImpl)
    ///////////////////////////////////////////////////////////////////

    /** \relates CapFactoryImpl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const CapFactoryImpl & obj )
    {
      return str << "CapFactoryImpl";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CapFactory
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::CapFactory
  //	METHOD TYPE : Ctor
  //
  CapFactory::CapFactory()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::CapFactory
  //	METHOD TYPE : Ctor
  //
  CapFactory::CapFactory( detail::CapFactoryImplPtr impl_r )
  : _pimpl( impl_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::~CapFactory
  //	METHOD TYPE : Dtor
  //
  CapFactory::~CapFactory()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::sayFriend
  //	METHOD TYPE : detail::constCapFactoryImplPtr
  //
  detail::constCapFactoryImplPtr CapFactory::sayFriend() const
  { return _pimpl; }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CapFactory::parse
  //	METHOD TYPE : Capability
  //
  Capability CapFactory::parse( const std::string & strval_r ) const
  {
    // fix
    return Capability( new capability::FileCap );
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CapFactory & obj )
  {
    return str << *obj.sayFriend();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
