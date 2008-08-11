/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYppCommitPolicy.h
 *
*/
#ifndef ZYPP_ZYPPCOMMITPOLICY_H
#define ZYPP_ZYPPCOMMITPOLICY_H

#include <iosfwd>

#include "zypp/target/rpm/RpmFlags.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYppCommitPolicy
  //
  /** */
  class ZYppCommitPolicy
  {
  public:
    ZYppCommitPolicy();

  public:
    unsigned restrictToMedia() const
    { return _restrictToMedia; }

    bool dryRun() const
    { return _dryRun; }

    target::rpm::RpmInstFlags rpmInstFlags() const
    { return _rpmInstFlags; }

    bool rpmNoSignature() const
    { return _rpmInstFlags.testFlag( target::rpm::RPMINST_NOSIGNATURE ); }

    bool rpmExcludeDocs() const
    { return _rpmInstFlags.testFlag( target::rpm::RPMINST_EXCLUDEDOCS ); }


    bool syncPoolAfterCommit() const
    { return _syncPoolAfterCommit; }

  public:
    /** Restrict commit to media 1.
     * Fake outstanding YCP fix: Honour restriction to media 1
     * at installation, but install all remaining packages if
     * post-boot (called with <tt>mediaNr_r &gt; 1</tt>).
     */
    ZYppCommitPolicy & restrictToMedia( unsigned mediaNr_r )
    { _restrictToMedia = ( mediaNr_r == 1 ) ? 1 : 0; return *this; }

    /** Process all media (default) */
    ZYppCommitPolicy & allMedia()
    { return restrictToMedia( 0 ); }

    /** Set dry run (default: false) */
    ZYppCommitPolicy & dryRun( bool yesNo_r )
    { _dryRun = yesNo_r; return *this; }

    /** The default \ref target::rpm::RpmInstFlags. (default: none)*/
    ZYppCommitPolicy &  rpmInstFlags( target::rpm::RpmInstFlags newFlags_r )
    { _rpmInstFlags = newFlags_r; return *this; }

    /** Use rpm option --nosignature (default: false) */
    ZYppCommitPolicy & rpmNoSignature( bool yesNo_r )
    { _rpmInstFlags.setFlag( target::rpm::RPMINST_NOSIGNATURE, yesNo_r ); return *this; }

    /** Use rpm option --excludedocs (default: false) */
    ZYppCommitPolicy & rpmExcludeDocs( bool yesNo_r )
    { _rpmInstFlags.setFlag( target::rpm::RPMINST_EXCLUDEDOCS, yesNo_r ); return *this; }

    /** Kepp pool in sync with the Target databases after commit (default: true) */
    ZYppCommitPolicy & syncPoolAfterCommit( bool yesNo_r )
    { _syncPoolAfterCommit = yesNo_r; return *this; }

  private:
    unsigned                  _restrictToMedia;
    bool                      _dryRun;
    target::rpm::RpmInstFlags _rpmInstFlags;
    bool                      _syncPoolAfterCommit;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ZYppCommitPolicy Stream output. */
  std::ostream & operator<<( std::ostream & str, const ZYppCommitPolicy & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPPCOMMITPOLICY_H
