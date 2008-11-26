#include "Tools.h"
#include <zypp/ResObjects.h>

#include "zypp/pool/GetResolvablesToInsDel.h"

Pathname mroot( "/tmp/Bb" );
TestSetup test( mroot, Arch_ppc64 );

bool checkCaps( Capabilities caps_r );

struct RunableCache
{
  typedef std::tr1::unordered_map<sat::Solvable,TriBool> CacheType;
  typedef std::vector<sat::Solvable>                     AnalyzeStack;

  RunableCache()
  : _ltag( "[0000]" )
  {}

  /**
   * Test whether there is a runable provider for each requirement.
   */
  bool isRunable( const PoolItem & pi ) const
  { return isRunable( pi.satSolvable() ); }

  bool isRunable( sat::Solvable solv_r ) const
  {
    tribool & cent( get( solv_r ) );
    if ( indeterminate( cent ) )
      return (cent = analyze( solv_r ));
    return cent;
  }

  /**
   * Test whether there is a runable provider for each pre-requirement.
   */
  bool isInstallable( const PoolItem & pi ) const
  { return isInstallable( pi.satSolvable() ); }

  bool isInstallable( sat::Solvable solv_r ) const
  {
    tribool & cent( get( solv_r ) );
    if ( cent )
      return true; // if runable then also installable.
    return checkCaps( solv_r.prerequires() );
  }

  /** Clear the cache. */
  void clear() const
  {
    _cache.clear();
    _stack.clear();
    _ltag = "[0000]";
  }

  private:
    /**
     * Determine whether this solvable is runable.
     */
    bool analyze( sat::Solvable solv_r ) const
    {
      if ( ! push( solv_r ) )
      {
        if ( _stack.back() != solv_r )
          SEC << _ltag << "??Already on stack: " << solv_r << " " << _stack << endl;
        // else it's a self requirement
        return true; // assume runnable?
      }

      INT << _ltag << "->" << solv_r << " " << _stack << endl;
      bool ret = checkCaps( solv_r.requires() );
      if ( ! ret )
      {
        ERR << " Not runable: " << solv_r << endl;
      }
      INT << _ltag << "<-" << solv_r << " " << _stack << endl;

      if ( ! pop( solv_r ) )
      {
        INT << "Stack corrupted! Expect " << solv_r << " " << _stack << endl;
      }
      return ret;
    }

    /**
     * For each capability find a runable provider.
     */
    bool checkCaps( Capabilities caps_r ) const
    {
      for_( it, caps_r.begin(), caps_r.end() )
      {
        if ( ! findRunableProvider( *it ) )
          return false;
      }
      return true;
    }

    /**
     * Find a runable provider of a capability on system.
     *
     * A runable package is already installed and all of
     * its requirements are met by runable packages.
     */
    bool findRunableProvider( Capability cap_r ) const
    {
      _MIL("findRunableProvider") << _ltag << "  " << cap_r << endl;
      sat::WhatProvides prv( cap_r );
      for_( pit, prv.begin(), prv.end() )
      {
        if ( ! *pit )
        {
          _DBG("findRunableProvider") << _ltag << "     by system" << endl;
          return true; // noSolvable provides: i.e. system provides
        }

        PoolItem pi( *pit );
        if ( pi.status().onSystem() )
        {
          if ( isRunable( pi ) )
          {
            _DBG("findRunableProvider") << _ltag << "    " << pi << endl;
            return true;
          }
          else
          {
            _WAR("findRunableProvider") << _ltag << "    " << pi << endl;
          }
        }
      }
      ERR << _ltag << "    NO on system provider for " << cap_r << endl;
      return false;
    }

  private:
    /** Push a new solvable to the AnalyzeStack, or return false is already on stack. */
    bool push( sat::Solvable solv_r ) const
    {
      if ( find( _stack.begin(), _stack.end(), solv_r ) == _stack.end() )
      {
        _stack.push_back( solv_r );
        _ltag = str::form( "[%04u]", _stack.size() );
        return true;
      }
      // cycle?
      return false;
    }

    /** Pop solvable from AnalyzeStack (expecting it to be \c solv_r). */
    bool pop( sat::Solvable solv_r ) const
    {
      if ( _stack.back() == solv_r )
      {
        _stack.pop_back();
        _ltag = str::form( "[%04u]", _stack.size() );
        return true;
      }
      // stack corrupted?
      return false;
    }

    /** Return cache entry, initializing new entries with \ref indeterminate.*/
    tribool & get( sat::Solvable solv_r ) const
    {
      CacheType::iterator it( _cache.find( solv_r ) );
      if ( it == _cache.end() )
        return (_cache[solv_r] = indeterminate);
      return _cache[solv_r];
    }

    mutable CacheType    _cache;
    mutable AnalyzeStack _stack;
    mutable std::string  _ltag;
};

RunableCache rcache;

//==================================================

bool upgrade()
{
  bool rres = false;
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    UpgradeStatistics u;
    rres = getZYpp()->resolver()->doUpgrade( u );
  }
  if ( ! rres )
  {
    ERR << "upgrade " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "upgrade " << rres << endl;
  return true;
}

bool solve()
{
  static unsigned run = 0;
  USR << "Solve " << run++ << endl;
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }

  return true;
}

bool verify()
{
  bool rres = solve();
  ResPool pool( test.pool() );
  for_( it, make_filter_begin<resfilter::ByTransact>(pool),
        make_filter_end<resfilter::ByTransact>(pool) )
  {
    if ( it->status().transacts() &&
         it->status().isBySolver() )
    {
      WAR << "MISSING " << *it << endl;
    }
  }
  return rres;
}

inline void save()
{
  test.poolProxy().saveState();
}

inline void restore()
{
  test.poolProxy().restoreState();
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  Pathname mroot( "/tmp/Bb" );
  TestSetup test( mroot, Arch_ppc64 );

  ResPool pool( test.pool() );
  sat::Pool satpool( test.satpool() );

  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    test.loadTarget();
    test.loadTestcaseRepos( "/suse/ma/BUGS/439802/bug439802/YaST2/solverTestcase" );
  }

  if ( 0 )
  {
    PoolItem p = getPi<Package>( "bash", Edition("3.1-24.14"), Arch("ppc64") );
    p.status().setTransact( true, ResStatus::USER );
  }

  save();
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    getPi<Product>( "SUSE_SLES", Edition("11"), Arch("ppc64") ).status().setTransact( true, ResStatus::USER );
    getPi<Package>( "sles-release", Edition("11-54.3"), Arch("ppc64") ).status().setTransact( true, ResStatus::USER );
    upgrade();
  }
  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  pool::GetResolvablesToInsDel collect( pool, pool::GetResolvablesToInsDel::ORDER_BY_MEDIANR );
  restore();
  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  {
    for_( it, collect._toDelete.begin(), collect._toDelete.end() )
    {
      it->status().setTransact( true, ResStatus::USER );
      SEC << *it << endl;
//       vdumpPoolStats( SEC << "Transacting:"<< endl,
//                       make_filter_begin<resfilter::ByTransact>(pool),
//                       make_filter_end<resfilter::ByTransact>(pool) ) << endl;

      (rcache.isInstallable( *it ) ? MIL : ERR) << *it << " deletable?" << endl;
    }
  }

  if ( 0 ) {
    for_( it, collect._toInstall.begin(), collect._toInstall.end() )
    {
      it->status().setTransact( true, ResStatus::USER );
      SEC << *it << endl;
//       vdumpPoolStats( SEC << "Transacting:"<< endl,
//                       make_filter_begin<resfilter::ByTransact>(pool),
//                       make_filter_end<resfilter::ByTransact>(pool) ) << endl;
      (rcache.isInstallable( *it ) ? MIL : ERR) << *it << " installable?" << endl;
    }
  }


#if 0
  //getPi<>( "", "", Edition(""), Arch("") );
  getPi<Product>( "SUSE_SLES", Edition("11"), Arch("ppc64") ).status().setTransact( true, ResStatus::USER );
  getPi<Package>( "sles-release", Edition("11-54.3"), Arch("ppc64") ).status().setTransact( true, ResStatus::USER );

  ResPool pool( test.pool() );
  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;
  upgrade();
  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  pool::GetResolvablesToInsDel collect( pool, pool::GetResolvablesToInsDel::ORDER_BY_MEDIANR );
  MIL << "GetResolvablesToInsDel:" << endl << collect << endl;

  if ( 1 )
  {
    // Collect until the 1st package from an unwanted media occurs.
    // Further collection could violate install order.
    bool hitUnwantedMedia = false;
    typedef pool::GetResolvablesToInsDel::PoolItemList PoolItemList;
    PoolItemList::iterator fst=collect._toInstall.end();
    for ( PoolItemList::iterator it = collect._toInstall.begin(); it != collect._toInstall.end(); ++it)
    {
      ResObject::constPtr res( it->resolvable() );

      if ( hitUnwantedMedia
           || ( res->mediaNr() && res->mediaNr() != 1 ) )
      {
        if ( !hitUnwantedMedia )
          fst=it;
        hitUnwantedMedia = true;
      }
    }
    dumpRange( WAR << "toInstall1: " << endl,
               collect._toInstall.begin(), fst ) << endl;
    dumpRange( WAR << "toInstall2: " << endl,
               fst, collect._toInstall.end() ) << endl;
    dumpRange( ERR << "toDelete: " << endl,
               collect._toDelete.begin(), collect._toDelete.end() ) << endl;
  }
  else
  {
    dumpRange( WAR << "toInstall: " << endl,
               collect._toInstall.begin(), collect._toInstall.end() ) << endl;
    dumpRange( ERR << "toDelete: " << endl,
               collect._toDelete.begin(), collect._toDelete.end() ) << endl;
  }
  INT << "===[END]============================================" << endl << endl;
  return 0;
#endif





  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::TmpLineWriter shutUp;
  return 0;
}

