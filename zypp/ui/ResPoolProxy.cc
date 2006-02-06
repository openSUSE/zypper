/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/ResPoolProxy.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/ui/ResPoolProxy.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ResPoolProxy::Impl
    //
    /** ResPoolProxy implementation. */
    struct ResPoolProxy::Impl
    {
    public:
      Impl()
      {}

      Impl( ResPool_Ref pool_r )
      : _pool( pool_r )
      {
#if 0
        ui::PP collect;
        for_each( query.begin(), query.end(),
                  functorRef<void,ResPool::Item>( collect ) );
        collect.dumpOn();
#endif

      }



    private:
      ResPool_Ref _pool;

    public:
      /** Offer default Impl. */
      static shared_ptr<Impl> nullimpl()
      {
        static shared_ptr<Impl> _nullimpl( new Impl );
        return _nullimpl;
      }
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ResPoolProxy::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const ResPoolProxy::Impl & obj )
    {
      return str << "ResPoolProxy::Impl";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ResPoolProxy
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ResPoolProxy::ResPoolProxy
    //	METHOD TYPE : Ctor
    //
    ResPoolProxy::ResPoolProxy()
    : _pimpl( Impl::nullimpl() )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ResPoolProxy::ResPoolProxy
    //	METHOD TYPE : Ctor
    //
    ResPoolProxy::ResPoolProxy( ResPool_Ref pool_r )
    : _pimpl( new Impl( pool_r ) )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ResPoolProxy::~ResPoolProxy
    //	METHOD TYPE : Dtor
    //
    ResPoolProxy::~ResPoolProxy()
    {}

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj )
    {
      return str << *obj._pimpl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
