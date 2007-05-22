/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/repository/memory/DSrcPackageImpl.cc
 *
*/
#include "zypp2/repository/memory/DSrcPackageImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repository
{ /////////////////////////////////////////////////////////////////
namespace memory
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : SrcPackageImpl::SrcPackageImpl
//	METHOD TYPE : Ctor
//
DSrcPackageImpl::DSrcPackageImpl(data::SrcPackage_Ptr ptr)
    : _media_number( 1 )
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : SrcPackageImpl::~SrcPackageImpl
//	METHOD TYPE : Dtor
//
DSrcPackageImpl::~DSrcPackageImpl()
{}


Pathname DSrcPackageImpl::location() const
{
  return _location;
}

ByteCount DSrcPackageImpl::archivesize() const
{
  return _archivesize;
}

DiskUsage DSrcPackageImpl::diskusage() const
{
  return _diskusage;
}

Source_Ref DSrcPackageImpl::source() const
{
  return Source_Ref::noSource;
}

unsigned DSrcPackageImpl::sourceMediaNr() const
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
