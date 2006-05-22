/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResObject.cc
 *
*/
#include "zypp/ResObject.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/detail/ResObjectImplIf.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResObject::ResObject
  //	METHOD TYPE : Ctor
  //
  ResObject::ResObject( const Kind & kind_r,
                        const NVRAD & nvrad_r )
  : Resolvable( kind_r, nvrad_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResObject::~ResObject
  //	METHOD TYPE : Dtor
  //
  ResObject::~ResObject()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResObject::dumpOn
  //	METHOD TYPE : std::ostream &
  //
  std::ostream & ResObject::dumpOn( std::ostream & str ) const
  {
    str << "[S" << source().numericId() << ":" << sourceMediaNr() << "]";
    return Resolvable::dumpOn( str );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	ResObject interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  Text ResObject::summary() const
  { return pimpl().summary().text(); }

  Text ResObject::description() const
  { return pimpl().description().text(); }

  Text ResObject::insnotify() const
  { return pimpl().insnotify().text(); }

  Text ResObject::delnotify() const
  { return pimpl().delnotify().text(); }

  License ResObject::licenseToConfirm() const
  { return pimpl().licenseToConfirm().text(); }

  Vendor ResObject::vendor() const
  { return pimpl().vendor(); }

  ByteCount ResObject::size() const
  { return pimpl().size(); }

  ByteCount ResObject::archivesize() const
  { return pimpl().archivesize(); }

  Source_Ref ResObject::source() const
  { return pimpl().source(); }

  unsigned ResObject::sourceMediaNr() const
  { return pimpl().sourceMediaNr(); }

  bool ResObject::installOnly() const
  { return pimpl().installOnly(); }

  Date ResObject::buildtime() const
  { return pimpl().buildtime(); }

  Date ResObject::installtime() const
  { return pimpl().installtime(); }

  ZmdId ResObject::zmdid () const
  { return pimpl().zmdid(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
