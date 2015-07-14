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

#define ZYPP_USE_RESOLVER_INTERNALS

#include "zypp/Resolver.h"
#include "zypp/ZConfig.h"
#include "zypp/TriBool.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/Testcase.h"
#include "zypp/solver/detail/ItemCapKind.h"
#include "zypp/sat/Transaction.h"

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
  : _pimpl( new solver::detail::ResolverInternal(pool) )
  {}

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

  sat::Transaction Resolver::getTransaction()
  { return _pimpl->getTransaction(); }

  bool Resolver::doUpgrade()
  { return _pimpl->doUpgrade(); }

  void Resolver::doUpdate()
  { _pimpl->doUpdate(); }

  void Resolver::setForceResolve( bool yesno_r )	{ _pimpl->setForceResolve( yesno_r ); }
  bool Resolver::forceResolve() const			{ return _pimpl->forceResolve(); }

  void Resolver::setIgnoreAlreadyRecommended( bool yesno_r) { _pimpl->setIgnoreAlreadyRecommended( yesno_r ); }
  bool Resolver::ignoreAlreadyRecommended() const	{ return _pimpl->ignoreAlreadyRecommended(); }

  void Resolver::setOnlyRequires( bool yesno_r )	{ _pimpl->setOnlyRequires( yesno_r ); }
  void Resolver::resetOnlyRequires()			{ _pimpl->setOnlyRequires( indeterminate ); }
  bool Resolver::onlyRequires() const			{ return _pimpl->onlyRequires(); }

  void Resolver::setUpgradeMode( bool yesno_r )		{ return _pimpl->setUpgradeMode( yesno_r ); }
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

  void Resolver::setCleandepsOnRemove( bool yesno_r )	{ _pimpl->setCleandepsOnRemove( yesno_r ); }
  void Resolver::setDefaultCleandepsOnRemove()		{ _pimpl->setCleandepsOnRemove( indeterminate ); }
  bool Resolver::cleandepsOnRemove() const		{ return _pimpl->cleandepsOnRemove(); }

#define ZOLV_FLAG_BOOL( ZSETTER, ZGETTER )					\
  void Resolver::ZSETTER( bool yesno_r ){ _pimpl->ZSETTER( yesno_r ); }		\
  bool Resolver::ZGETTER() const	{ return _pimpl->ZGETTER(); }		\

#define ZOLV_FLAG_TRIBOOL( ZSETTER, ZDEFAULT, ZGETTER )				\
  ZOLV_FLAG_BOOL( ZSETTER , ZGETTER )						\
  void Resolver::ZDEFAULT()		{ _pimpl->ZSETTER( indeterminate ); }	\

  ZOLV_FLAG_TRIBOOL( dupSetAllowDowngrade,	dupSetDefaultAllowDowngrade,	dupAllowDowngrade )
  ZOLV_FLAG_TRIBOOL( dupSetAllowNameChange,	dupSetDefaultAllowNameChange,	dupAllowNameChange )
  ZOLV_FLAG_TRIBOOL( dupSetAllowArchChange,	dupSetDefaultAllowArchChange,	dupAllowArchChange )
  ZOLV_FLAG_TRIBOOL( dupSetAllowVendorChange,	dupSetDefaultAllowVendorChange,	dupAllowVendorChange )

#undef ZOLV_FLAG_BOOL
#undef ZOLV_FLAG_TRIBOOL

  void Resolver::addUpgradeRepo( Repository repo_r )	{ _pimpl->addUpgradeRepo( repo_r ); }
  bool Resolver::upgradingRepo( Repository repo_r ) const { return _pimpl->upgradingRepo( repo_r ); }
  void Resolver::removeUpgradeRepo( Repository repo_r )	{ _pimpl->removeUpgradeRepo( repo_r ); }
  void Resolver::removeUpgradeRepos()			{ _pimpl->removeUpgradeRepos(); }

  void Resolver::addRequire( const Capability & capability )	{ _pimpl->addExtraRequire( capability ); }
  void Resolver::addConflict( const Capability & capability )	{ _pimpl->addExtraConflict( capability ); }
  void Resolver::removeRequire( const Capability & capability )	{ _pimpl->removeExtraRequire( capability ); }
  void Resolver::removeConflict( const Capability & capability ){ _pimpl->removeExtraConflict( capability ); }

  CapabilitySet Resolver::getRequire() const	{ return _pimpl->extraRequires(); }
  CapabilitySet Resolver::getConflict() const	{ return _pimpl->extraConflicts(); }

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

  std::ostream & operator<<( std::ostream & str, const Resolver & obj )
  { return str << *obj._pimpl; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
