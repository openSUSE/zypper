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
    : _restrictToMedia( 0 )
    , _dryRun( false )
    {}

  public:
    unsigned restrictToMedia() const
    { return _restrictToMedia; }

    bool dryRun() const
    { return _dryRun; }

  public:
    /** Restrict commit to a certain media number
     * \deprecated
     */
    ZYppCommitPolicy & restrictToMedia( unsigned mediaNr_r )
    { _restrictToMedia = mediaNr_r; return *this; }

    /** Process all media (default) */
    ZYppCommitPolicy & allMedia()
    { return restrictToMedia( 0 ); }

    /** Set dry run (default: false) */
    ZYppCommitPolicy & dryRun( bool yesNo_r = true )
    { _dryRun = yesNo_r; return *this; }

  private:
    unsigned _restrictToMedia;
    bool     _dryRun;

  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ZYppCommitPolicy Stream output. */
  std::ostream & operator<<( std::ostream & str, const ZYppCommitPolicy & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPPCOMMITPOLICY_H
