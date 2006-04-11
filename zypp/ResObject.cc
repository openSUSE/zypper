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

  ByteCount ResObject::size() const
  { return pimpl().size(); }

  Source_Ref ResObject::source() const
  { return pimpl().source(); }

  ZmdId ResObject::zmdid () const
  { return pimpl().zmdid(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
