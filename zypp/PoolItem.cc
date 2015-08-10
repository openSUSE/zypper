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
#include "zypp/base/DefaultIntegral.h"

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
  /** PoolItem implementation.
   * \c _buddy handling:
   * \li \c ==0 no buddy
   * \li \c >0 this uses \c _buddy status
   * \li \c <0 this status used by \c -_buddy
   */
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
      { return _buddy > 0 ? PoolItem(buddy()).status() : _status; }

      sat::Solvable buddy() const
      {
        if ( !_buddy )
          return sat::Solvable::noSolvable;
        if ( _buddy < 0 )
          return sat::Solvable( -_buddy );
        return sat::Solvable( _buddy );
      }

      void setBuddy( const sat::Solvable & solv_r );

      ResObject::constPtr resolvable() const
      { return _resolvable; }

      ResStatus & statusReset() const
      {
        _status.setLock( false, zypp::ResStatus::USER );
        _status.resetTransact( zypp::ResStatus::USER );
        return _status;
      }

    public:
      bool isUndetermined() const
      {
	  return status().isUndetermined();
      }

      bool isRelevant() const
      {
	  return !status().isNonRelevant();
      }

      bool isSatisfied() const
      {
	  return status().isSatisfied();
      }

      bool isBroken() const
      {
	  return status().isBroken();
      }

      bool isNeeded() const
      {
	return status().isToBeInstalled() || ( isBroken() && ! status().isLocked() );
      }

      bool isUnwanted() const
      {
	return isBroken() && status().isLocked();
      }

    private:
      mutable ResStatus     _status;
      ResObject::constPtr   _resolvable;
      DefaultIntegral<sat::detail::IdType,sat::detail::noId> _buddy;

    /** \name Poor man's save/restore state.
       * \todo There may be better save/restore state strategies.
     */
    //@{
    public:
      void saveState() const
      { _savedStatus = status(); }
      void restoreState() const
      { status() = _savedStatus; }
      bool sameState() const
      {
        if ( status() == _savedStatus )
          return true;
        // some bits changed...
        if ( status().getTransactValue() != _savedStatus.getTransactValue()
             && ( ! status().isBySolver() // ignore solver state changes
                  // removing a user lock also goes to bySolver
                  || _savedStatus.getTransactValue() == ResStatus::LOCKED ) )
          return false;
        if ( status().isLicenceConfirmed() != _savedStatus.isLicenceConfirmed() )
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

  inline void PoolItem::Impl::setBuddy( const sat::Solvable & solv_r )
  {
    PoolItem myBuddy( solv_r );
    if ( myBuddy )
    {
      myBuddy._pimpl->_buddy = -resolvable()->satSolvable().id();
      _buddy = myBuddy.satSolvable().id();
      DBG << *this << " has buddy " << myBuddy << endl;
    }
  }

  ///////////////////////////////////////////////////////////////////
  //	class PoolItem
  ///////////////////////////////////////////////////////////////////

  PoolItem::PoolItem()
  : _pimpl( Impl::nullimpl() )
  {}

  PoolItem::PoolItem( const sat::Solvable & solvable_r )
  : _pimpl( ResPool::instance().find( solvable_r )._pimpl )
  {}

  PoolItem::PoolItem( const ResObject::constPtr & resolvable_r )
  : _pimpl( ResPool::instance().find( resolvable_r )._pimpl )
  {}

  PoolItem::PoolItem( Impl * implptr_r )
  : _pimpl( implptr_r )
  {}

  PoolItem PoolItem::makePoolItem( const sat::Solvable & solvable_r )
  {
    return PoolItem( new Impl( makeResObject( solvable_r ), solvable_r.isSystem() ) );
  }

  PoolItem::~PoolItem()
  {}

  ResPool PoolItem::pool() const
  { return ResPool::instance(); }


  ResStatus & PoolItem::status() const			{ return _pimpl->status(); }
  ResStatus & PoolItem::statusReset() const		{ return _pimpl->statusReset(); }
  sat::Solvable PoolItem::buddy() const			{ return _pimpl->buddy(); }
  void PoolItem::setBuddy( const sat::Solvable & solv_r )	{ _pimpl->setBuddy( solv_r ); }
  bool PoolItem::isUndetermined() const			{ return _pimpl->isUndetermined(); }
  bool PoolItem::isRelevant() const			{ return _pimpl->isRelevant(); }
  bool PoolItem::isSatisfied() const			{ return _pimpl->isSatisfied(); }
  bool PoolItem::isBroken() const			{ return _pimpl->isBroken(); }
  bool PoolItem::isNeeded() const			{ return _pimpl->isNeeded(); }
  bool PoolItem::isUnwanted() const			{ return _pimpl->isUnwanted(); }
  void PoolItem::saveState() const			{ _pimpl->saveState(); }
  void PoolItem::restoreState() const			{ _pimpl->restoreState(); }
  bool PoolItem::sameState() const			{ return _pimpl->sameState(); }
  ResObject::constPtr PoolItem::resolvable() const	{ return _pimpl->resolvable(); }


  std::ostream & operator<<( std::ostream & str, const PoolItem & obj )
  { return str << *obj._pimpl; }

} // namespace zypp
///////////////////////////////////////////////////////////////////
