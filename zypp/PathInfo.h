/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/PathInfo.h
 *
*/
#ifndef ZYPP_PATHINFO_H
#define ZYPP_PATHINFO_H

#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/Pathname.h>
#include <zypp-core/base/Function.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class StrMatcher;

  ///////////////////////////////////////////////////////////////////
  /** Types and functions for filesystem operations.
   * \todo move zypp::filesystem stuff into separate header
   * \todo Add tmpfile and tmpdir handling.
   * \todo think about using Exceptions in zypp::filesystem
   * \todo provide a readdir iterator; at least provide an interface
   * using an insert_iterator to be independent from std::container.
  */
  namespace filesystem
  {

    /**
     * Convenience returning <tt>StrMatcher( "[^.]*", Match::GLOB )</tt>
     * \see \ref dirForEach
     */
    const StrMatcher & matchNoDots();

    /**
     * \overload taking a \ref StrMatcher to filter the entries for which \a fnc_r is invoked.
     *
     * For convenience a \ref StrMatcher \ref matchNoDots is provided in this namespace.</tt>
     *
     * \code
     *   bool cbfnc( const Pathname & dir_r, const char *const str_r )
     *   {
     *     D BG <*< " - " << dir_r/str_r << endl;
     *     return true;
     *   }
     *   // Print no-dot files in "/tmp" via callback
     *   filesystem::dirForEach( "/tmp", filesystem::matchNoDots(), cbfnc );
     *
     *   // same via lambda
     *   filesystem::dirForEach( "/tmp", filesystem::matchNoDots(),
     *                           [](const Pathname & dir_r, const std::string & str_r)->bool
     *                           {
     *                             DBG << " - " << dir_r/str_r << endl;
     *                             return true;
     *                           });
     * \endcode
     */
    int dirForEach( const Pathname & dir_r, const StrMatcher & matcher_r, function<bool(const Pathname &, const char *const)> fnc_r );

    /////////////////////////////////////////////////////////////////
  } // namespace filesystem
  ///////////////////////////////////////////////////////////////////

  /** Dragged into namespace zypp. */
  using filesystem::PathInfo;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATHINFO_H
