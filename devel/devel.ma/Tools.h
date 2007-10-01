#ifndef Tools_h
#define Tools_h

#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <set>

#include "Printing.h"
#include "FakePool.h"

#include <zypp/base/Easy.h>
#include <zypp/base/Counter.h>
#include <zypp/base/Measure.h>

#include <zypp/PathInfo.h>
#include <zypp/Date.h>
#include <zypp/ResObject.h>
#include <zypp/pool/PoolStats.h>

#include <zypp/Repository.h>
#include <zypp/RepoManager.h>

using namespace zypp;
using zypp::debug::Measure;
using std::endl;

///////////////////////////////////////////////////////////////////

#define for_(IT,BEG,END) for ( typeof(BEG) IT = BEG; IT != END; ++IT )

///////////////////////////////////////////////////////////////////
//

void waitForInput()
{
  int i;
  USR << "WAITING FOR INPUT!" << endl;
  std::cin >> i;
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
  { USR << pi << " (" << pi.resolvable().get() << ")" <<endl; }
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

  RepoManagerOptions mgropt;
  mgropt.repoCachePath    = mgrdir_r/"cache";
  mgropt.repoRawCachePath = mgrdir_r/"raw_cache";
  mgropt.knownReposPath   = mgrdir_r/"repos.d/";

  return RepoManager( mgropt );
}

///////////////////////////////////////////////////////////////////
#endif // Tools_h
