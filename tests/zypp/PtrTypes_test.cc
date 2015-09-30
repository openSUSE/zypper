#include <iostream>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include <zypp/base/PtrTypes.h>
#include <zypp/base/ReferenceCounted.h>
#include <zypp/base/ProvideNumericId.h>

#define BOOST_TEST_MODULE PtrTypes

using std::endl;
using namespace zypp;
using namespace zypp::base;


#define TRACE_TAG DBG << this->numericId() << " " << __PRETTY_FUNCTION__ << endl

/** Logs Ctor, CopyCtor, Assign and Dtor. */
template<class TTrace>
  struct Trace : public ProvideNumericId<TTrace,unsigned>
  {
    Trace()                            { TRACE_TAG; }
    Trace( const Trace & )             { TRACE_TAG; }
    ~Trace()                           { TRACE_TAG; }
    Trace & operator=( const Trace & ) { TRACE_TAG; return *this; }
  };

/** Data class for shared_ptr */
struct NonIntrusive : private Trace<NonIntrusive>
{
  using Trace<NonIntrusive>::numericId;
};

/** Data class for intrusive_ptr */
struct Intrusive : public ReferenceCounted,
                   private Trace<Intrusive>
{
  using Trace<Intrusive>::numericId;
};

namespace zypp
{
  template<>
    inline NonIntrusive * rwcowClone<NonIntrusive>( const NonIntrusive * rhs )
    { return new NonIntrusive( *rhs ); }

  template<>
    inline Intrusive * rwcowClone<Intrusive>( const Intrusive * rhs )
    { return new Intrusive( *rhs ); }

}

/******************************************************************
**
*/
#define T_NULL       assert( !ptr ); assert( ptr == nullptr )
#define T_NOT_NULL   assert( ptr ); assert( ptr != nullptr )
#define T_UNIQUE     assert( ptr.unique() ); assert( ptr.use_count() < 2 )
#define T_NOT_UNIQUE assert( !ptr.unique() ); assert( ptr.use_count() >= 2 )
// Also comapre with underlying shared ptr type.
#define T_EQ(a,b)   assert( a == b ); assert( a == b.cgetPtr() ); assert( a.cgetPtr() == b ); assert( a.cgetPtr() == b.cgetPtr() );
#define T_NE(a,b)   assert( a != b ); assert( a != b.cgetPtr() ); assert( a.cgetPtr() != b ); assert( a.cgetPtr() != b.cgetPtr() );

template<class RW>
  void test()
  {
    MIL << __PRETTY_FUNCTION__ << std::endl;
    // typedefs that should be provided:
    typedef typename RW::_Ptr               _Ptr;
    typedef typename RW::_constPtr          _constPtr;
    typedef typename _Ptr::element_type      _Ptr_element_type;
    typedef typename _constPtr::element_type _constPtr_element_type;
    // initial NULL
    RW ptr;
    T_NULL;
    T_UNIQUE;
    T_EQ(ptr,ptr);
    // assign
    ptr = RW( new _Ptr_element_type );
    T_NOT_NULL;
    T_UNIQUE;
    T_EQ(ptr,ptr);
    {
      // share
      RW ptr2( ptr );
      T_NOT_NULL;
      T_NOT_UNIQUE;
      T_EQ(ptr,ptr2);
      // unshare
      ptr2.reset();
      T_NOT_NULL;
      T_UNIQUE;
      T_NE(ptr,ptr2);
      // different impl
      ptr2.reset( new _Ptr_element_type );
      T_NE(ptr,ptr2);
   }
    // assign
    ptr.reset( 0 );
    T_NULL;
    T_UNIQUE;
    // nullptr compatible
    ptr.reset( nullptr );
    T_NULL;
    T_UNIQUE;
    ptr = nullptr;
    T_NULL;
    T_UNIQUE;
    ptr = RW( nullptr );
    T_NULL;
    T_UNIQUE;


  }

template<class RW>
  void cowt()
  {
    test<RW>();
    MIL << __PRETTY_FUNCTION__ << std::endl;
    typedef typename RW::_Ptr::element_type _Ptr_element_type;
    // create
    RW ptr( new _Ptr_element_type );
    unsigned long ptrid = ptr->numericId();
    // share
    RW ptr2( ptr );
    // clone aon access
    unsigned long ptrid2 = ptr2->numericId();
    assert( ptrid != ptrid2 );
  }

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
BOOST_AUTO_TEST_CASE(basic_test)
{
  MIL << "===[START]=====" << endl;
  test<RW_pointer<NonIntrusive,          rw_pointer::Shared<NonIntrusive> > >();
  test<RW_pointer<const NonIntrusive,    rw_pointer::Shared<NonIntrusive> > >();
  test<RW_pointer<Intrusive,             rw_pointer::Intrusive<Intrusive> > >();
  test<RW_pointer<const Intrusive,       rw_pointer::Intrusive<Intrusive> > >();

  cowt<RWCOW_pointer<NonIntrusive,       rw_pointer::Shared<NonIntrusive> > >();
  cowt<RWCOW_pointer<const NonIntrusive, rw_pointer::Shared<NonIntrusive> > >();
  cowt<RWCOW_pointer<Intrusive,          rw_pointer::Intrusive<Intrusive> > >();
  cowt<RWCOW_pointer<const Intrusive,    rw_pointer::Intrusive<Intrusive> > >();

  MIL << "===[DONE]=====" << endl;
}
