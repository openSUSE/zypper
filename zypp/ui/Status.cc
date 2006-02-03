/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/Status.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"

#include "zypp/ui/Status.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    /******************************************************************
    **
    **	FUNCTION NAME : asString
    **	FUNCTION TYPE : std::string
    */
    std::string asString( const Status & obj )
    {
      switch ( obj ) {
#define ENUM_OUT(V) case V: return #V; break

        ENUM_OUT( S_Protected );
        ENUM_OUT( S_Taboo );
        ENUM_OUT( S_Del );
        ENUM_OUT( S_Install );
        ENUM_OUT( S_Update );
        ENUM_OUT( S_AutoDel );
        ENUM_OUT( S_AutoInstall );
        ENUM_OUT( S_AutoUpdate );
        ENUM_OUT( S_NoInst );
        ENUM_OUT( S_KeepInstalled );

#undef ENUM_OUT
      }

      INT << "Unknown ui::Status " << (unsigned)obj << std::endl;
      return "Status(UNKNOWN)";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
