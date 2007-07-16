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

#include "zypp/repo/PackageDelta.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace packagedelta
  { /////////////////////////////////////////////////////////////////


    std::ostream & operator<<( std::ostream & str, const PatchRpm & obj )
    {
      str
      << "PatchRpm(" << obj.location()
      << '|' << obj.buildtime()
      << '|';
      return dumpRangeLine( str, obj.baseversions().begin(), obj.baseversions().end() )
      << ')';
    }

    std::ostream & operator<<( std::ostream & str, const DeltaRpm & obj )
    {
      return str
      << "DeltaRpm(" << obj.location()
      << '|' << obj.buildtime()
      << '|' << obj.baseversion().edition()
      << ',' << obj.baseversion().buildtime()
      << ',' << obj.baseversion().checksum()
      << ',' << obj.baseversion().sequenceinfo()
      << ')';
    }

    /////////////////////////////////////////////////////////////////
  } // namespace packagedelta
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
