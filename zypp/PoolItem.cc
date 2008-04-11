/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PoolItem.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/PoolItem.h"
#include "zypp/ResPool.h"
#include "zypp/Package.h"
#include "zypp/VendorAttr.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PoolItem::Impl
  //
  /** PoolItem implementation. */
  struct PoolItem::Impl
  {
    public:
      Impl() {}

      Impl( ResObject::constPtr res_r,
            const ResStatus & status_r )
      : _status( status_r )
      , _resolvable( res_r )
      {}

      ResStatus & status() const
      { return _status; }

      ResObject::constPtr resolvable() const
      { return _resolvable; }

      ResStatus & statusReset() const
      {
        _status.setLock( false, zypp::ResStatus::USER );
        _status.resetTransact( zypp::ResStatus::USER );
        return _status;
      }

    public:

      bool isRelevant() const
      {
        return true;
      }

      bool isSatisfied() const
      {
        return true;
      }

      bool isBroken() const
      {
        return true;
      }

    private:
      mutable ResStatus     _status;
      ResObject::constPtr   _resolvable;

    /** \name Poor man's save/restore state.
       * \todo There may be better save/restore state strategies.
     */
    //@{
    public:
      void saveState() const
      { _savedStatus = _status; }
      void restoreState() const
      { _status = _savedStatus; }
      bool sameState() const
      {
        if (    _status.getTransactValue() != _savedStatus.getTransactValue()
                && !_status.isBySolver() )
          return false;
        if ( _status.isLicenceConfirmed() != _savedStatus.isLicenceConfirmed() )
          return false;
        return true;
      }
    private:
      mutable ResStatus _savedStatus;
    //@}

    public:
      /** Offer default Impl. */
      static shared_ptr<Impl> nullimpl()
      {
        static shared_ptr<Impl> _nullimpl( new Impl );
        return _nullimpl;
      }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PoolItem::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PoolItem::Impl & obj )
  {
    str << obj.status();
    if (obj.resolvable())
	str << *obj.resolvable();
    else
	str << "(NULL)";
    return str;
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
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PoolItem::PoolItem
  //	METHOD TYPE : Ctor
  //
  PoolItem::PoolItem( const sat::Solvable & solvable_r )
  : _pimpl( ResPool::instance().find( solvable_r )._pimpl )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PoolItem::PoolItem
  //	METHOD TYPE : Ctor
  //
  PoolItem::PoolItem( const ResObject::constPtr & resolvable_r )
  : _pimpl( ResPool::instance().find( resolvable_r )._pimpl )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PoolItem::PoolItem
  //	METHOD TYPE : Ctor
  //
  PoolItem::PoolItem( Impl * implptr_r )
  : _pimpl( implptr_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PoolItem::makePoolItem
  //	METHOD TYPE : PoolItem
  //
  PoolItem PoolItem::makePoolItem( const sat::Solvable & solvable_r )
  { return PoolItem( new Impl( makeResObject( solvable_r ), solvable_r.isSystem() ) ); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PoolItem::~PoolItem
  //	METHOD TYPE : Dtor
  //
  PoolItem::~PoolItem()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PoolItem::pool
  //	METHOD TYPE : ResPool
  //
  ResPool PoolItem::pool() const
  { return ResPool::instance(); }

  ///////////////////////////////////////////////////////////////////
  //
  //	Forward to Impl:
  //
  ///////////////////////////////////////////////////////////////////

  ResStatus & PoolItem::status() const
  { return _pimpl->status(); }

  ResStatus & PoolItem::statusReset() const
  { return _pimpl->statusReset(); }

  bool PoolItem::isRelevant() const
  { return _pimpl->isSatisfied(); }

  bool PoolItem::isSatisfied() const
  { return _pimpl->isSatisfied(); }

  bool PoolItem::isBroken() const
  { return _pimpl->isSatisfied(); }

  void PoolItem::saveState() const
  { _pimpl->saveState(); }

  void PoolItem::restoreState() const
  { _pimpl->restoreState(); }

  bool PoolItem::sameState() const
  { return _pimpl->sameState(); }

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
} // namespace zypp
///////////////////////////////////////////////////////////////////
