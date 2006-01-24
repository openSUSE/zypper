#include "zypp/ResPoolManager.h"
#include "zypp/ResFilters.h"

namespace zypp
{
  ///////////////////////////////////////////////////////////////////

  bool somefunction( ResObject::constPtr p, Capability c )
  {
    return true;
  }

  class SomeClass
  {
    bool somemember( ResObject::constPtr p, Capability c )
    {
      return true;
    }
  };

  struct SomeFunctor
  {
    bool operator()( ResObject::constPtr p, Capability c )
    {
      return true;
    }
  };

  ///////////////////////////////////////////////////////////////////

  
  
  ///////////////////////////////////////////////////////////////////

  template<class _RedirectToFunction>
    struct MyFunction : public functor::ResObjectFilterFunctor
    {
      bool operator()( ResObject::constPtr p ) const
      {
        Capability c;
        // c = ....
        return _fnc( p, c );
      }

      MyFunction( _RedirectToFunction fnc_r )
      : _fnc( fnc_r  )
      {}
      _RedirectToFunction _fnc;
    };

  template<class _RedirectToFunction>
    MyFunction<_RedirectToFunction> makeMyFunction( _RedirectToFunction r )
    { return MyFunction<_RedirectToFunction>( r ); }

  ///////////////////////////////////////////////////////////////////


  void test( const ResPool & pool_r )
  {
    int res = invokeOnEach( pool_r.byNameBegin("rpm"),
                            pool_r.byNameEnd("rpm"),
                            makeMyFunction(somefunction) );

    res = invokeOnEach( pool_r.byNameBegin("rpm"),
                        pool_r.byNameEnd("rpm"),
                        makeMyFunction(SomeFunctor()) );
    
    SomeClass c;
    
  }

}

int main()
{
  zypp::ResPoolManager pool;
  zypp::test( pool.accessor() );
  return 0;
}

