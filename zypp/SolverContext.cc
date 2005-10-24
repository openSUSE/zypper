/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SolverContext.cc
 *
*/
#include <iostream>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/SolverContext.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SolverContextImpl
    //
    /** SolverContext implementation */
    struct SolverContextImpl : public base::ReferenceCounted, private base::NonCopyable
    {
      /** Default ctor*/
      SolverContextImpl();
      /** Dtor */
      ~SolverContextImpl();
    };
    ///////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(SolverContextImpl)

    /** \relates SolverContextImpl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const SolverContextImpl & obj )
    {
      return str << "SolverContextImpl";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SolverContext
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SolverContext::SolverContext
  //	METHOD TYPE : Ctor
  //
  SolverContext::SolverContext()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SolverContext::SolverContext
  //	METHOD TYPE : Ctor
  //
  SolverContext::SolverContext( detail::SolverContextImplPtr impl_r )
  : _pimpl( impl_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SolverContext::~SolverContext
  //	METHOD TYPE : Dtor
  //
  SolverContext::~SolverContext()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SolverContext::sayFriend
  //	METHOD TYPE : detail::constSolverContextImplPtr
  //
  detail::constSolverContextImplPtr SolverContext::sayFriend() const
  { return _pimpl; }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const SolverContext & obj )
  {
    return str << *obj.sayFriend();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
