#include <ctime>

#include <iostream>
#include <list>
#include <map>
#include <set>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/Functional.h>

#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"

#include <zypp/SourceFactory.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

#include "zypp/ResPoolManager.h"
#include "zypp/ui/Selectable.h"

using namespace std;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::resfilter;

///////////////////////////////////////////////////////////////////

struct Print : public std::unary_function<ResObject::constPtr, bool>
{
  bool operator()( ResObject::constPtr ptr )
  {
    USR << *ptr << endl;
    return true;
  }
};

///////////////////////////////////////////////////////////////////

template<class _IntT>
  struct Counter
  {
    Counter()
    : _value( _IntT(0) )
    {}

    Counter( _IntT value_r )
    : _value( _IntT( value_r ) )
    {}

    operator _IntT &()
    { return _value; }

    operator const _IntT &() const
    { return _value; }

    _IntT _value;
  };

struct Rstats : public std::unary_function<ResObject::constPtr, void>
{
  void operator()( ResObject::constPtr ptr )
  {
    ++_total;
    ++_perKind[ptr->kind()];
  }

  typedef std::map<ResolvableTraits::KindType,Counter<unsigned> > KindMap;
  Counter<unsigned> _total;
  KindMap           _perKind;
};

std::ostream & operator<<( std::ostream & str, const Rstats & obj )
{
  str << "Total: " << obj._total;
  for( Rstats::KindMap::const_iterator it = obj._perKind.begin(); it != obj._perKind.end(); ++it )
    {
      str << endl << "  " << it->first << ":\t" << it->second;
    }
  return str;
}

template<class _Iterator>
  void rstats( _Iterator begin, _Iterator end )
  {
    DBG << __PRETTY_FUNCTION__ << endl;
    Rstats stats;
    for_each( begin, end, functorRef<void,ResObject::constPtr>(stats) );
    MIL << stats << endl;
  }

///////////////////////////////////////////////////////////////////

namespace zypp { namespace ui
{

  struct PP
  {
    typedef std::set<ResPool::Item>         ItemC;
    struct SelC
    {
      void add( ResPool::Item it )
      { available.insert( it ); }
      ItemC installed;
      ItemC available;
    };
    typedef std::map<std::string,SelC>      NameC;
    typedef std::map<ResObject::Kind,NameC> KindC;

    KindC _kinds;

    void operator()( ResPool::Item it )
    {
      _kinds[it->kind()][it->name()].add( it );
    }

    void dumpOn() const
    {
      for ( KindC::const_iterator it = _kinds.begin(); it != _kinds.end(); ++it )
        {
          ERR << it->first << endl;
          for ( NameC::const_iterator nit = it->second.begin(); nit != it->second.end(); ++nit )
            {
              WAR << nit->first << endl;
              MIL << "i " << nit->second.installed.size()
                  << " a " << nit->second.available.size() << endl;
            }
        }
    }
  };

  class ResPoolProxy
  {
  public:

  };



}}



///////////////////////////////////////////////////////////////////

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  string infile( "p" );
  if (argc >= 2 )
    infile = argv[1];

  Source_Ref src( SourceFactory().createFrom( new source::susetags::SuseTagsImpl(infile) ) );
  MIL << src.resolvables().size() << endl;

  ResPoolManager pool;
  pool.insert( src.resolvables().begin(), src.resolvables().end() );
  MIL << pool << endl;

  ResPool query( pool.accessor() );
  rstats( query.begin(), query.end() );

  ui::PP collect;
  for_each( query.begin(), query.end(),
            functorRef<void,ResPool::Item>( collect ) );
  collect.dumpOn();






  return 0;
}

