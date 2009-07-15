/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Resolver.cc
 *
*/
#include <iostream>

#include "zypp/Resolver.h"
#include "zypp/ZConfig.h"
#include "zypp/TriBool.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/Testcase.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  using namespace solver;

  IMPL_PTR_TYPE(Resolver);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolver::Resolver
  //	METHOD TYPE : Ctor
  //
  Resolver::Resolver( const ResPool & pool )
  {
    _pimpl = new solver::detail::Resolver(pool);
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolver::~Resolver
  //	METHOD TYPE : Dtor
  //
  Resolver::~Resolver()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Resolver interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////
  bool Resolver::verifySystem ()
  { return _pimpl->verifySystem(); }

  bool Resolver::resolvePool ()
  { return _pimpl->resolvePool(); }

  bool Resolver::resolveQueue( solver::detail::SolverQueueItemList & queue )
  { return _pimpl->resolveQueue(queue); }

  void Resolver::undo()
  { _pimpl->undo(); }

  ResolverProblemList Resolver::problems ()
  { return _pimpl->problems (); }

  void Resolver::applySolutions( const ProblemSolutionList & solutions )
  { _pimpl->applySolutions (solutions); }

  bool Resolver::doUpgrade()
  { return _pimpl->doUpgrade(); }

  void Resolver::doUpdate()
  { _pimpl->doUpdate(); }

  void Resolver::setForceResolve( bool yesno_r )	{ _pimpl->setForceResolve( yesno_r ); }
  bool Resolver::forceResolve()				{ return _pimpl->forceResolve(); }

  void Resolver::setIgnoreAlreadyRecommended( bool yesno_r) { _pimpl->setIgnoreAlreadyRecommended( yesno_r ); }
  bool Resolver::ignoreAlreadyRecommended()		{ return _pimpl->ignoreAlreadyRecommended(); }

  void Resolver::setOnlyRequires( bool yesno_r )	{ _pimpl->setOnlyRequires( yesno_r ); }
  void Resolver::resetOnlyRequires()			{ _pimpl->setOnlyRequires( indeterminate ); }
  bool Resolver::onlyRequires()				{ return _pimpl->onlyRequires(); }

  bool Resolver::upgradeMode() const			{ return _pimpl->isUpgradeMode(); }

  void Resolver::setAllowVendorChange( bool yesno_r )	{ _pimpl->setAllowVendorChange( yesno_r ); }
  void Resolver::setDefaultAllowVendorChange()		{ _pimpl->setAllowVendorChange( indeterminate ); }
  bool Resolver::allowVendorChange() const		{ return _pimpl->allowVendorChange(); }

  void Resolver::setSystemVerification( bool yesno_r )	{ _pimpl->setVerifyingMode( yesno_r ); }
  void Resolver::setDefaultSystemVerification()		{ _pimpl->setVerifyingMode( indeterminate ); }
  bool Resolver::systemVerification() const		{ return _pimpl->isVerifyingMode(); }

  void Resolver::setSolveSrcPackages( bool yesno_r )	{ _pimpl->setSolveSrcPackages( yesno_r ); }
  void Resolver::setDefaultSolveSrcPackages()		{ _pimpl->setSolveSrcPackages( indeterminate ); }
  bool Resolver::solveSrcPackages() const		{ return _pimpl->solveSrcPackages(); }


  void Resolver::addRequire( const Capability & capability )	{ _pimpl->addExtraRequire( capability ); }
  void Resolver::addConflict( const Capability & capability )	{ _pimpl->addExtraConflict( capability ); }
  void Resolver::removeRequire( const Capability & capability )	{ _pimpl->removeExtraRequire( capability ); }
  void Resolver::removeConflict( const Capability & capability ){ _pimpl->removeExtraConflict( capability ); }

  CapabilitySet Resolver::getRequire()	{ return _pimpl->extraRequires(); }
  CapabilitySet Resolver::getConflict()	{ return _pimpl->extraConflicts(); }

  std::list<PoolItem> Resolver::problematicUpdateItems() const
  { return _pimpl->problematicUpdateItems(); }

  bool Resolver::createSolverTestcase( const std::string & dumpPath, bool runSolver )
  {
    solver::detail::Testcase testcase (dumpPath);
    return testcase.createTestcase(*_pimpl, true, runSolver);
  }

  solver::detail::ItemCapKindList Resolver::isInstalledBy( const PoolItem & item )
  { return _pimpl->isInstalledBy (item); }

  solver::detail::ItemCapKindList Resolver::installs( const PoolItem & item )
  { return _pimpl->installs (item); }

  solver::detail::ItemCapKindList Resolver::satifiedByInstalled( const PoolItem & item )
  { return _pimpl->satifiedByInstalled (item); }

  solver::detail::ItemCapKindList Resolver::installedSatisfied( const PoolItem & item )
  { return _pimpl->installedSatisfied (item); }

  void Resolver::reset()
  { _pimpl->reset( false ); /* Do not keep extra requires/conflicts */ }



  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
