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
#include <zypp/Bit.h>

using namespace std;
using namespace zypp;
using namespace zypp::functor;

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

///////////////////////////////////////////////////////////////////

struct SelItem
{
  SelItem( ResObject::Ptr ptr_r )
  : _status( 0 )
  , _ptr( ptr_r )
  {}

  unsigned       _status;
  ResObject::Ptr _ptr;

  operator ResObject::Ptr()
  { return _ptr; }

  operator ResObject::constPtr() const
  { return _ptr; }

  ResObject::Ptr operator->()
  { return _ptr; }

  ResObject::constPtr operator->() const
  { return _ptr; }

  bool operator<( const SelItem & rhs ) const
  { return _ptr < rhs._ptr; }
};


struct FakeConv : public std::unary_function<Resolvable::Ptr, void>
{
  void operator()( Resolvable::Ptr ptr )
  {
    if ( ptr->name()[0] == 's' )
      {
        ptr = fakeResKind<detail::SelectionImpl>( ptr );
      }
    else if ( ptr->name()[0] == 'p' )
      {
        ptr = fakeResKind<detail::PatchImpl>( ptr );
      }
    _store.insert( SelItem(asKind<ResObject>(ptr)) );
  }


  typedef SelItem           ValueT;
  typedef std::set<ValueT>  ContainerT;

  ContainerT _store;

  typedef ContainerT::iterator       IteratorT;
  typedef ContainerT::const_iterator ConstIteratorT;

  ConstIteratorT begin() const
  { return _store.begin(); }

  ConstIteratorT end() const
  { return _store.end(); }

  template<class _Filter>
    filter_iterator<_Filter, ConstIteratorT> begin() const
    { return make_filter_iterator( _Filter(), begin(), end() ); }

  template<class _Filter>
    filter_iterator<_Filter, ConstIteratorT> begin( _Filter f ) const
    { return make_filter_iterator( f, begin(), end() ); }

  template<class _Filter>
    filter_iterator<_Filter, ConstIteratorT> end() const
    { return make_filter_iterator( _Filter(), end(), end() ); }

  template<class _Filter>
    filter_iterator<_Filter, ConstIteratorT> end( _Filter f ) const
    { return make_filter_iterator( f, end(), end() ); }


};

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
  for_each( src.resolvables().begin(), src.resolvables().end(), functorRef(fakeconv) );

  Rstats rstats = Rstats();
  for_each( fakeconv.begin( byKind<Package>() ),
            fakeconv.end( byKind<Package>() ),
            functorRef(rstats) );
  MIL << rstats << endl;

  SelItem it( *fakeconv.begin() );
  SEC << it->kind() << it->name() << endl;


  INT << "===[END]============================================" << endl;
  return 0;
}

