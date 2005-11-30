/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/solver/Context.h
 *
*/
#ifndef ZYPP_SOLVER_CONTEXT_H
#define ZYPP_SOLVER_CONTEXT_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/SolverFwd.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Context
    //
    /** Solver context.
    */
    class Context : public base::ReferenceCounted, private base::NonCopyable
    {
    public:
      typedef Context            Self;
      typedef Context_Ptr        Ptr;
      typedef Context_constPtr   constPtr;
      friend std::ostream & operator<<( std::ostream & str, const Context & obj );

    public:
      /** Default ctor */
      Context();
      /** Dtor */
      ~Context();
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Context Stream output */
    extern std::ostream & operator<<( std::ostream & str, const Context & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace solver
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOLVER_CONTEXT_H
