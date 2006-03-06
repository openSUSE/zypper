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
  const ResStatus ResStatus::impossible			 (UNINSTALLED, UNDETERMINED, KEEP_STATE, EXPLICIT_INSTALL, EXPLICIT_REMOVE, IMPOSSIBLE);
  const ResStatus ResStatus::toBeUninstalledSoft	 (INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, SOFT_REMOVE);
  const ResStatus ResStatus::toBeUninstalledDueToUnlink	 (INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, DUE_TO_UNLINK);
  const ResStatus ResStatus::toBeUninstalledDueToObsolete(INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, DUE_TO_OBSOLETE);
  const ResStatus ResStatus::toBeUninstalledDueToUpgrade (INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, DUE_TO_UPGRADE);
  const ResStatus ResStatus::installed			 (INSTALLED,   UNDETERMINED);
  const ResStatus ResStatus::uninstalled		 (UNINSTALLED, UNDETERMINED);
  const ResStatus ResStatus::incomplete			 (INSTALLED,   INCOMPLETE);
  const ResStatus ResStatus::complete			 (INSTALLED,   SATISFIED);
  const ResStatus ResStatus::satisfied			 (UNINSTALLED, SATISFIED);
  const ResStatus ResStatus::unneeded			 (UNINSTALLED, UNNEEDED);
  const ResStatus ResStatus::needed			 (UNINSTALLED, INCOMPLETE, TRANSACT);

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


  ResStatus::ResStatus (enum StateValue s, enum EstablishValue e, enum TransactValue t, enum InstallDetailValue i, enum RemoveDetailValue r, enum SolverStateValue ssv)
    : _bitfield (s)
  {
    fieldValueAssign<EstablishField>(e);
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

    str << (obj.isUnneeded() ? "U" :
	( obj.isSatisfied() ? "S" :
	( obj.isIncomplete() ? "I" : "_") ) );

    str << (obj.transacts () ? "T"
                             : (obj.isLocked() ? "L" : "_") );

    if (obj.isBySolver()) str << "s";
    else if (obj.isByApplLow()) str << "l";
    else if (obj.isByApplHigh()) str << "h";
    else if (obj.isByUser()) str << "u";

    str << (obj.isToBeUninstalledDueToObsolete() ? "O" :
	( obj.isToBeUninstalledDueToUnlink() ? "L" :
	( obj.isToBeUninstalledDueToUpgrade() ? "U" :
	( obj.isToBeInstalledSoft() ? "S" : "_" ) ) ) );

    str << (obj.isSeen() ? "@" :
	( obj.isImpossible() ? "X" : "" ) );

    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
