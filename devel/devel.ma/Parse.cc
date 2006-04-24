#include <ctime>
#include <iostream>
#include "Tools.h"

#include <zypp/base/PtrTypes.h>
#include <zypp/base/Exception.h>
#include <zypp/base/ProvideNumericId.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"
#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>

#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Package.h"
#include "zypp/Language.h"
#include "zypp/NameKindProxy.h"


using namespace std;
using namespace zypp;
using namespace zypp::ui;
using namespace zypp::functor;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

namespace container
{
  template<class _Tp>
    bool isIn( const std::set<_Tp> & cont, const typename std::set<_Tp>::value_type & val )
    { return cont.find( val ) != cont.end(); }
}

///////////////////////////////////////////////////////////////////

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

template <class _Iterator, class _Filter, class _Function>
  inline _Function for_each_if( _Iterator begin_r, _Iterator end_r,
                                _Filter filter_r,
                                _Function fnc_r )
  {
    for ( _Iterator it = begin_r; it != end_r; ++it )
      {
        if ( filter_r( *it ) )
          {
            fnc_r( *it );
          }
      }
    return fnc_r;
  }

struct PrintPoolItem
{
  void operator()( const PoolItem & pi ) const
  {
    USR << "S" << pi->source().numericId()
        << "/M" << mediaId(pi)
        << " " << pi << endl;
  }
  unsigned mediaId( const PoolItem & pi ) const
  {
    Package::constPtr pkg( asKind<Package>(pi.resolvable()) );
    if ( pkg )
      return pkg->mediaId();
    return 0;
  }
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

struct PoolItemSelect
{
  void operator()( const PoolItem & pi ) const
  {
    if ( pi->source().numericId() == 2 )
      pi.status().setTransact( true, ResStatus::USER );
  }
};

///////////////////////////////////////////////////////////////////
typedef std::list<PoolItem> PoolItemList;
typedef std::set<PoolItem>  PoolItemSet;
#include "zypp/solver/detail/InstallOrder.h"
using zypp::solver::detail::InstallOrder;
#include "Iorder.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  struct CollectTransacting
  {
    typedef std::list<PoolItem> PoolItemList;

    void operator()( const PoolItem & pi )
    {
      if ( pi.status().isToBeInstalled() )
        {
          _toInstall.insert( pi );
        }
      else if ( pi.status().isToBeUninstalled() )
        {
          if ( pi.status().isToBeUninstalledDueToObsolete()
               || pi.status().isToBeUninstalledDueToUpgrade() )
            _skipToDelete.insert( pi );
          else
            _toDelete.insert( pi );
        }
    }

    PoolItemSet _toInstall;
    PoolItemSet _toDelete;
    PoolItemSet _skipToDelete;
  };

  std::ostream & operator<<( std::ostream & str, const CollectTransacting & obj )
  {
    str << "CollectTransacting:" << endl;
    dumpPoolStats( str << " toInstall: ",
                   obj._toInstall.begin(), obj._toInstall.end() ) << endl;
    dumpPoolStats( str << " toDelete: ",
                   obj._toDelete.begin(), obj._toDelete.end() ) << endl;
    dumpPoolStats( str << " skipToDelete: ",
                   obj._skipToDelete.begin(), obj._skipToDelete.end() ) << endl;
    return str;
  }
}

///////////////////////////////////////////////////////////////////
#if 0
template<class _InstIterator, class _DelIterator, class _OutputIterator>
void strip_obsoleted_to_delete( _InstIterator instBegin_r, _InstIterator instEnd_r,
                                _DelIterator  delBegin_r,  _DelIterator  delEnd_r,
                                _OutputIterator skip_r )
  {
    if ( instBegin_r == instEnd_r
         || delBegin_r == delEnd_r )
    return; // ---> nothing to do

    // build obsoletes from inst
    CapSet obsoletes;
    for ( /**/; instBegin_r != instEnd_r; ++instBegin_r )
    {
      //xxxxx
      //PoolItem_Ref item( *it );
      //obsoletes.insert( item->dep(Dep::OBSOLETES).begin(), item->dep(Dep::OBSOLETES).end() );
    }
  if ( obsoletes.size() == 0 )
    return; // ---> nothing to do

  // match them... ;(
  PoolItemList undelayed;
  // forall applDelete Packages...
  for ( PoolItemList::iterator it = deleteList_r.begin();
	it != deleteList_r.end(); ++it )
    {
      PoolItem_Ref ipkg( *it );
      bool delayPkg = false;
      // ...check whether an obsoletes....
      for ( CapSet::iterator obs = obsoletes.begin();
            ! delayPkg && obs != obsoletes.end(); ++obs )
        {
          // ...matches anything provided by the package?
          for ( CapSet::const_iterator prov = ipkg->dep(Dep::PROVIDES).begin();
                prov != ipkg->dep(Dep::PROVIDES).end(); ++prov )
            {
              if ( obs->matches( *prov ) == CapMatch::yes )
                {
                  // if so, delay package deletion
                  DBG << "Ignore appl_delete (should be obsoleted): " << ipkg << endl;
                  delayPkg = true;
                  ipkg.status().setTransact( false, ResStatus::USER );
                  break;
                }
            }
        }
      if ( ! delayPkg ) {
        DBG << "undelayed " << ipkg << endl;
        undelayed.push_back( ipkg );
      }
    }
  // Puhh...
  deleteList_r.swap( undelayed );

}
#endif
///////////////////////////////////////////////////////////////////

struct SetTransactValue
{
  SetTransactValue( ResStatus::TransactValue newVal_r, ResStatus::TransactByValue causer_r )
  : _newVal( newVal_r )
  , _causer( causer_r )
  {}

  ResStatus::TransactValue   _newVal;
  ResStatus::TransactByValue _causer;

  bool operator()( const PoolItem & pi ) const
  { return pi.status().setTransactValue( _newVal, _causer ); }
};

struct StatusReset : public SetTransactValue
{
  StatusReset()
  : SetTransactValue( ResStatus::KEEP_STATE, ResStatus::USER )
  {}
};


inline bool selectForTransact( const NameKindProxy & nkp )
{
  if ( nkp.availableEmpty() )
    return false;

  return nkp.availableBegin()->status().setTransact( true, ResStatus::USER );
}

  struct PrintX
  {
    PrintX( map<NVRA,string> & nvrmap )
    : m(nvrmap)
    {}
    void operator()( const NVRA & i ) const
    {
      INT << i << ": " << m[i] << endl;
    }
    map<NVRA,string> & m;
  };

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "xxx" );
  INT << "===[START]==========================================" << endl;

  ResPool pool( getZYpp()->pool() );

  if ( 0 )
    {
      getZYpp()->initTarget( sysRoot );
      getZYpp()->addResolvables( getZYpp()->target()->resolvables(), true );
      INT << "Added target: " << pool << endl;
    }

  if ( 1 ) {
    SourceManager::sourceManager()->restore( sysRoot );
    if ( SourceManager::sourceManager()->allSources().empty() )
      {
        Source_Ref src1;
        if ( 1 )
          {
            Url myUrl( "dir:/mounts/machcd2/CDs/SLES-10-CD-i386-Beta10/CD1" );
            USR << src1 << endl;
            try
              {
                src1 = SourceFactory().createFrom( myUrl, "/", Date::now().asSeconds() );
              }
            catch ( const Exception & )
              {
                ;
              }
            USR << src1 << endl;
          }
        SourceManager::sourceManager()->addSource( src1 );
        SourceManager::sourceManager()->store( sysRoot, true );
      }

    Source_Ref src( *SourceManager::sourceManager()->Source_begin() );
  }

  USR << "=======================" << endl;

  SourceManager::sourceManager()->removeSource( SourceManager::sourceManager()->Source_begin()->numericId() );
  SourceManager::sourceManager()->store( sysRoot, true );


#if 0
  Source_Ref src1( createSource( "dir:/mounts/machcd2/CDs/SLES-10-CD-i386-Beta10/CD1" ) );
  Source_Ref src2( createSource( "dir:/mounts/machcd2/kukuk/sles10-sp-i386/CD1" ) );
  getZYpp()->addResolvables( src1.resolvables() );
  getZYpp()->addResolvables( src2.resolvables() );
  INT << "Pool: " << pool << endl;

  selectForTransact( nameKindProxy<Pattern>( pool, "default" ) );
  selectForTransact( nameKindProxy<Pattern>( pool, "x11" ) );
  selectForTransact( nameKindProxy<Pattern>( pool, "kde" ) );
  selectForTransact( nameKindProxy<Pattern>( pool, "OOo" ) );

  vdumpPoolStats( INT << "Transacting: ",
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  bool eres, rres;
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    eres = getZYpp()->resolver()->establishPool();
    rres = getZYpp()->resolver()->resolvePool();
  }
  MIL << "est " << eres << " slv " << rres << endl;

  vdumpPoolStats( INT << "Transacting: ",
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;


  ZYppCommitPolicy policy;
  policy.rpmNoSignature();
  if ( 0 )
    {
      ZYppCommitResult res( getZYpp()->commit( policy ) );
    }
  else
    {
      for ( unsigned mnr = 1; mnr < 6; ++mnr )
        {
          policy.restrictToMedia( mnr );
          ZYppCommitResult res( getZYpp()->commit( policy ) );
        }
    }
#endif
  INT << "===[END]============================================" << endl << endl;
  return 0;
}

