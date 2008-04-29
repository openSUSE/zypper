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

  const ResStatus ResStatus::toBeInstalled		 (UNINSTALLED, UNDETERMINED, TRANSACT);
  const ResStatus ResStatus::toBeInstalledSoft		 (UNINSTALLED, UNDETERMINED, TRANSACT, SOFT_INSTALL);
  const ResStatus ResStatus::toBeUninstalled		 (INSTALLED,   UNDETERMINED, TRANSACT);
  const ResStatus ResStatus::toBeUninstalledSoft	 (INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, SOFT_REMOVE);
  const ResStatus ResStatus::toBeUninstalledDueToObsolete(INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, DUE_TO_OBSOLETE);
  const ResStatus ResStatus::toBeUninstalledDueToUpgrade (INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, DUE_TO_UPGRADE);
  const ResStatus ResStatus::installed			 (INSTALLED);
  const ResStatus ResStatus::uninstalled		 (UNINSTALLED);

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


  ResStatus::ResStatus (enum StateValue s, enum ValidateValue v, enum TransactValue t, enum InstallDetailValue i, enum RemoveDetailValue r, enum SolverStateValue ssv)
    : _bitfield (s)
  {
    fieldValueAssign<ValidateField>(v);
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

    str << (obj.isBroken() ? "B" :
	( obj.isSatisfied() ? "S" :
	( obj.isNonRelevant() ? "N" : "_") ) );

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
#ifdef HARDLOCKTEST
    str << (obj.isUserLockQueryMatch() ?  "L" : "_" );
#endif
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
