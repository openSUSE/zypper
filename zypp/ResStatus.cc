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
  const ResStatus ResStatus::toBeUninstalledDueToUnlink	 (INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, DUE_TO_UNLINK);
  const ResStatus ResStatus::toBeUninstalledDueToObsolete(INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, DUE_TO_OBSOLETE);
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


  ResStatus::ResStatus (enum StateValue s, enum EstablishValue e, enum TransactValue t, enum InstallDetailValue i, enum RemoveDetailValue r)
    : _bitfield (s)
  {
    fieldValueAssign<EstablishField>(e);
    fieldValueAssign<TransactField>(t);
    if (t == TRANSACT) {
	if (s == INSTALLED) fieldValueAssign<TransactDetailField>(r);
	else fieldValueAssign<TransactDetailField>(i);
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

    str << (obj.transacts () ? "T" : "_");

    if (obj.transacts ()) {
	if (obj.isBySolver()) str << "s";
	else if (obj.isByApplLow()) str << "l";
	else if (obj.isByApplHigh()) str << "h";
	else if (obj.isByUser()) str << "u";
    }

    str << (obj.isToBeUninstalledDueToObsolete() ? "O" :
	( obj.isToBeUninstalledDueToUnlink() ? "U" :
	( obj.isToBeInstalledSoft() ? "S" : "_" ) ) );

    if (obj == ResStatus::toBeInstalled)		str << "<tobeinstalled>";
    if (obj == ResStatus::toBeInstalledSoft)		str << "<tobesoftinstalled>";
    if (obj == ResStatus::toBeUninstalled)		str << "<tobeuninstalled>";
    if (obj == ResStatus::toBeUninstalledDueToUnlink)	str << "<tobeuninstalledduetounlink>";
    if (obj == ResStatus::toBeUninstalledDueToObsolete)	str << "<tobeuninstalledduetoobsolete>";
    if (obj == ResStatus::satisfied)			str << "<satisfied>";			// uninstalled, satisfied
    if (obj == ResStatus::complete)			str << "<complete>";			// installed, satisfied
    if (obj == ResStatus::unneeded)			str << "<uneeded>";			// uninstalled, unneeded
    if (obj == ResStatus::needed)			str << "<needed>";			// uninstalled, incomplete
    if (obj == ResStatus::incomplete)			str << "<incomplete>";			// installed, incomplete

    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
