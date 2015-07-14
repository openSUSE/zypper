/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/solver/detail/Testcase.h
 *
*/

#ifndef ZYPP_SOLVER_DETAIL_TESTCASE_H
#define ZYPP_SOLVER_DETAIL_TESTCASE_H
#ifndef ZYPP_USE_RESOLVER_INTERNALS
#error Do not directly include this file!
#else

#include <string>

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      class Resolver;

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Testcase
      /**
       * Generating a testcase of the current pool and solver state.
       **/
      class Testcase
      {
	private:
	  std::string dumpPath; // Path of the generated testcase

	public:
	  Testcase();
	  Testcase( const std::string & path );
	  ~Testcase();

	  bool createTestcase( Resolver & resolver, bool dumpPool = true, bool runSolver = true );
      };

      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
#endif // ZYPP_USE_RESOLVER_INTERNALS
#endif // ZYPP_SOLVER_DETAIL_TESTCASE_H
