/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SolverContext.h
 *
*/
#ifndef ZYPP_SOLVERCONTEXT_H
#define ZYPP_SOLVERCONTEXT_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(SolverContextImpl)
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SolverContext
  //
  /** */
  class SolverContext
  {
  public:
    /** Default ctor */
    SolverContext();
    /** Factory ctor */
    explicit
    SolverContext( detail::SolverContextImplPtr impl_r );
    /** Dtor */
    ~SolverContext();

  private:
    /** Pointer to implementation */
    detail::SolverContextImplPtr _pimpl;
  public:
    /** Avoid a bunch of friend decl. */
    detail::constSolverContextImplPtr sayFriend() const;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SolverContext Stream output */
  extern std::ostream & operator<<( std::ostream & str, const SolverContext & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOLVERCONTEXT_H
