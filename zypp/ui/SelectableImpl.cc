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

    struct StatusHelper
    {
      StatusHelper( const Selectable::Impl & impl )
      : _impl( impl )
      , inst( impl.installedObj() )
      , cand( impl.candidateObj() )
      {}

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


      bool userSetInstall() const
      { return false; }

      bool userSetDelete() const
      { return false; }

      bool userUnset() const
      { return false; }

      bool autoSetInstall() const
      { return false; }

      bool autoSetDelete() const
      { return false; }

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

        case S_Del:
          return self.userSetDelete();
          break;

        case S_Install:
          return self.hasCandidateOnly() && self.userSetInstall();
          break;

        case S_Update:
          return self.hasBoth() && self.userSetInstall();
          break;

        case S_AutoDel:
          return self.autoSetDelete();
          break;

        case S_AutoInstall:
          return self.hasCandidateOnly() && self.autoSetInstall();
          break;

        case S_AutoUpdate:
          return self.hasBoth() && self.autoSetInstall();
          break;

        case S_KeepInstalled:
          return self.hasInstalled() && self.userUnset();
          break;

        case S_NoInst:
          return !self.hasInstalled() && self.userUnset();
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
