/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/TargetException.cc
 *
*/

#include <string>
#include <iostream>

#include <zypp/target/TargetException.h>
#include <zypp/base/Gettext.h>


///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace target {
  /////////////////////////////////////////////////////////////////

    TargetAbortedException::TargetAbortedException()
      : TargetAbortedException ( _("Installation has been aborted as directed.") )
    { }

    std::ostream & TargetAbortedException::dumpOn( std::ostream & str ) const
    {
        //call base implementation, do not hardcode a string, do not blame user ( fixes bnc#978193 )
        return TargetException::dumpOn( str );
    }


  /////////////////////////////////////////////////////////////////
  } // namespace target
} // namespace zypp
///////////////////////////////////////////////////////////////////
