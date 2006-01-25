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

#include <zypp/PathInfo.h>
#include <zypp/SourceFactory.h>
#include <zypp/source/Builtin.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

#include <zypp/Resolvable.h>
#include <zypp/Package.h>
#include <zypp/detail/PackageImpl.h>
#include <zypp/Selection.h>
#include <zypp/detail/SelectionImpl.h>
#include <zypp/Patch.h>
#include <zypp/detail/PatchImpl.h>


#include <zypp/ResFilters.h>
#include <zypp/ResStatus.h>
#include <zypp/ResPoolManager.h>

#include <zypp/ZYppFactory.h>

using namespace std;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::resfilter;

///////////////////////////////////////////////////////////////////
namespace zypp
{

}
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

struct Print : public std::unary_function<ResObject::constPtr, bool>
{
  bool operator()( ResObject::constPtr ptr )
  {
    USR << *ptr << endl;
    return true;
  }
};
struct xPrint : public std::unary_function<ResObject::constPtr, bool>
{
  bool operator()( ResObject::constPtr ptr )
  {
    USR << *ptr << endl;
    return isKind<Package>(ptr);
  }
};


///////////////////////////////////////////////////////////////////

template <class _Impl>
  typename _Impl::ResType::Ptr fakeResKind( Resolvable::Ptr from_r )
  {
    // fake different kind based on NVRAD
    NVRAD nvrad( from_r );
    shared_ptr<_Impl> impl;
    return detail::makeResolvableAndImpl( nvrad, impl );
  }

///////////////////////////////////////////////////////////////////

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
    for_each( begin, end, functorRef(stats) );
    MIL << stats << endl;
  }

///////////////////////////////////////////////////////////////////

struct FakeConv : public std::unary_function<Resolvable::Ptr, void>
{
  typedef ResObject::Ptr ValueT;
  typedef std::set<ValueT>  ContainerT;

  void operator()( Resolvable::Ptr ptr )
  {
    if ( ptr->arch() == "noarch" )
      {
        ptr = fakeResKind<detail::PatchImpl>( ptr );
      }
    else if (  ptr->arch() == "i586" )
      {
        ptr = fakeResKind<detail::SelectionImpl>( ptr );
      }
    _store.insert( asKind<ResObject>(ptr) );


    DBG << *ptr << endl << ptr->deps() << endl;

  }

  ContainerT _store;
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{

  struct OnCapMatchCallback : public CapFilterFunctor
  {
    bool operator()( ResObject::constPtr p, const Capability & match ) const
    {
    }
  };

  struct CallOnCapMatchIn : public ResObjectFilterFunctor
  {
    bool operator()( ResObject::constPtr p ) const
    {
      const CapSet & depSet( p->dep( _dep ) ); // dependency set in p to iterate
      ByCapMatch  matching( _cap );            // predicate: true if match with _cap

      invokeOnEach( depSet.begin(), depSet.end(), // iterate this set
                    matching,                     // Filter: if match
                    std::bind1st(_fnc,p) );       // Action: invoke _fnc(p,match)
      // Maybe worth to note: Filter and Action are invoked with the same
      // iterator, thus Action will use the same capability that cause
      // the match in Filter.
    }

    CallOnCapMatchIn( Dep dep_r, const Capability & cap_r,
                      OnCapMatchCallback fnc_r )
    : _dep( dep_r )
    , _cap( cap_r )
    , _fnc( fnc_r )
    {}
    Dep                _dep;
    const Capability & _cap;
    OnCapMatchCallback _fnc;
  };


  void example()
  {
    ResPool            query;
    Dep                dep( Dep::PROVIDES );
    Capability         cap;
    OnCapMatchCallback fnc;
    //-------------

    int ret
    = invokeOnEach( query.byCapabilityIndexBegin( cap.index(), dep ), // begin()
                    query.byCapabilityIndexEnd( cap.index(), dep ),   // end()
                    CallOnCapMatchIn( dep, cap, fnc ) );              // Action(ResObject::constPtr)
  }


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

  Source src( SourceFactory().createFrom( new source::susetags::SuseTagsImpl(infile) ) );
  MIL << src.resolvables().size() << endl;

  FakeConv fakeconv;
  std::copy( src.resolvables().begin(), src.resolvables().end(),
             make_function_output_iterator( functorRef(fakeconv) ) );
  //for_each( src.resolvables().begin(), src.resolvables().end(),
  //          functorRef(fakeconv) );

  rstats( fakeconv._store.begin(), fakeconv._store.end() );

  ResPoolManager pool;
  ResPool query( pool.accessor() );
  rstats( query.begin(), query.end() );

  pool.insert( fakeconv._store.begin(), fakeconv._store.end() );
  MIL << pool << endl;
  rstats( query.begin(), query.end() );

  rstats( query.byKindBegin<Package>(), query.byKindEnd<Package>() );
  rstats( query.byKindBegin<Selection>(), query.byKindEnd<Selection>() );
  rstats( query.byKindBegin<Pattern>(), query.byKindEnd<Pattern>() );
  rstats( query.byKindBegin<Patch>(), query.byKindEnd<Patch>() );
  rstats( query.byNameBegin("rpm"), query.byNameEnd("rpm") );


  SEC << invokeOnEach( query.byNameBegin("rpm"), query.byNameEnd("rpm"),
                       Print() ) << endl;
  SEC << invokeOnEach( query.byNameBegin("rpm"), query.byNameEnd("rpm"),
                       xPrint() ) << endl;

  std::string i( "/bin/sh" );
  SEC << invokeOnEach( query.byCapabilityIndex(i,Dep::PROVIDES),
                       query.byCapabilityIndex(i,Dep::PROVIDES),
                       Print() ) << endl;
  i = "/bin/sh";


  INT << "===[END]============================================" << endl;
  return 0;
}

