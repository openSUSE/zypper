/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYpp.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/ZYpp.h"
#include "zypp/zypp_detail/ZYppImpl.h"
#include "zypp/sat/Pool.h"
#include "zypp/ManagedFile.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ZYpp::ZYpp( const Impl_Ptr & impl_r )
  : _pimpl( impl_r )
  {}

  ZYpp::~ZYpp()
  {}

  std::ostream & operator<<( std::ostream & str, const ZYpp & obj )
  { return str << *obj._pimpl; }

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to Impl:
  //
  ///////////////////////////////////////////////////////////////////

  ResPool ZYpp::pool() const
  { return _pimpl->pool(); }

  DiskUsageCounter::MountPointSet ZYpp::diskUsage()
  { return _pimpl->diskUsage(); }

  void ZYpp::setPartitions(const DiskUsageCounter::MountPointSet &mp)
  { return _pimpl->setPartitions(mp); }

  DiskUsageCounter::MountPointSet ZYpp::getPartitions() const
  { return _pimpl->getPartitions(); }

  ResPoolProxy ZYpp::poolProxy() const
  { return _pimpl->poolProxy(); }

  Resolver_Ptr ZYpp::resolver() const
  { return _pimpl->resolver(); }

  KeyRing_Ptr ZYpp::keyRing() const
  { return _pimpl->keyRing(); }

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to Impl:
  //
  ///////////////////////////////////////////////////////////////////

  Target_Ptr ZYpp::target() const
  { return _pimpl->target(); }

  Target_Ptr ZYpp::getTarget() const
  { return _pimpl->getTarget(); }

  void ZYpp::initializeTarget( const Pathname & root, bool doRebuild_r )
  { _pimpl->initializeTarget( root, doRebuild_r ); }

  void ZYpp::finishTarget()
  { _pimpl->finishTarget(); }

  ZYppCommitResult ZYpp::commit( const ZYppCommitPolicy & policy_r )
  { return _pimpl->commit( policy_r ); }

  void ZYpp::installSrcPackage( const SrcPackage_constPtr & srcPackage_r )
  { _pimpl->installSrcPackage( srcPackage_r ); }

  ManagedFile ZYpp::provideSrcPackage( const SrcPackage_constPtr & srcPackage_r )
  {return _pimpl->provideSrcPackage( srcPackage_r ); }
  ///////////////////////////////////////////////////////////////////

  Pathname ZYpp::homePath() const
  { return _pimpl->homePath(); }

  Pathname ZYpp::tmpPath() const
  { return _pimpl->tmpPath(); }

  void ZYpp::setHomePath( const Pathname & path )
  { _pimpl->setHomePath(path); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
