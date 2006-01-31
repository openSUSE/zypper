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

  void Resolver::verifySystem ()
  { return _pimpl->verifySystem(); }
  void Resolver::establishPool ()
  { return _pimpl->establishPool(); }
  bool Resolver::resolvePool ()
  { return _pimpl->resolvePool (); }
  ResolverProblemList Resolver::problems ()
  { return _pimpl->problems (); }
  bool Resolver::applySolutions( const ProblemSolutionList & solutions )
  { return _pimpl->applySolutions (solutions); }      
  void Resolver::doUpgrade( UpgradeStatistics & opt_stats_r )
  { return _pimpl->doUpgrade(opt_stats_r); }

  // ResolverContext_constPtr bestContext (void) const;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
