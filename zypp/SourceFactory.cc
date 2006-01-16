/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceFactory.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/SourceFactory.h"
#include "zypp/source/Builtin.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceFactory::Impl
  //
  /** Currently not used: SourceFactory implementation. */
  struct SourceFactory::Impl
  {
  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceFactory
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceFactory::SourceFactory
  //	METHOD TYPE : Ctor
  //
  SourceFactory::SourceFactory()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceFactory::~SourceFactory
  //	METHOD TYPE : Dtor
  //
  SourceFactory::~SourceFactory()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceFactory::createFrom
  //	METHOD TYPE : Source
  //
  Source SourceFactory::createFrom( const Source::Impl_Ptr & impl_r )
  {
    if ( ! impl_r )
      ZYPP_THROW( Exception("NULL implementation passed to SourceFactory") );

    return Source( impl_r );
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const SourceFactory & obj )
  {
    return str << "SourceFactory";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
