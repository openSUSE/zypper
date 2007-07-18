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
  //
  //	CLASS NAME : RepoStatus
  //
  /**
   * \short Local facts about a repository
   * This class represents the status of a
   * repository on the system.
   *
   * Anything that is not provided on the metadata
   * files, like the timestamp of the downloaded
   * metadata, and its checksum.
   */
  class RepoStatus
  {
    friend std::ostream & operator<<( std::ostream & str, const RepoStatus & obj );

  public:
    
    /**
     * Checksum of the repository.
     * Usually the checksum of the index, but any
     * checksum that changes when the repository changes
     * in any way is sufficient.
     */
    std::string checksum() const;
    
    /**
     * timestamp of the repository. If the repository
     * changes, it has to be updated as well with the
     * new timestamp.
     */
    Date timestamp() const;
    
    /**
     * \short Is the status empty?
     *
     * An empty status means that the status
     * was not calculated.
     */
    bool empty() const;

    /**
     * set the repository checksum \see checksum
     * \param checksum
     */
    RepoStatus & setChecksum( const std::string &checksum );
    
    /**
     * set the repository timestamp \see timestamp
     * \param timestamp
     */
    RepoStatus & setTimestamp( const Date &timestamp );

    /** Implementation  */
    class Impl;

  public:
    /** Default ctor */
    RepoStatus();
    
    /**
     * \short Status from a single file
     * As most repository state is represented
     * by the status of the index file, you can
     * construct the status from a file.
     *
     * \note construct from a non existing
     * file will result in an empty status
     */
    RepoStatus( const Pathname &file );
    
    /** Dtor */
    ~RepoStatus();

  public:

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoStatus Stream output */
  std::ostream & operator<<( std::ostream & str, const RepoStatus & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_REPOSTATUS_H
