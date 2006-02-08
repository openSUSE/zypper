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

    /**
     * \todo Support non USER level mipulation.
    */
    struct StatusHelper
    {
      StatusHelper( const Selectable::Impl & impl )
      : _impl( impl )
      , inst( impl.installedObj() )
      , cand( impl.candidateObj() )
      {}

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
      /** \todo fix it, handle avaialable list */
      bool setInstall( ResStatus::TransactByValue by_r ) const
      {
        if ( cand )
          {
            if ( inst )
              inst.status().setTransact( false, by_r );
            return cand.status().setTransact( true, by_r );
          }
        return false;
      }

      bool setDelete( ResStatus::TransactByValue by_r ) const
      {
        if ( inst )
          {
            if ( cand )
              cand.status().setTransact( false, by_r );
            return inst.status().setTransact( true, by_r );
          }
        return false;
      }

      bool unset( ResStatus::TransactByValue by_r ) const
      {
        if ( inst )
          inst.status().setTransact( false, by_r );
        if ( cand )
          cand.status().setTransact( false, by_r );
        return true;
      }

    public:
      const Selectable::Impl & _impl;
      PoolItem inst;
      PoolItem cand;
    };

    /**
     * \todo Still open questions in state calculation;
     * neglecting TABOO/PROTECTED
    */
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

      return( installedObj() ? S_KeepInstalled : S_NoInst );
    }


    /**
     * \todo Neglecting TABOO/PROTECTED
    */
    bool Selectable::Impl::set_status( const Status state_r )
    {
      StatusHelper self( *this );

      switch ( state_r )
        {
        case S_Protected:
        case S_Taboo:
          return false;

        case S_AutoDel:
        case S_AutoInstall:
        case S_AutoUpdate:
          // Auto level is SOLVER level. UI may query, but not
          // set at this level.
          break;

        case S_Del:
          return self.setDelete( ResStatus::USER );
          break;

        case S_Install:
          return self.hasCandidateOnly() && self.setInstall( ResStatus::USER );
          break;

        case S_Update:
          return self.hasBoth() && self.setInstall( ResStatus::USER );
          break;

        case S_KeepInstalled:
          return self.hasInstalled() && self.unset( ResStatus::USER );
          break;

        case S_NoInst:
          return !self.hasInstalled() && self.unset( ResStatus::USER );
          break;
        }

      return false;
    }


    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
