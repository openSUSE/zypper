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
#include "zypp/Repository.h"
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
    str << "[S" << repository().numericId() << ":" << mediaNr() << "]";
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

  Repository ResObject::repository() const
  { return pimpl().repository(); }

  ByteCount ResObject::downloadSize() const
  { return pimpl().downloadSize(); }
  
  unsigned ResObject::mediaNr() const
  { return pimpl().mediaNr(); }

  bool ResObject::installOnly() const
  { return pimpl().installOnly(); }

  Date ResObject::buildtime() const
  { return pimpl().buildtime(); }

  Date ResObject::installtime() const
  { return pimpl().installtime(); }
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
