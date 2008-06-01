/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/SelectableImpl.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/ui/SelectableImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : StatusHelper
    //
    /**
    */
    struct StatusHelper
    {
      StatusHelper( const Selectable::Impl & impl )
      : _impl( impl )
      , inst( impl.installedObj() )
      , cand( impl.candidateObj() )
      {}

      typedef Selectable::Impl::available_const_iterator available_const_iterator;

      //
      // Queries
      //
      bool hasInstalled() const
      { return inst; }

      bool hasCandidate() const
      { return cand; }

      bool hasInstalledOnly() const
      { return inst && !cand; }

      bool hasCandidateOnly() const
      { return cand && !inst; }

      bool hasBoth() const
      { return inst && cand; }

      //
      // ResStatus manip
      //
      void resetTransactingCandidates() const
      {
        for ( available_const_iterator it = _impl.availableBegin();
              it != _impl.availableEnd(); ++it )
          {
            (*it).status().setTransact( false, ResStatus::USER );
          }
      }
      void unlockCandidates() const
      {
        for ( available_const_iterator it = _impl.availableBegin();
              it != _impl.availableEnd(); ++it )
          {
            (*it).status().setTransact( false, ResStatus::USER );
            (*it).status().setLock( false, ResStatus::USER );
          }
      }
      void lockCandidates() const
      {
        for ( available_const_iterator it = _impl.availableBegin();
              it != _impl.availableEnd(); ++it )
          {
            (*it).status().setTransact( false, ResStatus::USER );
            (*it).status().setLock( true, ResStatus::USER );
          }
      }

      bool setInstall() const
      {
        if ( cand )
          {
	      if ( inst ) {
		  inst.status().setTransact( false, ResStatus::USER );
		  inst.status().setLock    ( false, ResStatus::USER );
                  if ( ! cand->installOnly() )
                  {
                    // This is what the solver most probabely will do.
                    // If we are wrong the solver will correct it. But
                    // this way we will get a better disk usage result,
                    // even if no autosolving is on.
                    inst.status().setTransact( true,  ResStatus::SOLVER );
                  }
	      }
              unlockCandidates();
	      return cand.status().setTransact( true, ResStatus::USER );
          }
        return false;
      }

      bool setDelete() const
      {
        if ( inst )
          {
            resetTransactingCandidates();
	    inst.status().setLock( false, ResStatus::USER );
            return inst.status().setTransact( true, ResStatus::USER );
          }
        return false;
      }

      bool unset() const
      {
	  if ( inst ) {
	      inst.status().setTransact( false, ResStatus::USER );
	      inst.status().setLock( false, ResStatus::USER );
	  }
          unlockCandidates();
	  return true;
      }

      bool setProtected() const
      {
	  if ( inst ) {
              resetTransactingCandidates();
	      inst.status().setTransact( false, ResStatus::USER );
	      return inst.status().setLock( true, ResStatus::USER );
	  } else
	      return false;
      }

      bool setTaboo() const
      {
	  if ( cand ) {
              lockCandidates();
	      return true;
	  } else
	      return false;
      }


    public:
      const Selectable::Impl & _impl;
      PoolItem inst;
      PoolItem cand;
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Selectable::Impl
    //
    ///////////////////////////////////////////////////////////////////

    Status Selectable::Impl::status() const
    {
      PoolItem cand( candidateObj() );
      if ( cand && cand.status().transacts() )
        {
          if ( cand.status().isByUser() )
            return( installedObj() ? S_Update : S_Install );
          else
            return( installedObj() ? S_AutoUpdate : S_AutoInstall );
        }

      if ( installedObj() && installedObj().status().transacts() )
        {
          return( installedObj().status().isByUser() ? S_Del : S_AutoDel );
        }

      if ( installedObj() && installedObj().status().isLocked() )
	  return S_Protected;

      if ( !installedObj() && allCandidatesLocked() )
	  return S_Taboo;

      // KEEP state: non packages count as installed if they are satisfied.
      if ( installedObj() )
          return S_KeepInstalled;

      if ( kind() != ResKind::package
           && cand.status().isSatisfied() ) // no installed, so we must have candidate
          return S_KeepInstalled;

      return S_NoInst;
    }

    bool Selectable::Impl::setStatus( const Status state_r )
    {
      StatusHelper self( *this );

      switch ( state_r )
        {
        case S_Protected:
	    return self.setProtected();
        case S_Taboo:
	    return self.setTaboo();
        case S_AutoDel:
        case S_AutoInstall:
        case S_AutoUpdate:
          // Auto level is SOLVER level. UI may query, but not
          // set at this level.
          break;

        case S_Del:
          return self.setDelete();
          break;

        case S_Install:
          return self.hasCandidateOnly() && self.setInstall();
          break;

        case S_Update:
          return self.hasBoth() && self.setInstall();
          break;

        case S_KeepInstalled:
          return self.hasInstalled() && self.unset();
          break;

        case S_NoInst:
          return !self.hasInstalled() && self.unset();
          break;
        }

      return false;
    }

    PoolItem Selectable::Impl::setCandidate( ResObject::constPtr byUser_r )
    {
      _candidate = PoolItem();

      if ( byUser_r ) // must be in available list
        {
          for ( available_const_iterator it = availableBegin();
                it != availableEnd(); ++it )
            {
              if ( it->resolvable() == byUser_r )
                {
                  _candidate = *it;
                  break;
                }
            }
        }

      if ( _candidate )
        {
          PoolItem trans( transactingCandidate() );
          if ( trans && trans != _candidate )
            {
              // adjust transact to the new cancidate
              trans.status().setTransact( false, ResStatus::USER );
              _candidate.status().setTransact( true, ResStatus::USER );
            }
        }

      return _candidate;
    }

    ResStatus::TransactByValue Selectable::Impl::modifiedBy() const
    {
      PoolItem cand( candidateObj() );
      if ( cand && cand.status().transacts() )
        return cand.status().getTransactByValue();

      if ( installedObj() && installedObj().status().transacts() )
        return installedObj().status().getTransactByValue();

      if ( cand )
        return cand.status().getTransactByValue();

      if ( installedObj() )
        return installedObj().status().getTransactByValue();

      return ResStatus::SOLVER;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
