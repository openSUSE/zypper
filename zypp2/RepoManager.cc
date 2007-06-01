/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoManager.cc
 *
*/

#include <iostream>
#include <list>
#include <algorithm>
#include "zypp/base/Exception.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/Logger.h"
#include "zypp/PathInfo.h"
#include "zypp/parser/IniDict.h"

#include "zypp2/RepoManager.h"

using namespace std;
using namespace zypp;
using namespace zypp::filesystem;
using parser::IniDict;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  RepoManagerOptions::RepoManagerOptions()
  {
    ZConfig globalConfig;
    repoCachePath = globalConfig.defaultRepoCachePath();
    repoRawCachePath = globalConfig.defaultRepoRawCachePath();
    knownReposPath = globalConfig.defaultKnownReposPath();
  }
    
  /**
   * \short List of RepoInfo's from a file.
   * \param file pathname of the file to read.
   */
  static std::list<RepoInfo> repositories_in_file( const Pathname &file )
  {
    InputStream is(file);
    IniDict dict(is);
    std::list<RepoInfo> repos;
    
    for ( IniDict::section_const_iterator its = dict.sectionsBegin();
          its != dict.sectionsEnd();
          ++its )
    {
      MIL << (*its) << endl;
      
      RepoInfo info;
      info.setAlias(*its);
                    
      for ( IniDict::entry_const_iterator it = dict.entriesBegin(*its);
            it != dict.entriesEnd(*its);
            ++it )
      {
        
        //MIL << (*it).first << endl;
        if (it->first == "name" )
          info.setName(it-> second);
        else if ( it->first == "enabled" )
          info.setEnabled( it->second == "1" );
        else if ( it->first == "baseurl" )
          info.addBaseUrl( Url(it->second) );
        else if ( it->first == "type" )
          info.setType(it->second);
      }
      
      // add it to the list.
      repos.push_back(info);
    }
    
    return repos;
  }
  
  /**
   * \short List of RepoInfo's from a directory
   *
   * Goes trough every file in a directory and adds all
   * RepoInfo's contained in that file.
   *
   * \param file pathname of the file to read.
   */
  static std::list<RepoInfo> repositories_in_path( const Pathname &dir )
  {
    std::list<RepoInfo> repos;
    list<Pathname> entries;
    if ( filesystem::readdir( entries, Pathname(dir), false ) != 0 )
      ZYPP_THROW(Exception("failed to read directory"));
    
    for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
    {
      Pathname file = *it;
      std::list<RepoInfo> repos_here = repositories_in_file(file);
      std::copy( repos_here.begin(), repos_here.end(), std::back_inserter(repos));
    }
    return repos;
  }
  
  std::list<RepoInfo> RepoManager::knownRepositories()
  {
    return repositories_in_path("/etc/zypp/repos.d");
  }

  /** RepoManager implementation. */
  struct RepoManager::Impl
  {
    Impl( const RepoManagerOptions &opt )
      : options(opt)
    {
    
    }
    
    Impl()
    {
    
    }
    
    RepoManagerOptions options;
    
  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoManager::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepoManager::Impl & obj )
  {
    return str << "RepoManager::Impl";
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoManager
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoManager::RepoManager
  //	METHOD TYPE : Ctor
  //
  RepoManager::RepoManager( const RepoManagerOptions &opt )
  : _pimpl( new Impl(opt) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoManager::~RepoManager
  //	METHOD TYPE : Dtor
  //
  RepoManager::~RepoManager()
  {}

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const RepoManager & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp2
///////////////////////////////////////////////////////////////////
