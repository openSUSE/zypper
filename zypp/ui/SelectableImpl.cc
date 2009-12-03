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
      StatusHelper( const Selectable::Impl & impl, ResStatus::TransactByValue causer_r )
      : _impl( impl )
      , inst( impl.installedObj() )
      , cand( impl.candidateObj() )
      , causer( causer_r )
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

      /** \name Topevel methods must restore status on failure. */
      //@{
      bool setInstall()
      {
        if ( cand )
        {
          if ( inst ) {
            ResStatus & inststatus( backup( inst.status() ) );
            if ( ! inststatus.setTransact( false, causer ) ) return restore();
            if ( ! inststatus.setLock    ( false, causer ) ) return restore();
            if ( ! cand->installOnly() )
            {
              // This is what the solver most probabely will do.
              // If we are wrong the solver will correct it. But
              // this way we will get a better disk usage result,
              // even if no autosolving is on.
              inststatus.setTransact( true, ResStatus::SOLVER );
            }
          }
          if ( ! unlockCandidates() ) return restore();
          ResStatus & candstatus( backup( cand.status() ) );
          if ( ! candstatus.setTransact( true, causer ) ) return restore();
          return true;
        }
        return false;
      }

      bool setDelete()
      {
        if ( inst )
        {
          if ( ! resetTransactingCandidates() ) return restore();
          ResStatus & inststatus( backup( inst.status() ) );
          if ( ! inststatus.setLock( false, causer ) ) return restore();
          if ( ! inststatus.setTransact( true, causer ) ) return restore();
          return true;
        }
        return false;
      }

      bool unset()
      {
        if ( inst )
        {
          ResStatus & inststatus( backup( inst.status() ) );
          if ( ! inststatus.setTransact( false, causer ) ) return restore();
          if ( ! inststatus.setLock( false, causer ) ) return restore();
        }
        if ( ! unlockCandidates() ) return restore();
        return true;
      }

      bool setProtected()
      {
        if ( causer != ResStatus::USER ) // by user only
          return false;

        if ( inst ) {
          resetTransactingCandidates();
          inst.status().setTransact( false, causer );
          return inst.status().setLock( true, causer );
        } else
          return false;
      }

      bool setTaboo()
      {
        if ( causer != ResStatus::USER ) // by user only
          return false;

        if ( cand ) {
          lockCandidates();
          return true;
        } else
          return false;
      }
      //@}

    private:
      /** \name Helper methods backup status but do not replay. */
      //@{
      bool resetTransactingCandidates()
      {
        for_( it, _impl.availableBegin(), _impl.availableEnd() )
        {
          ResStatus & status( backup( (*it).status() ) );
          if ( ! status.setTransact( false, causer ) ) return false;
        }
        return true;
      }
      bool unlockCandidates()
      {
        for_( it, _impl.availableBegin(), _impl.availableEnd() )
        {
          ResStatus & status( backup( (*it).status() ) );
          if ( ! status.setTransact( false, causer ) ) return false;
          if ( ! status.setLock( false, causer ) ) return false;
        }
        return true;
      }
      bool lockCandidates()
      {
        for_( it, _impl.availableBegin(), _impl.availableEnd() )
        {
          ResStatus & status( backup( (*it).status() ) );
          if ( ! status.setTransact( false, causer ) ) return false;
          if ( ! status.setLock( true, causer ) ) return false;
        }
        return true;
      }
      //@}

    private:
      const Selectable::Impl & _impl;
      PoolItem                   inst;
      PoolItem                   cand;
      ResStatus::TransactByValue causer;

    private:
      // No backup replay needed if causer is user,
      // because actions should always succeed.

      ResStatus & backup( ResStatus & status_r )
      {
        if ( causer != ResStatus::USER )
          _backup.push_back( status_r );
        return status_r;
      }

      bool restore()
      {
        if ( causer != ResStatus::USER )
        {
          for_( rit, _backup.rbegin(), _backup.rend() )
          {
            rit->replay();
          }
        }
        return false; // restore is done on error - return restore();
      }

      std::vector<resstatus::StatusBackup> _backup;
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

      // KEEP state:
      if ( installedObj() )
        return S_KeepInstalled;
      // Report pseudo installed items as installed, if they are satisfied.
      if ( traits::isPseudoInstalled( kind() )
           && cand.status().isSatisfied() ) // no installed, so we must have candidate
        return S_KeepInstalled;

      return S_NoInst;
    }

    bool Selectable::Impl::setStatus( const Status state_r, ResStatus::TransactByValue causer_r )
    {
      StatusHelper self( *this, causer_r );

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

    PoolItem Selectable::Impl::setCandidate( const PoolItem & newCandidate_r, ResStatus::TransactByValue causer_r )
    {
      PoolItem newCandidate;

      if ( newCandidate_r ) // must be in available list
      {
        for_( it, availableBegin(), availableEnd() )
        {
          if ( *it == newCandidate_r )
          {
            newCandidate = *it;
            break;
          }
        }
      }

      if ( newCandidate )
      {
        PoolItem trans( transactingCandidate() );
        if ( trans && trans != newCandidate )
        {
          // adjust transact to the new cancidate
          if (    trans.status().maySetTransact( false, causer_r )
               && newCandidate.status().maySetTransact( true, causer_r ) )
          {
            trans.status().setTransact( false, causer_r );
            newCandidate.status().setTransact( true, causer_r );
          }
          else
          {
            // No permission to change a transacting candidate.
            // Leave _candidate untouched and return NULL.
            return PoolItem();
          }
        }
      }

      return _candidate = newCandidate;
    }


    bool Selectable::Impl::pickInstall( const PoolItem & pi_r, ResStatus::TransactByValue causer_r, bool yesno_r )
    {
      if ( pi_r.satSolvable().ident() != ident() || pi_r.satSolvable().isSystem() )
        return false; // not my PoolItem or an installed one

      return pi_r.status().setTransact( yesno_r, causer_r );
    }

    bool Selectable::Impl::pickDelete( const PoolItem & pi_r, ResStatus::TransactByValue causer_r, bool yesno_r )
    {
      if ( pi_r.satSolvable().ident() != ident() || ! pi_r.satSolvable().isSystem() )
        return false; // not my PoolItem or not installed

      return pi_r.status().setTransact( yesno_r, causer_r );
    }

    ///////////////////////////////////////////////////////////////////

    Status Selectable::Impl::pickStatus( const PoolItem & pi_r ) const
    {
      if ( pi_r.satSolvable().ident() == ident() )
      {
        if ( pi_r.satSolvable().isSystem() )
        {
          // have installed!
          if ( pi_r.status().isLocked() )
            return S_Protected;

          // at least one identical available transacing?
          for_( it, _availableItems.begin(), _availableItems.end() )
          {
            if ( identical( *it, pi_r ) )
            {
              if ( (*it).status().transacts() )
                return( (*it).status().isByUser() ? S_Update : S_AutoUpdate );
            }
          }

          // no update, so maybe delete?
          if ( pi_r.status().transacts() )
            return ( pi_r.status().isByUser() ? S_Del : S_AutoDel );

          // keep
          return S_KeepInstalled;
        }
        else
        {
          // have available!
          if ( pi_r.status().isLocked() )
            return S_Taboo;

          // have identical installed? (maybe transacting):
          PoolItem inst;
          for_( it, _installedItems.begin(), _installedItems.end() )
          {
            if ( identical( *it, pi_r ) )
            {
              if ( (*it).status().transacts() )
              {
                inst = *it;
                break;
              }
              if ( !inst )
                inst = *it;
            }
          }

          // check for inst/update
          if ( pi_r.status().transacts() )
          {
            if ( inst )
              return( pi_r.status().isByUser() ? S_Update : S_AutoUpdate );
            else
              return( pi_r.status().isByUser() ? S_Install : S_AutoInstall );
          }

          // no inst/update, so maybe delete?
          if ( ! inst )
            return  S_NoInst;

          if ( inst.status().transacts() )
            return( inst.status().isByUser() ? S_Del : S_AutoDel );

          return S_KeepInstalled;
        }
      }
      return Status(-1); // not my PoolItem
    }

    ///////////////////////////////////////////////////////////////////

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
