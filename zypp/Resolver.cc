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
  { return _pimpl->verifySystem(); }
  bool Resolver::establishPool ()
  { return _pimpl->establishPool(); }
  bool Resolver::freshenPool ()
  { return _pimpl->freshenPool(); }
  bool Resolver::resolvePool ()
  { return _pimpl->resolvePool (); }
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
  bool Resolver::transactResObject( ResObject::constPtr robj, bool install)
  { return _pimpl->transactResObject( robj, install ); }
  bool Resolver::transactResKind( Resolvable::Kind kind )
  { return _pimpl->transactResKind( kind ); }
  void Resolver::transactReset( ResStatus::TransactByValue causer )
  { _pimpl->transactReset( causer ); }
  std::list<PoolItem_Ref> Resolver::problematicUpdateItems( void ) const
  { return _pimpl->problematicUpdateItems(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
