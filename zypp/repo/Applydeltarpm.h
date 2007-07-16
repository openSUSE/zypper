/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/Applydeltarpm.h
 *
*/
#ifndef ZYPP_SOURCE_APPLYDELTARPM_H
#define ZYPP_SOURCE_APPLYDELTARPM_H

#include <iosfwd>
#include <string>

#include "zypp/base/Function.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** Namespace wrapping invocations of /usr/bin/applydeltarpm. */
  ///////////////////////////////////////////////////////////////////
  namespace applydeltarpm
  { /////////////////////////////////////////////////////////////////

    /** Test whether an execuatble applydeltarpm program is available. */
    bool haveApplydeltarpm();

    /** \name Check if reconstruction of rpm is possible.
     * \see <tt>man applydeltarpm</tt>
    */
    //@{
    /** Check via sequence info.
     * \see <tt>applydeltarpm [-c|-C] -s sequence</tt>
    */
    bool check( const std::string & sequenceinfo_r, bool quick_r = false );

    /** Check via deltarpm.
     * \see <tt>applydeltarpm [-c|-C] deltarpm</tt>
    */
    bool check( const Pathname & delta_r, bool quick_r = false );

    /** Quick via check sequence info.*/
    inline bool quickcheck( const std::string & sequenceinfo_r )
    { return check( sequenceinfo_r, true ); }

    /** Quick check via deltarpm.*/
    inline bool quickcheck( const Pathname & delta_r )
    { return check( delta_r, true ); }
    //@}

    /** \name Re-create a new rpm from binary delta.
     * \see <tt>man applydeltarpm</tt>
    */
    //@{
    /** progress reporting */
    typedef function<void( unsigned )> Progress;

    /** Apply a binary delta to on-disk data to re-create a new rpm.
     * \see <tt>applydeltarpm deltarpm newrpm</tt>
    */
    bool provide( const Pathname & delta_r, const Pathname & new_r,
                  const Progress & report_r = Progress() );

    /** Apply a binary delta to an old rpm to re-create a new rpm.
     * \see <tt>applydeltarpm -r oldrpm deltarpm newrpm</tt>
    */
    bool provide( const Pathname & old_r, const Pathname & delta_r,
                  const Pathname & new_r,
                  const Progress & report_r = Progress() );
    //@}

    /////////////////////////////////////////////////////////////////
  } // namespace applydeltarpm
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_APPLYDELTARPM_H
