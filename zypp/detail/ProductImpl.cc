/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ProductImpl.cc
 *
*/

#include "zypp/detail/ProductImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    IMPL_PTR_TYPE(ProductImpl)

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProductImpl
    //
    ///////////////////////////////////////////////////////////////////

    /** Default ctor */
    ProductImpl::ProductImpl( const std::string & name_r,
			    const Edition & edition_r,
			    const Arch & arch_r )
    : ResolvableImpl( ResKind( "script"),
		      name_r,
		      edition_r,
		      arch_r )
    {
    }
    /** Dtor */
    ProductImpl::~ProductImpl()
    {
    }

    std::list<std::string> ProductImpl::description() const
    {
      return _description;
    }

    std::string ProductImpl::category() const {
      return _category;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
