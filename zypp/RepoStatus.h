/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoStatus.h
 *
*/
#ifndef ZYPP2_REPOSTATUS_H
#define ZYPP2_REPOSTATUS_H

#include <iosfwd>
#include "zypp/base/PtrTypes.h"
#include "zypp/CheckSum.h"
#include "zypp/Date.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class RepoStatus
  /// \brief Track changing files or directories.
  ///
  /// Compute timestamp and checksum for individual files or
  /// directories (recursively) to track changing content.
  ///
  /// The timestamp most probably denotes the time the data were
  /// changed the last time, that's why it is exposed.
  ///
  /// The checksum however is an implementation detail and of no
  /// use outside this class. \ref operator== tells if the checksums
  /// of two rRepoStatus are the same.
  ///////////////////////////////////////////////////////////////////
  class RepoStatus
  {
    friend std::ostream & operator<<( std::ostream & str, const RepoStatus & obj );
    friend RepoStatus operator&&( const RepoStatus & lhs, const RepoStatus & rhs );
    friend bool operator==( const RepoStatus & lhs, const RepoStatus & rhs );

  public:
    /** Default ctor */
    RepoStatus();

    /** Compute status for single file or directory (recursively)
     *
     * \note Construction from a non existing file will result
     * in an empty status.
     */
    explicit RepoStatus( const Pathname & path_r );

    /** Dtor */
    ~RepoStatus();

  public:
    /** Reads the status from a cookie file
     * \returns An empty \ref RepoStatus if the file does not
     * exist or is not readable.
     * \see \ref saveToCookieFile
     */
    static RepoStatus fromCookieFile( const Pathname & path );

    /** Save the status information to a cookie file
     * \throws Exception if the file can't be saved
     * \see \ref fromCookieFile
     */
    void saveToCookieFile( const Pathname & path_r ) const;

  public:
    /** Whether the status is empty (default constucted) */
    bool empty() const;

    /** The time the data were changed the last time */
    Date timestamp() const;

  public:
    class Impl;			///< Implementation
  private:
    RWCOW_pointer<Impl> _pimpl;	///< Pointer to implementation
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoStatus Stream output */
  std::ostream & operator<<( std::ostream & str, const RepoStatus & obj );

  /** \relates RepoStatus Combine two RepoStatus (combined checksum and newest timestamp) */
  RepoStatus operator&&( const RepoStatus & lhs, const RepoStatus & rhs );

  /** \relates RepoStatus Whether 2 RepoStatus refer to the same content checksum */
  bool operator==( const RepoStatus & lhs, const RepoStatus & rhs );

  /** \relates RepoStatus Whether 2 RepoStatus refer to different content checksums */
  inline bool operator!=( const RepoStatus & lhs, const RepoStatus & rhs )
  { return ! ( lhs == rhs ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_REPOSTATUS_H
