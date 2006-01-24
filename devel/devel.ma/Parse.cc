#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <iterator>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>

#include <zypp/SourceFactory.h>

//#include <zypp/ResStore.h>
//#include <zypp/ResFilters.h>

using std::endl;
#if 0

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
  //
  //  Just for the stats
  //
  ////////////////////////////////////////////////////////////////////////////
  struct Measure
  {
    time_t _begin;
    Measure()
    : _begin( time(NULL) )
    {
      USR << "START MEASURE..." << endl;
    }
    ~Measure()
    {
      USR << "DURATION: " << (time(NULL)-_begin) << " sec." << endl;
    }
  };
  ////////////////////////////////////////////////////////////////////////////




  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  Types
//
////////////////////////////////////////////////////////////////////////////

using namespace zypp;
using namespace std;

////////////////////////////////////

struct ResFilter {};
struct ResActor {};

///////////////////////////////////////////////////////////////////

template<bool _val>
  struct Always
  {
    bool operator()( ResObject::Ptr ) const
    { return _val; }
  };

///////////////////////////////////////////////////////////////////

  /** Filter by name. */
  template <class _Function>
    struct ByNameFilter
    {
      ByNameFilter( const std::string & name_r, const _Function & fnc_r = _Function() )
      : _name( name_r )
      , _fnc( fnc_r )
      {}

      bool operator()( ResObject::Ptr p ) const
      {
        return ( p->name() == _name ) && _fnc( p );
      }

      std::string _name;
      const _Function & _fnc;
    };

  /** Convenience creating appropriate ByNameFilter. */
  template <class _Function>
    ByNameFilter<_Function> byName( const std::string & name_r, const _Function & fnc_r )
    { return ByNameFilter<_Function>( name_r, fnc_r ); }

  ByNameFilter<Always<true> > byName( const std::string & name_r )
  { return ByNameFilter<Always<true> >( name_r ); }


///////////////////////////////////////////////////////////////////

ResStore store;

template <class _Function, class _Filter>
  unsigned sforEach( _Filter filter_r, _Function fnc_r )
  {
    unsigned cnt = 0;
    for ( ResStore::const_iterator it = store.begin(); it != store.end(); ++it )
      {
        if ( filter_r( *it ) )
          {
            if ( fnc_r( *it ) )
              ++cnt;
            else
              break;
          }
      }
    return cnt;
  }

template <class _Function>
  unsigned  sforEach( _Function fnc_r )
  {
    return sforEach( Always<true>(), fnc_r );
  }

#endif
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
//
//  Main
//
////////////////////////////////////////////////////////////////////////////
int main( int argc, char* argv[] )
{
#if 0
  INT << "===[START]==========================================" << endl;
  string infile( "p" );
  if (argc >= 2 )
    infile = argv[1];

  try
    {
      std::list<Package::Ptr> result( source::susetags::parsePackages( infile ) );
      SEC << result.size() << endl;
      store.insert( result.begin(), result.end() );
      MIL << store.size() << endl;
    }
  catch( Exception & excpt )
    {
      ZYPP_RETHROW( excpt );
    }

  INT << sforEach( byName("rpm"), Always<true>() ) << endl;

  INT << "===[END]============================================" << endl;
#endif
  return 0;
}
