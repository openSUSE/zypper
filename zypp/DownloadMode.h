/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/DownloadMode.h
 *
*/
#ifndef ZYPP_DOWNLOADMODE_H
#define ZYPP_DOWNLOADMODE_H

#include <iosfwd>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** Supported commit download policies. */
  enum DownloadMode
  {
    DownloadDefault, //!< libzypp will decide what to do.
    DownloadOnly,	//!< Just download all packages to the local cache.
			//!< Do not install. Implies a dry-run.
    DownloadInAdvance,	//!< First download all packages to the local cache.
			//!< Then start to install.
    DownloadInHeaps,	//!< Similar to DownloadInAdvance, but try to split
			//!< the transaction into heaps, where at the end of
			//!< each heap a consistent system state is reached.
    DownloadAsNeeded	//!< Alternating download and install. Packages are
			//!< cached just to avid CD/DVD hopping. This is the
			//!< traditional behaviour.
  };

  /** \relates DownloadMode Parse from string.
   * On success the \ref DownloadMode is returned via \a result_r,
   * and the function returns \c true. Otherwise it returns \c false
   * and \a result_r remains unchanged.
   */
  bool deserialize( const std::string & str_r, DownloadMode & result_r );

  /** \relates DownloadMode Parse from string.
   * Similar as \ref deserialize, but silently return \ref DownloadDefault
   * in case of a parse error.
   */
  inline DownloadMode deserializeDownloadMode( const std::string & str_r )
  {
    DownloadMode ret( DownloadDefault );
    deserialize( str_r, ret );
    return ret;
  }

  /** \relates DownloadMode Stream output. */
  std::ostream & operator<<( std::ostream & str, DownloadMode obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DOWNLOADMODE_H
