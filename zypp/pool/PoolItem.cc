/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/pool/PoolItem.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/pool/PoolItem.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PoolItem::Impl
    //
    /** PoolItem implementation. */
    struct PoolItem::Impl
    {
      Impl( ResObject::constPtr res_r,
            const ResStatus & status_r = ResStatus() )
      : _status( status_r )
      , _resolvable( res_r )
      {}

      ResStatus & status() const
      { return _status; }

      ResObject::constPtr resolvable() const
      { return _resolvable; }

    private:
      mutable ResStatus   _status;
      ResObject::constPtr _resolvable;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates PoolItem::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const PoolItem::Impl & obj )
    {
      return str << "PoolItem::Impl";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PoolItem
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PoolItem::PoolItem
    //	METHOD TYPE : Ctor
    //
    PoolItem::PoolItem()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PoolItem::PoolItem
    //	METHOD TYPE : Ctor
    //
    PoolItem::PoolItem( ResObject::constPtr res_r )
    : _pimpl( new Impl( res_r ) )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PoolItem::PoolItem
    //	METHOD TYPE : Ctor
    //
    PoolItem::PoolItem( ResObject::constPtr res_r, const ResStatus & status_r )
    : _pimpl( new Impl( res_r, status_r ) )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PoolItem::~PoolItem
    //	METHOD TYPE : Dtor
    //
    PoolItem::~PoolItem()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	Forward to Impl:
    //
    ///////////////////////////////////////////////////////////////////

    ResStatus & PoolItem::status() const
    { return _pimpl->status(); }

    ResObject::constPtr PoolItem::resolvable() const
    { return _pimpl->resolvable(); }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const PoolItem & obj )
    {
      return str << *obj._pimpl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
