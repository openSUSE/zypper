#include <ctime>

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <vector>

#include "Printing.h"

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

#include <zypp/CapFactory.h>

#include <zypp/ResFilters.h>
#include <zypp/ResStatus.h>
#include <zypp/ResPoolManager.h>

#include <zypp/ZYppFactory.h>
#include <zypp/Callback.h>

using namespace std;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::resfilter;

///////////////////////////////////////////////////////////////////
namespace zypp
{
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : NumImpl
    //
    /** Num implementation. */
    struct NumImpl
    {
      NumImpl()
      : _i( -1 )
      { SEC << "NumImpl(" << _i << ")" << std::endl; }

      NumImpl( int i_r )
      : _i( i_r )
      { INT << "NumImpl(" << _i << ")" << std::endl; }

      ~NumImpl()
      { ERR << "---NumImpl(" << _i << ")" << std::endl; }

      int i() const { return _i; }

      int _i;

    public:
      /** Offer default Impl. */
      static shared_ptr<NumImpl> nullimpl()
      {
        static shared_ptr<NumImpl> _nullimpl( new NumImpl );
        return _nullimpl;
      }
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Num::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const NumImpl & obj )
    {
      return str << "Num(" << obj._i << ")";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Num
    //
    /** */
    class Num
    {
      friend std::ostream & operator<<( std::ostream & str, const Num & obj );

    public:
      /** Implementation  */
      typedef NumImpl Impl;

    public:
      /** Default ctor */
      Num()
      : _pimpl( Impl::nullimpl() )
      {}
      /** Dtor */
      ~Num()
      {}

    public:
      Num( const shared_ptr<Impl> & pimpl_r )
      : _pimpl( pimpl_r ? pimpl_r : Impl::nullimpl() )
      {}

      int i() const { return _pimpl->i(); }

    private:
      /** Pointer to implementation */
      RWCOW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Num Stream output */
    std::ostream & operator<<( std::ostream & str, const Num & obj )
    { return str << *obj._pimpl; }

    ///////////////////////////////////////////////////////////////////


    struct NumPool
    {
      NumPool()
      : _pool(10)
      {}

      Num get( int i )
      {
        Num r( _get( i ) );
        MIL << i << " -> " << r << std::endl;
        return r;
      }
      Num _get( int i )
      {
        if ( i < 0 || i >= 10 )
          return Num();
        if ( ! _pool[i] )
          _pool[i] = shared_ptr<NumImpl>( new NumImpl( i ) );
        return _pool[i];
      }

      vector<shared_ptr<NumImpl> > _pool;
    };

    std::ostream & operator<<( std::ostream & str, const NumPool & obj )
    {
      pprint( obj._pool );
      return str;
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

  NumPool a;
  DBG << a << endl;
  DBG << a.get( -2 ) << endl;
  DBG << a.get( 3 ) << endl;
  DBG << a.get( 13 ) << endl;
  DBG << a << endl;


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

