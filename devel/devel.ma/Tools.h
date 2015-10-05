#ifndef Tools_h
#define Tools_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <map>
#include <set>

#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "zypp/../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST

#include <zypp/base/Easy.h>
#include <zypp/base/Counter.h>
#include <zypp/base/Measure.h>

#include <zypp/PathInfo.h>
#include <zypp/Date.h>
#include <zypp/ResObject.h>
#include <zypp/pool/PoolStats.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ResPool.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/ui/Selectable.h"
#include <zypp/Repository.h>
#include <zypp/RepoManager.h>


using namespace zypp;
using zypp::debug::Measure;
using std::endl;

///////////////////////////////////////////////////////////////////

#define for_providers_(IT,CAP) for ( sat::WhatProvides::const_iterator IT = sat::WhatProvides( Capability CAP ).begin(), _for_end = sat::WhatProvides().end(); IT != _for_end; ++IT )

///////////////////////////////////////////////////////////////////

template<typename T>
void whichType( T )
{ INT << __PRETTY_FUNCTION__ << endl; }
template<typename T>
void whichType()
{ INT << __PRETTY_FUNCTION__ << endl; }

void waitForInput()
{
  std::string i;
  USR << "WAITING FOR INPUT!" << endl;
  std::cin >> i;
}

///////////////////////////////////////////////////////////////////

void mksrc( const std::string & url, const std::string & alias, RepoManager & repoManager )
{
  RepoInfo nrepo;
  nrepo.setAlias( alias );
  nrepo.setName( alias );
  nrepo.setEnabled( true );
  nrepo.setAutorefresh( false );
  nrepo.addBaseUrl( Url(url) );

  if ( ! repoManager.isCached( nrepo ) )
  {
    repoManager.buildCache( nrepo );
  }

  repoManager.loadFromCache( nrepo );
}

///////////////////////////////////////////////////////////////////
//
template<class TCondition>
  struct SetTrue
  {
    SetTrue( TCondition cond_r )
    : _cond( cond_r )
    {}

    template<class Tp>
      bool operator()( Tp t ) const
      {
        _cond( t );
        return true;
      }

    TCondition _cond;
  };

template<class TCondition>
  inline SetTrue<TCondition> setTrue_c( TCondition cond_r )
  {
    return SetTrue<TCondition>( cond_r );
  }

struct PrintPoolItem
{
  void operator()( const PoolItem & pi ) const
  { USR << pi << endl; }
};

template <class TIterator>
  std::ostream & vdumpPoolStats( std::ostream & str,
                                 TIterator begin_r, TIterator end_r )
  {
    pool::PoolStats stats;
    std::for_each( begin_r, end_r,

                   functor::chain( setTrue_c(PrintPoolItem()),
                                   setTrue_c(functor::functorRef<void,ResObject::constPtr>(stats)) )

                 );
    return str << stats;
  }

///////////////////////////////////////////////////////////////////
// rstats

typedef zypp::pool::PoolStats Rstats;

template<class TIterator>
  void rstats( TIterator begin, TIterator end )
  {
    DBG << __PRETTY_FUNCTION__ << endl;
    Rstats stats;
    for_each( begin, end, functor::functorRef<void,ResObject::constPtr>(stats) );
    MIL << stats << endl;
  }

template<class TContainer>
  void rstats( const TContainer & c )
  {
    rstats( c.begin(), c.end() );
  }

///////////////////////////////////////////////////////////////////

inline RepoManager makeRepoManager( const Pathname & mgrdir_r )
{
  // set via zypp.conf
  return RepoManager();
}

///////////////////////////////////////////////////////////////////

template<class TRes>
ui::Selectable::Ptr getSel( const std::string & name_r )
{
  ResPoolProxy uipool( getZYpp()->poolProxy() );
  for_(it, uipool.byKindBegin<TRes>(), uipool.byKindEnd<TRes>() )
  {
    if ( (*it)->name() == name_r )
      return (*it);
  }
  return 0;
}



template<class TRes>
PoolItem getPi( const std::string & alias_r, const std::string & name_r, const Edition & ed_r, const Arch & arch_r )
{
  PoolItem ret;
  ResPool pool( getZYpp()->pool() );
  for_(it, pool.byIdentBegin<TRes>(name_r), pool.byIdentEnd<TRes>(name_r) )
  {
    if (    ( ed_r.empty()    || ed_r.match((*it)->edition()) == 0 )
         && ( arch_r.empty()  || arch_r == (*it)->arch()  )
         && ( alias_r.empty() || alias_r == (*it)->repository().alias() ) )
    {
      if ( !ret || ret->repository().alias() == sat::Pool::systemRepoAlias() )
      {
        ret = (*it);
        MIL << "    ->" << *it << endl;
      }
      else
      {
        DBG << "    - " << *it << endl;
      }
    }
    else
    {
      DBG << "     ?" << *it << endl;
    }
  }
  return ret;
}
template<class TRes>
PoolItem getPi( const std::string & name_r, const Edition & ed_r, const Arch & arch_r )
{
  return getPi<TRes>( "", name_r, ed_r, arch_r );
}
template<class TRes>
PoolItem getPi( const std::string & name_r )
{
  return getPi<TRes>( name_r, Edition(), Arch_empty );
}
template<class TRes>
PoolItem getPi( const std::string & name_r, const Edition & ed_r )
{
  return getPi<TRes>( name_r, ed_r, Arch_empty );
}
template<class TRes>
PoolItem getPi( const std::string & name_r, const Arch & arch_r )
{
  return getPi<TRes>( name_r, Edition(), arch_r );
}

///////////////////////////////////////////////////////////////////
#endif // Tools_h
