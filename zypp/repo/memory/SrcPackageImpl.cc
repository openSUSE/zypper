/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/memory/SrcPackageImpl.cc
 *
*/
#include "zypp/repo/memory/SrcPackageImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
namespace memory
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : SrcPackageImpl::SrcPackageImpl
//	METHOD TYPE : Ctor
//
SrcPackageImpl::SrcPackageImpl(data::SrcPackage_Ptr ptr)
    : _media_number( 1 )
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : SrcPackageImpl::~SrcPackageImpl
//	METHOD TYPE : Dtor
//
SrcPackageImpl::~SrcPackageImpl()
{}


Pathname SrcPackageImpl::location() const
{
  return _location;
}

ByteCount SrcPackageImpl::archivesize() const
{
  return _archivesize;
}

DiskUsage SrcPackageImpl::diskusage() const
{
  return _diskusage;
}

Source_Ref SrcPackageImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned SrcPackageImpl::sourceMediaNr() const
{
  return _media_number;
}

/////////////////////////////////////////////////////////////////
} // namespace memory
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
