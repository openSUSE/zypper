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

    /** Simple ResStatus backup stack.
     * \ref restore simply rewinds all remembered status.
    */
    class StatusBackup
    {
      public:
        typedef ResStatus::TransactByValue Causer;

      public:
        /** Backup status. */
        ResStatus & backup( ResStatus & status_r )
        {
          _backup.push_back( status_r );
          return status_r;
        }
        /** \overload */
        ResStatus & backup( const PoolItem & pi_r )
        { return backup( pi_r.status() ); }

        /** Backup status. */
        ResStatus & operator()( ResStatus & status_r )
        { return backup( status_r ); }
        /** \overload */
        ResStatus & operator()( const PoolItem & pi_r )
        { return backup( pi_r.status() ); }

        /** Restore all status. */
        bool restore()
        {
          for_( rit, _backup.rbegin(), _backup.rend() )
            rit->replay();
          return false; // restore is done on error - return restore();
        }

      public:
        /** lowlevel \c ResStatus::setTransact */
        bool setTransact( const PoolItem & pi_r, bool yesno_r, Causer causer_r )
        { return backup( pi_r ).setTransact( yesno_r, causer_r ); }

        /** lowlevel \c ResStatus::setLock */
        bool setLock( const PoolItem & pi_r, bool yesno_r, Causer causer_r )
        { return backup( pi_r ).setLock( yesno_r, causer_r ); }

        /** lowlevel \c ResStatus::setTransact(true). */
        bool setTransactTrue( const PoolItem & pi_r, Causer causer_r )
        { return setTransact( pi_r, true, causer_r ); }

        /** lowlevel \c ResStatus::setTransact(false). */
        bool setTransactFalse( const PoolItem & pi_r, Causer causer_r )
        { return setTransact( pi_r, false, causer_r ); }

      public:
        /** highevel set transact (force unlock). */
        bool transact( const PoolItem & pi_r, Causer causer_r )
        {
          ResStatus & status( backup( pi_r ) );
          if ( ! status.setLock( false, causer_r ) ) return false;
          if ( ! status.setTransact( true, causer_r ) ) return false;
          return true;
        }

        /** highlevel set locked. */
        bool lock( const PoolItem & pi_r, Causer causer_r )
        {
          ResStatus & status( backup( pi_r ) );
          if ( ! status.setTransact( false, causer_r ) ) return false;
          if ( ! status.setLock( true, causer_r ) ) return false;
          return true;
        }

        /** highlevel unlock (also unsets transact). */
        bool unlock( const PoolItem & pi_r, Causer causer_r )
        {
          ResStatus & status( backup( pi_r ) );
          if ( ! status.setTransact( false, causer_r ) ) return false;
          if ( ! status.setLock( false, causer_r ) ) return false;
          return true;
        }

        /** Highlevel action. */
        typedef bool (StatusBackup::*Action)( const PoolItem &, Causer );

        /** Highlevel action on range of items. */
        template <class _Iter>
        bool forEach( _Iter begin_r, _Iter end_r, Action action_r, Causer causer_r )
        {
          for_( it, begin_r, end_r )
            if ( ! (this->*action_r)( *it, causer_r ) )
              return false;
          return true;
        }

      private:
        std::vector<resstatus::StatusBackup> _backup;
    };

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
            if ( ! cand->multiversionInstall() )
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
      bool restore() { return backup.restore(); }
      StatusBackup backup;
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

    bool Selectable::Impl::setStatus( Status state_r, ResStatus::TransactByValue causer_r )
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

    ///////////////////////////////////////////////////////////////////
    bool Selectable::Impl::setPickStatus( const PoolItem & pi_r, Status state_r, ResStatus::TransactByValue causer_r )
    {
      if ( pi_r.satSolvable().ident() == ident() )
        return false;  // not my PoolItem

      if ( ! multiversionInstall() )
        return false;  // We're not yet ready for this.
      // TODO: Without multiversionInstall take care at most ONE available is set
      // to install. Upon install ALL installed get deleted. Only upon deletetion
      // one might pick individual versions (but more than one would be an error here).

      StatusBackup backup;
      std::vector<PoolItem> i;
      std::vector<PoolItem> a;

      for_( it, installedBegin(), installedEnd() )
        if ( identical( *it, pi_r ) )
          i.push_back( *it );
      for_( it, availableBegin(), availableEnd() )
        if ( identical( *it, pi_r ) )
          i.push_back( *it );

      switch ( state_r )
      {
        case S_Protected:
          if ( causer_r == ResStatus::USER && ! i.empty() )
          {
            if ( ! backup.forEach( i.begin(), i.end(), &StatusBackup::lock, causer_r ) ) return backup.restore();
            if ( ! backup.forEach( a.begin(), a.end(), &StatusBackup::setTransactFalse, causer_r ) ) return backup.restore();
            return true;
          }
          break;

        case S_Taboo:
          if ( causer_r == ResStatus::USER && ! a.empty() )
          {
            if ( ! backup.forEach( a.begin(), a.end(), &StatusBackup::lock, causer_r ) ) return backup.restore();
            return true;
          }
          break;

        case S_AutoDel:
        case S_AutoInstall:
        case S_AutoUpdate:
          // Auto level is SOLVER level. UI may query, but not
          // set at this level.
          break;

        case S_Del:
          if ( ! i.empty() )
          {
            if ( ! backup.forEach( i.begin(), i.end(), &StatusBackup::transact, causer_r ) ) return backup.restore();
            if ( ! backup.forEach( a.begin(), a.end(), &StatusBackup::setTransactFalse, causer_r ) ) return backup.restore();
            return true;
          }
          break;

        case S_Install:
          if ( i.empty() && ! a.empty() )
          {
            // maybe unlock candidate only?
            if ( ! backup.forEach( a.begin(), a.end(), &StatusBackup::unlock, causer_r ) ) return backup.restore();
            const PoolItem & cand( pi_r.status().isInstalled() ? *a.begin() : pi_r ); // status already backed up above
            if ( ! cand.status().setTransact( true, causer_r ) ) return backup.restore();
            return true;
          }
          break;

        case S_Update:
          if ( ! i.empty() && ! a.empty() )
          {
            if ( ! backup.forEach( i.begin(), i.end(), &StatusBackup::unlock, causer_r ) ) return backup.restore();
            if ( ! backup.forEach( i.begin(), i.end(), &StatusBackup::setTransactTrue, ResStatus::SOLVER ) ) return backup.restore();

            return true;
          }
          break;

        case S_KeepInstalled:
          if ( ! i.empty()  )
          {
            if ( ! backup.forEach( i.begin(), i.end(), &StatusBackup::unlock, causer_r ) ) return backup.restore();
            if ( ! backup.forEach( a.begin(), a.end(), &StatusBackup::unlock, causer_r ) ) return backup.restore();
            return true;
          }
          break;

        case S_NoInst:
          if ( i.empty()  )
          {
            if ( ! backup.forEach( a.begin(), a.end(), &StatusBackup::unlock, causer_r ) ) return backup.restore();
            return true;
          }
          break;
      }
      return false;
    }

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
