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


ResStatus ResStatus::toBeInstalled		 = ResStatus (UNINSTALLED, UNDETERMINED, TRANSACT);
ResStatus ResStatus::toBeInstalledSoft		 = ResStatus (UNINSTALLED, UNDETERMINED, TRANSACT, SOFT_REQUIRES);
ResStatus ResStatus::toBeUninstalled		 = ResStatus (INSTALLED,   UNDETERMINED, TRANSACT);
ResStatus ResStatus::toBeUninstalledDueToUnlink	 = ResStatus (INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, DUE_TO_UNLINK);
ResStatus ResStatus::toBeUninstalledDueToObsolete= ResStatus (INSTALLED,   UNDETERMINED, TRANSACT, EXPLICIT_INSTALL, DUE_TO_OBSOLETE);
ResStatus ResStatus::incomplete			 = ResStatus (INSTALLED,   INCOMPLETE);
ResStatus ResStatus::complete			 = ResStatus (INSTALLED,   SATISFIED);
ResStatus ResStatus::satisfied			 = ResStatus (UNINSTALLED, SATISFIED);
ResStatus ResStatus::unneeded			 = ResStatus (UNINSTALLED, UNNEEDED);
ResStatus ResStatus::needed			 = ResStatus (UNINSTALLED, INCOMPLETE);

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
