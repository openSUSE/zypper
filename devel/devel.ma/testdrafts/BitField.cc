#include <ctime>

#include <iostream>
#include <list>
#include <map>
#include <set>

#include "Measure.h"
#include "Printing.h"

#include <zypp/base/Logger.h>
#include <zypp/base/LogControl.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/Functional.h>

#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"

#include <zypp/SourceFactory.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

#include "zypp/ResPoolManager.h"
#include "zypp/ResPoolProxy.h"

using namespace std;
using namespace zypp;
using namespace zypp::ui;
using namespace zypp::functor;

///////////////////////////////////////////////////////////////////

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

struct XByInstalled : public std::unary_function<ui::Selectable::constPtr,bool>
{
  bool operator()( const ui::Selectable::constPtr & obj ) const
  {
    return obj->hasInstalledObj();
  }
};

template<class FT>
  void fieldInfo()
  {
    MIL << bit::asString(FT::Mask::value)    << '|' << FT::begin << '-' << FT::end << '|' << FT::size << endl;
    MIL << bit::asString(FT::Mask::inverted) << endl;
  }


void testr()
{
  fieldInfo<ResStatus::StateField>();
  DBG << bit::asString((ResStatus::FieldType)ResStatus::UNINSTALLED) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::INSTALLED) << endl;
  fieldInfo<ResStatus::EstablishField>();
  DBG << bit::asString((ResStatus::FieldType)ResStatus::UNDETERMINED) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::UNNEEDED) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::SATISFIED) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::INCOMPLETE) << endl;
  fieldInfo<ResStatus::TransactField>();
  DBG << bit::asString((ResStatus::FieldType)ResStatus::KEEP_STATE) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::TRANSACT) << endl;
  fieldInfo<ResStatus::TransactByField>();
  DBG << bit::asString((ResStatus::FieldType)ResStatus::SOLVER) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::APPL_LOW) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::APPL_HIGH) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::USER) << endl;
  fieldInfo<ResStatus::TransactDetailField>();
  DBG << bit::asString((ResStatus::FieldType)ResStatus::EXPLICIT_INSTALL) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::SOFT_INSTALL) << endl;
  DBG << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::EXPLICIT_REMOVE) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::SOFT_REMOVE) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::DUE_TO_OBSOLETE) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::DUE_TO_UNLINK) << endl;
  fieldInfo<ResStatus::SolverStateField>();
  DBG << bit::asString((ResStatus::FieldType)ResStatus::NORMAL) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::SEEN) << endl;
  DBG << bit::asString((ResStatus::FieldType)ResStatus::IMPOSSIBLE) << endl;
}



///////////////////////////////////////////////////////////////////

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  string infile( "p" );
  if (argc >= 2 )
    infile = argv[1];

  MIL << ResStatus::toBeUninstalledDueToUnlink << endl;
  MIL << ResStatus::toBeUninstalledDueToObsolete << endl;
  testr();
  return 0;

  NVRAD a( "foo", Edition("1.1") );
  NVRAD b( "foo", Edition("1.0") );
  SEC << (a==b) << endl;
  SEC << (a!=b) << endl;
  SEC << (a<b) << endl;
  set<NVRAD> s;
  s.insert(a);
  s.insert(b);
  SEC << s.size() << endl;
  return 0;

  Url url("dir:/Local/ma/zypp/libzypp/devel/devel.ma/CD1");
  Measure x( "SourceFactory.create" );
  Source_Ref src( SourceFactory().createFrom( url ) );
  x.stop();
  Source_Ref trg( SourceFactory().createFrom( url ) );

  //Source_Ref src( SourceFactory().createFrom( new source::susetags::SuseTagsImpl(infile) ) );
  //MIL << src.resolvables().size() << endl;

  ResPoolManager pool;
  x.start( "pool.insert" );
  pool.insert( src.resolvables().begin(), src.resolvables().end() );
  x.stop();
  MIL << pool << endl;

  ResPool query( pool.accessor() );
  rstats( query.begin(), query.end() );

  ResPoolProxy y2pm( query );

  pool.insert( trg.resolvables().begin(), trg.resolvables().end(), true );
  y2pm = ResPoolProxy( query );




  INT << "===[END]============================================" << endl << endl;
  return 0;
}

