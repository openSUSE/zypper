/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/PackageDelta.cc
 *
*/
#include <iostream>

#include "zypp/base/LogTools.h"

#include "zypp/source/PackageDelta.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace packagedelta
  { /////////////////////////////////////////////////////////////////


    std::ostream & operator<<( std::ostream & str, const BaseVersion & obj )
    {
      return str
      << "BaseVersion(" << obj.edition()
      << '|' << obj.checksum()
      << '|' << obj.buildtime()
      << ')';
    }

    std::ostream & operator<<( std::ostream & str, const PatchRpm & obj )
    {
      str
      << "PatchRpm(" << obj.arch()
      << '|' << obj.filename()
      << '|';
      return dumpRange( str, obj.baseVersions().begin(), obj.baseVersions().end() )
      << ')';
    }

    std::ostream & operator<<( std::ostream & str, const DeltaRpm & obj )
    {
      return str
      << "DeltaRpm(" << obj.arch()
      << '|' << obj.filename()
      << '|' << obj.baseVersion()
      << ')';
    }

    /////////////////////////////////////////////////////////////////
  } // namespace packagedelta
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
