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
#include "zypp/UpgradeStatistics.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/Testcase.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

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
  { return _pimpl->verifySystem(false); }
  bool Resolver::verifySystem (bool considerNewHardware)
  { return _pimpl->verifySystem(considerNewHardware); }
  bool Resolver::establishPool ()
  { return _pimpl->establishPool(); }
  bool Resolver::freshenPool ()
  { return _pimpl->freshenPool(); }
  bool Resolver::resolvePool ()
  { return _pimpl->resolvePool( false ); }// do not try all possibilities 
  bool Resolver::resolvePool( bool tryAllPossibilities )
  { return _pimpl->resolvePool( tryAllPossibilities ); }
  void Resolver::undo()
  { _pimpl->undo(); }
  solver::detail::ResolverContext_Ptr Resolver::context (void) const
  { return _pimpl->context(); }
  ResolverProblemList Resolver::problems ()
  { return _pimpl->problems (); }
  std::list<std::string> Resolver::problemDescription( void ) const
  { return _pimpl->problemDescription (); }    
  void Resolver::applySolutions( const ProblemSolutionList & solutions )
  { _pimpl->applySolutions (solutions); }      
  void Resolver::doUpgrade( UpgradeStatistics & opt_stats_r )
  { _pimpl->doUpgrade(opt_stats_r); }
  Arch Resolver::architecture() const
  { return _pimpl->architecture(); }
  void Resolver::setArchitecture( const Arch & arch )
  { _pimpl->setArchitecture( arch ); }
  void Resolver::setForceResolve( const bool force )
  { _pimpl->setForceResolve( force ); }
  const bool Resolver::forceResolve()
  { return _pimpl->forceResolve(); }
    void Resolver::setPreferHighestVersion( const bool highestVersion )
  { _pimpl->setPreferHighestVersion( highestVersion ); }
  const bool Resolver::preferHighestVersion()
  { return _pimpl->preferHighestVersion(); }
  bool Resolver::transactResObject( ResObject::constPtr robj, bool install)
  { return _pimpl->transactResObject( robj, install ); }
  bool Resolver::transactResKind( Resolvable::Kind kind )
  { return _pimpl->transactResKind( kind ); }
  void Resolver::transactReset( ResStatus::TransactByValue causer )
  { _pimpl->transactReset( causer ); }
  std::list<PoolItem_Ref> Resolver::problematicUpdateItems( void ) const
  { return _pimpl->problematicUpdateItems(); }
  void Resolver::setTimeout( int seconds )
  { _pimpl->setTimeout( seconds ); }
  void Resolver::setMaxSolverPasses (int count)
  { _pimpl->setMaxSolverPasses( count ); }
  int Resolver::timeout()
  { return _pimpl->timeout(); }
  int Resolver::maxSolverPasses()
  { return _pimpl->maxSolverPasses(); }
  bool Resolver::createSolverTestcase (const std::string & dumpPath)
  { solver::detail::Testcase testcase (dumpPath);
    return testcase.createTestcase(*_pimpl);}
#if 1
  const solver::detail::ItemCapKindList Resolver::isSelectedBy (const PoolItem_Ref item)
  { return _pimpl->isSelectedBy (item); }
  const solver::detail::ItemCapKindList Resolver::selects (const PoolItem_Ref item)
  { return _pimpl->selects (item); }
#endif


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
