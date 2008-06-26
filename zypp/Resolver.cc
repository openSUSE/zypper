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
#include "zypp/UpgradeStatistics.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/Testcase.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  using namespace solver;
    
  IMPL_PTR_TYPE(Resolver);
#if 0
  Resolver_Ptr Resolver::_resolver = NULL;
  Resolver_Ptr Resolver::resolver()
  {
    if (_resolver == NULL) {
	_resolver = new Resolver();
    }
    return _resolver;
  }
#endif
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
  bool Resolver::resolveQueue (zypp::solver::detail::SolverQueueItemList & queue)
  { return _pimpl->resolveQueue(queue); }    
  void Resolver::undo()
  { _pimpl->undo(); }
  ResolverProblemList Resolver::problems ()
  { return _pimpl->problems (); }
  void Resolver::applySolutions( const ProblemSolutionList & solutions )
  { _pimpl->applySolutions (solutions); }      
  bool Resolver::doUpgrade( UpgradeStatistics & opt_stats_r )
  { return _pimpl->doUpgrade(opt_stats_r); }
  void Resolver::doUpdate()
  { _pimpl->doUpdate(); }    
  void Resolver::setForceResolve( const bool force )
  { _pimpl->setForceResolve( force ); }
  bool Resolver::forceResolve()
  { return _pimpl->forceResolve(); }
  void Resolver::setIgnoreAlreadyRecommended
       (const bool ignoreAlreadyRecommended)
  { _pimpl->setIgnorealreadyrecommended (ignoreAlreadyRecommended); }
  bool Resolver::ignoreAlreadyRecommended()
  { return _pimpl->ignorealreadyrecommended(); }    
  void Resolver::setOnlyRequires( const bool onlyRequires )
  { onlyRequires ? _pimpl->setOnlyRequires( true ) : _pimpl->setOnlyRequires( false ); }
  void Resolver::resetOnlyRequires()
  { _pimpl->setOnlyRequires( indeterminate ); }    
  bool Resolver::onlyRequires()
  {
      if (_pimpl->onlyRequires() == indeterminate)
	  return ZConfig::instance().solver_onlyRequires();
      else
	  return _pimpl->onlyRequires();
  }

  void Resolver::addNoObsoletesCapability (const Capability & capability)
  { _pimpl->addNoObsoletesCapability (capability); }
  void Resolver::removeNoObsoletesCapability (const Capability & capability)
  { _pimpl->removeNoObsoletesCapability (capability); }
  CapabilitySet Resolver::noObsoletesCapability()
  { return _pimpl->noObsoletesCapability(); }	
  void Resolver::addNoObsoletesItem (const PoolItem & item)
  { _pimpl->addNoObsoletesItem (item); }
  void Resolver::removeNoObsoletesItem (const PoolItem & item)
  { _pimpl->removeNoObsoletesItem (item); }
  solver::detail::PoolItemSet Resolver::noObsoletesItem()
  { return _pimpl->noObsoletesItem(); }	
  void Resolver::addNoObsoletesName (const std::string & name)
  { _pimpl->addNoObsoletesName (name); }
  void Resolver::removeNoObsoletesName (const std::string & name)
  { _pimpl->removeNoObsoletesName (name); }	
  solver::detail::ObsoleteStrings Resolver::noObsoletesString ()
  { return _pimpl->noObsoletesString (); }	
  
  void Resolver::addRequire (const Capability & capability)
  { _pimpl->addExtraRequire( capability ); }
  void Resolver::addConflict (const Capability & capability)
  { _pimpl->addExtraConflict( capability ); }
  void Resolver::removeRequire (const Capability & capability)
  { _pimpl->removeExtraRequire( capability ); }
  void Resolver::removeConflict (const Capability & capability)
  { _pimpl->removeExtraConflict( capability ); }
  const CapabilitySet Resolver::getRequire ()
  { return _pimpl->extraRequires();}
  const CapabilitySet Resolver::getConflict ()
  { return _pimpl->extraConflicts();}      
  std::list<PoolItem> Resolver::problematicUpdateItems( void ) const
  { return _pimpl->problematicUpdateItems(); }
  bool Resolver::createSolverTestcase (const std::string & dumpPath)
  { solver::detail::Testcase testcase (dumpPath);
    return testcase.createTestcase(*_pimpl);}
  const solver::detail::ItemCapKindList Resolver::isInstalledBy (const PoolItem item)
  { return _pimpl->isInstalledBy (item); }
  const solver::detail::ItemCapKindList Resolver::installs (const PoolItem item)
  { return _pimpl->installs (item); }
  const solver::detail::ItemCapKindList Resolver::satifiedByInstalled (const PoolItem item)
  { return _pimpl->satifiedByInstalled (item); }
  const solver::detail::ItemCapKindList Resolver::installedSatisfied (const PoolItem item)
  { return _pimpl->installedSatisfied (item); }    
    


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
