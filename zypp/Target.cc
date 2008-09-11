/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Target.cc
 *
*/
#include <cassert>

#include <iostream>

#include "zypp/Target.h"
#include "zypp/target/TargetImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Target);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Target::Target
  //	METHOD TYPE : Ctor
  //
  Target::Target( const Pathname & root, bool doRebuild_r )
  : _pimpl( new Impl(root,doRebuild_r) )
  {
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Target::Target
  //	METHOD TYPE : Ctor
  //
  Target::Target( const Impl_Ptr & impl_r )
  : _pimpl( impl_r )
  {
    assert( impl_r );
  }

  Target_Ptr Target::_nullimpl;

  /** Null implementation */
  Target_Ptr Target::nullimpl()
  {
    if (! _nullimpl)
    {
      _nullimpl = new Target(target::TargetImpl::nullimpl());
    }
    return _nullimpl;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //	Forward to TargetImpl:
  //
  ///////////////////////////////////////////////////////////////////

  void Target::buildCache()
  { _pimpl->buildCache(); }

  void Target::cleanCache()
  { _pimpl->clearCache(); }

  void Target::load()
  { _pimpl->load(); }

  void Target::unload()
  { _pimpl->unload(); }

  target::rpm::RpmDb & Target::rpmDb()
  { return _pimpl->rpm(); }

  Pathname Target::root() const
  { return _pimpl->root(); }

  bool Target::providesFile (const std::string & name_str, const std::string & path_str) const
  { return _pimpl->providesFile (name_str, path_str); }

  std::string Target::whoOwnsFile (const std::string & path_str) const
  { return _pimpl->whoOwnsFile (path_str); }

  std::ostream & Target::dumpOn( std::ostream & str ) const
  { return _pimpl->dumpOn( str ); }

  bool Target::setInstallationLogfile(const Pathname & path_r)
  { return _pimpl->setInstallationLogfile(path_r); }

  Date Target::timestamp() const
  { return _pimpl->timestamp(); }

  std::string Target::release() const
  { return _pimpl->release(); }

  std::string Target::targetDistribution() const
  { return _pimpl->targetDistribution(); }

  std::string Target::targetDistributionRelease() const
  { return _pimpl->targetDistributionRelease(); }

  std::string Target::targetDistributionFlavor() const
  { return _pimpl->targetDistributionFlavor(); }

  std::string Target::anonymousUniqueId() const
  { return _pimpl->anonymousUniqueId(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
