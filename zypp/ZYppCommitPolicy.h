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
    ZYppCommitPolicy()
    : _restrictToMedia    ( 0 )
    , _dryRun             ( false )
    , _rpmNoSignature     ( false )
    , _syncPoolAfterCommit( true )
    {}

  public:
    unsigned restrictToMedia() const
    { return _restrictToMedia; }

    bool dryRun() const
    { return _dryRun; }

    bool rpmNoSignature() const
    { return _rpmNoSignature; }

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

    /** Use rpm option --nosignature (default: false) */
    ZYppCommitPolicy & rpmNoSignature( bool yesNo_r )
    { _rpmNoSignature = yesNo_r; return *this; }

    /** Kepp pool in sync with the Target databases after commit (default: true) */
    ZYppCommitPolicy & syncPoolAfterCommit( bool yesNo_r )
    { _syncPoolAfterCommit = yesNo_r; return *this; }

  private:
    unsigned _restrictToMedia;
    bool     _dryRun;
    bool     _rpmNoSignature;
    bool     _syncPoolAfterCommit;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ZYppCommitPolicy Stream output. */
  std::ostream & operator<<( std::ostream & str, const ZYppCommitPolicy & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPPCOMMITPOLICY_H
