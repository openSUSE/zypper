/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoManager.h
 *
*/
#ifndef ZYPP_REPOMANAGER_H
#define ZYPP_REPOMANAGER_H

#include <iosfwd>
#include <list>

#include "zypp/base/PtrTypes.h"
//#include "zypp/base/ReferenceCounted.h"
//#include "zypp/base/NonCopyable.h"
#include "zypp/Pathname.h"
#include "zypp/ZConfig.h"

#include "zypp2/RepoInfo.h"
#include "zypp2/repo/RepoException.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /**
   * Repo manager settings.
   * Settings default to ZYpp global settings.
   */
  struct RepoManagerOptions
  {
    RepoManagerOptions();
    
    Pathname repoCachePath;
    Pathname repoRawCachePath;
    Pathname knownReposPath;
  };

  /**
   * \short creates and provides information about known sources.
   *
   */
  class RepoManager
  {
    friend std::ostream & operator<<( std::ostream & str, const RepoManager & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
   RepoManager( const RepoManagerOptions &options = RepoManagerOptions() );
   
   /**
    * \short List known repositories.
    *
    * The known repositories are read from
    * \ref RepoManagerOptions::knownReposPath passed on the Ctor.
    * Which defaults to ZYpp global settings.
    *
    */
   std::list<RepoInfo> knownRepositories();
   
    /** Dtor */
    ~RepoManager();

  public:

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoManager Stream output */
  std::ostream & operator<<( std::ostream & str, const RepoManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp2
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_REPOMANAGER_H
