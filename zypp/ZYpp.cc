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

  ResPool ZYpp::pool() const
  { return _pimpl->pool(); }

  ResPoolProxy ZYpp::poolProxy() const
  { return _pimpl->poolProxy(); }

  Resolver_Ptr ZYpp::resolver() const
  { return _pimpl->resolver(); }

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to Impl:
  //
  ///////////////////////////////////////////////////////////////////

  //SourceFeed_Ref ZYpp::sourceFeed() const
  //{ return _pimpl->sourceFeed(); }

  void ZYpp::addResolvables (const ResStore& store, bool installed)
  {
    _pimpl->addResolvables (store, installed);
  }

  void ZYpp::removeResolvables (const ResStore& store)
  {
    _pimpl->addResolvables (store);
  }

  ///////////////////////////////////////////////////////////////////

  Target_Ptr ZYpp::target() const
  { return _pimpl->target(); }

  void ZYpp::initTarget(const Pathname & root, bool commit_only)
  { _pimpl->initTarget(root, commit_only); }

  void ZYpp::finishTarget()
  { _pimpl->finishTarget(); }

  ZYpp::CommitResult ZYpp::commit( int medianr_r )
  { return _pimpl->commit( medianr_r ); }

  ///////////////////////////////////////////////////////////////////

  void ZYpp::setTextLocale( const Locale & textLocale_r )
  { _pimpl->setTextLocale( textLocale_r ); }

  Locale ZYpp::getTextLocale() const
  { return _pimpl->getTextLocale(); }

  void ZYpp::setRequestedLocales( const LocaleSet & locales_r )
  { _pimpl->setRequestedLocales( locales_r ); }

  ZYpp::LocaleSet ZYpp::getRequestedLocales() const
  { return _pimpl->getRequestedLocales(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
