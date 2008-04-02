/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResStatus.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/ResStatus.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  const ResStatus ResStatus::toBeInstalled		 (UNINSTALLED, TRANSACT);
  const ResStatus ResStatus::toBeInstalledSoft		 (UNINSTALLED, TRANSACT, SOFT_INSTALL);
  const ResStatus ResStatus::toBeUninstalled		 (INSTALLED,   TRANSACT);
  const ResStatus ResStatus::toBeUninstalledSoft	 (INSTALLED,   TRANSACT, EXPLICIT_INSTALL, SOFT_REMOVE);
  const ResStatus ResStatus::toBeUninstalledDueToObsolete(INSTALLED,   TRANSACT, EXPLICIT_INSTALL, DUE_TO_OBSOLETE);
  const ResStatus ResStatus::toBeUninstalledDueToUpgrade (INSTALLED,   TRANSACT, EXPLICIT_INSTALL, DUE_TO_UPGRADE);
  const ResStatus ResStatus::installed			 (INSTALLED);
  const ResStatus ResStatus::uninstalled		 (UNINSTALLED);
  const ResStatus ResStatus::recommended		 (RECOMMENDED);
  const ResStatus ResStatus::suggested			 (SUGGESTED);        

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResStatus::ResStatus
  //	METHOD TYPE : Ctor
  //
  ResStatus::ResStatus()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResStatus::ResStatus
  //	METHOD TYPE : Ctor
  //
  ResStatus::ResStatus( bool isInstalled_r )
  : _bitfield( isInstalled_r ? INSTALLED : UNINSTALLED )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResStatus::~ResStatus
  //	METHOD TYPE : Dtor
  //
  ResStatus::~ResStatus()
  {}


  ResStatus::ResStatus (enum StateValue s, enum TransactValue t, enum InstallDetailValue i, enum RemoveDetailValue r, enum SolverStateValue ssv)
    : _bitfield (s)
  {
    fieldValueAssign<TransactField>(t);
    if (t == TRANSACT) {
	if (s == INSTALLED) fieldValueAssign<TransactDetailField>(r);
	else fieldValueAssign<TransactDetailField>(i);
    }
    if (ssv != NORMAL) {
	fieldValueAssign<SolverStateField>(ssv);
    }
  }


  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ResStatus & obj )
  {
    str << (obj.isInstalled() ? "I" : "U");

    str << (obj.transacts () ? "T"
                             : (obj.isLocked() ? "L" : "_") );

    if (obj.isBySolver()) str << "s";
    else if (obj.isByApplLow()) str << "l";
    else if (obj.isByApplHigh()) str << "h";
    else if (obj.isByUser()) str << "u";

    str << (obj.isToBeUninstalledDueToObsolete() ? "O" :
	( obj.isToBeUninstalledDueToUpgrade() ? "U" :
	( obj.isToBeInstalledSoft() ? "S" : "_" ) ) );

    str << (obj.isSeen() ? "@" : "" );

    str << (obj.isRecommended() ? "r" : "" );

    str << (obj.isSuggested() ? "s" : "" );    

    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
