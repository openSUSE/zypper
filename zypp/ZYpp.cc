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
//#include "zypp/base/Logger.h"

#include "zypp/ZYpp.h"
#include "zypp/zypp_detail/ZYppImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZYpp::ZYpp
  //	METHOD TYPE : Ctor
  //
  ZYpp::ZYpp( const Impl_Ptr & impl_r )
  : _pimpl( impl_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZYpp::~ZYpp
  //	METHOD TYPE : Dtor
  //
  ZYpp::~ZYpp()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ZYpp::dumpOn
  //	METHOD TYPE : std::ostream &
  //
  std::ostream & ZYpp::dumpOn( std::ostream & str ) const
  {
    return str << *_pimpl;
  }

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to Impl:
  //
  ///////////////////////////////////////////////////////////////////

  SourceFeed_Ref ZYpp::sourceFeed() const
  { return _pimpl->sourceFeed(); }

  ResPool ZYpp::pool() const
  { return _pimpl->pool(); }

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to Impl:
  //
  ///////////////////////////////////////////////////////////////////

  void ZYpp::addResolvables (const ResStore& store, bool installed)
  {
    _pimpl->addResolvables (store, installed);
  }

  void ZYpp::removeResolvables (const ResStore& store)
  {
    _pimpl->addResolvables (store);
  }

  Target_Ptr ZYpp::target() const
  { return _pimpl->target(); }

  void ZYpp::initTarget(const Pathname & root)
  { _pimpl->initTarget(root); }

  void ZYpp::finishTarget()
  { _pimpl->finishTarget(); }




  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
