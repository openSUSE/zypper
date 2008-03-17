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

#include <zypp/SourceFactory.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

#include "zypp/ResPoolManager.h"

using namespace std;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::resfilter;

namespace zypp
{
}

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

void test( ResPool::const_iterator begin, ResPool::const_iterator end, const Edition & ed, Rel op = Rel::EQ )
{
  SEC << "Serach for editions " << op << ' ' << ed << ':' << endl;
  SEC << invokeOnEach( begin, end,

                       resfilter::byEdition( Edition("2.0-1"), CompareBy<Edition>(op) ),

                       Print() ) << endl;
}

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

  SEC << invokeOnEach( query.begin(), query.end(),
                       Print() ) << endl;

  test( query.begin(), query.end(), Edition("2.0-1"), Rel::LT );
  test( query.begin(), query.end(), Edition("2.0-1"), Rel::LE );
  test( query.begin(), query.end(), Edition("2.0-1"), Rel::EQ );
  test( query.begin(), query.end(), Edition("2.0-1"), Rel::GE );
  test( query.begin(), query.end(), Edition("2.0-1"), Rel::GT );
  test( query.begin(), query.end(), Edition("2.0-1"), Rel::ANY );
  test( query.begin(), query.end(), Edition("2.0-1"), Rel::NONE );



  return 0;
}

