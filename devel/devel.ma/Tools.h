#ifndef Tools_h
#define Tools_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <map>
#include <set>

#include "Printing.h"
#include "FakePool.h"

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
template<class _Condition>
  struct SetTrue
  {
    SetTrue( _Condition cond_r )
    : _cond( cond_r )
    {}

    template<class _Tp>
      bool operator()( _Tp t ) const
      {
        _cond( t );
        return true;
      }

    _Condition _cond;
  };

template<class _Condition>
  inline SetTrue<_Condition> setTrue_c( _Condition cond_r )
  {
    return SetTrue<_Condition>( cond_r );
  }

struct PrintPoolItem
{
  void operator()( const PoolItem & pi ) const
  { USR << pi << endl; }
};

template <class _Iterator>
  std::ostream & vdumpPoolStats( std::ostream & str,
                                 _Iterator begin_r, _Iterator end_r )
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

template<class _Iterator>
  void rstats( _Iterator begin, _Iterator end )
  {
    DBG << __PRETTY_FUNCTION__ << endl;
    Rstats stats;
    for_each( begin, end, functor::functorRef<void,ResObject::constPtr>(stats) );
    MIL << stats << endl;
  }

template<class _Container>
  void rstats( const _Container & c )
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

template<class _Res>
ui::Selectable::Ptr getSel( const std::string & name_r )
{
  ResPoolProxy uipool( getZYpp()->poolProxy() );
  for_(it, uipool.byKindBegin<_Res>(), uipool.byKindEnd<_Res>() )
  {
    if ( (*it)->name() == name_r )
      return (*it);
  }
  return 0;
}



template<class _Res>
PoolItem getPi( const std::string & alias_r, const std::string & name_r, const Edition & ed_r, const Arch & arch_r )
{
  PoolItem ret;
  ResPool pool( getZYpp()->pool() );
  for_(it, pool.byIdentBegin<_Res>(name_r), pool.byIdentEnd<_Res>(name_r) )
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
template<class _Res>
PoolItem getPi( const std::string & name_r, const Edition & ed_r, const Arch & arch_r )
{
  return getPi<_Res>( "", name_r, ed_r, arch_r );
}
template<class _Res>
PoolItem getPi( const std::string & name_r )
{
  return getPi<_Res>( name_r, Edition(), Arch_empty );
}
template<class _Res>
PoolItem getPi( const std::string & name_r, const Edition & ed_r )
{
  return getPi<_Res>( name_r, ed_r, Arch_empty );
}
template<class _Res>
PoolItem getPi( const std::string & name_r, const Arch & arch_r )
{
  return getPi<_Res>( name_r, Edition(), arch_r );
}

///////////////////////////////////////////////////////////////////
#endif // Tools_h
