#include <ctime>

#include <iostream>
#include <list>
#include <map>
#include <set>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>

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

///////////////////////////////////////////////////////////////////
#if 0
namespace zypp
{
  /**
  */

  template<class _Res>
    struct PoolItem
    {
      typedef ResTraits<_Res>::PtrType      Res_Ptr;
      typedef ResTraits<_Res>::constPtrType Res_constPtr;

      PoolItem( Res_Ptr res_r )
      : _resolvable( res_r )
      {}

      ResStatus _status;
      Res_Ptr   _resolvable;
    };

  /**
  */
  class Pool
  {
  public:
    typedef PoolItem                Item;
    typedef PoolItem::Res           Res;
    typedef PoolItem::Res_Ptr       Res_Ptr;
    typedef PoolItem::Res_constPtr  Res_constPtr;

    //private:
    typedef ResObject                     ResT;
    typedef ResTraits<ResT>::KindType     ResKindT;
    typedef ResTraits<ResT>::PtrType      Res_Ptr;
    typedef ResTraits<ResT>::constPtrType Res_constPtr;

    typedef PoolItem<ResT>            ItemT;
    typedef std::set<ItemT>           KindStoreT;
    typedef std::map<ResKindT,ItemT>  StorageT;

  public:

    template <class _InputIterator>
      void addResolvables(  _InputIterator first_r, _InputIterator last_r )



  //private:
    /**  */
    StorageT _store;
    /**  */
    StorageT & store()
    { return _store; }
    /**  */
    const StorageT & store() const
    { return _store; }
  };

}
///////////////////////////////////////////////////////////////////

#endif
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


template <class _Functor>
  class RefFunctor
  {
  public:
    typedef typename _Functor::argument_type argument_type;
    typedef typename _Functor::result_type   result_type;

    RefFunctor( _Functor & f_r )
    : _f( f_r )
    {}

    result_type operator()( argument_type a1 ) const
    {
      return _f.operator()( a1 );
    }

  private:
    _Functor & _f;
  };

template <class _Functor>
  RefFunctor<_Functor> refFunctor( _Functor & f_r )
  { return RefFunctor<_Functor>( f_r ); }

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

struct Rstats : public std::unary_function<Resolvable::constPtr, void>
{
  void operator()( Resolvable::constPtr ptr )
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
    _store.insert( ptr );
  }

  std::set<Resolvable::Ptr> _store;

  typedef std::set<Resolvable::Ptr>  ContainerT;
  typedef ContainerT::iterator       IteratorT;
  typedef ContainerT::const_iterator ConstIteratorT;

  ConstIteratorT begin() const
  { return _store.begin(); }

  ConstIteratorT end() const
  { return _store.end(); }


  template<class _Filter>
    boost::filter_iterator<_Filter, ConstIteratorT> begin() const
    { return boost::make_filter_iterator( _Filter(), begin(), end() ); }

  template<class _Filter>
    boost::filter_iterator<_Filter, ConstIteratorT> begin( _Filter f ) const
    { return boost::make_filter_iterator( f, begin(), end() ); }

  template<class _Filter>
    boost::filter_iterator<_Filter, ConstIteratorT> end() const
    { return boost::make_filter_iterator( _Filter(), end(), end() ); }

  template<class _Filter>
    boost::filter_iterator<_Filter, ConstIteratorT> end( _Filter f ) const
    { return boost::make_filter_iterator( f, end(), end() ); }

};

///////////////////////////////////////////////////////////////////

struct xTrue
{
  bool operator()( Resolvable::Ptr p ) const
  {
    SEC << __FUNCTION__ << ' ' << p << endl;
    return true;
  }
};




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
  for_each( src.resolvables().begin(), src.resolvables().end(), refFunctor(fakeconv) );

  Rstats rstats = Rstats();
  for_each( fakeconv.begin( resfilter::byKind<Package>() ),
            fakeconv.end( resfilter::ByName( "Foo" ) ),
            refFunctor(rstats) );
  MIL << rstats << endl;


#if 0
  rstats = Rstats();
  for_each( boost::make_filter_iterator( resfilter::byKind<Package>(), begin, end ),
            boost::make_filter_iterator( resfilter::byKind<Package>(), end, end ),
            refFunctor(rstats) );
  MIL << rstats << endl;
#endif



  INT << "===[END]============================================" << endl;
  return 0;
}

