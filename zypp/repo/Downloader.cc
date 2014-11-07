/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"

#include "Downloader.h"
#include "zypp/KeyContext.h"
#include "zypp/ZYppCallbacks.h"

using namespace std;

namespace zypp
{
namespace repo
{

Downloader::Downloader()
{
}
Downloader::Downloader(const RepoInfo & repoinfo) : _repoinfo(repoinfo)
{
}
Downloader::~Downloader()
{
}

RepoStatus Downloader::status( MediaSetAccess &media )
{
  WAR << "Non implemented" << endl;
  return RepoStatus();
}

void Downloader::download( MediaSetAccess &media,
                           const Pathname &dest_dir,
                           const ProgressData::ReceiverFnc & progress )
{
  WAR << "Non implemented" << endl;
}

void Downloader::defaultDownloadMasterIndex( MediaSetAccess & media_r, const Pathname & destdir_r, const Pathname & masterIndex_r )
{
  Pathname sigpath = masterIndex_r.extend( ".asc" );
  Pathname keypath = masterIndex_r.extend( ".key" );

  SignatureFileChecker sigchecker;

  enqueue( OnMediaLocation( sigpath, 1 ).setOptional( true ) );
  start( destdir_r, media_r );
  reset();

  // only add the signature if it exists
  if ( PathInfo(destdir_r / sigpath).isExist() )
    sigchecker = SignatureFileChecker( destdir_r / sigpath );

  enqueue( OnMediaLocation( keypath, 1 ).setOptional( true ) );
  start( destdir_r, media_r );
  reset();

  KeyContext context;
  context.setRepoInfo( repoInfo() );
  // only add the key if it exists
  if ( PathInfo(destdir_r / keypath).isExist() )
    sigchecker.addPublicKey( destdir_r / keypath, context );
  else
    // set the checker context even if the key is not known (unsigned repo, key
    // file missing; bnc #495977)
    sigchecker.setKeyContext( context );

  if ( ! repoInfo().gpgCheck() )
  {
    WAR << "Signature checking disabled in config of repository " << repoInfo().alias() << endl;
  }
  enqueue( OnMediaLocation( masterIndex_r, 1 ),
	   repoInfo().gpgCheck() ? FileChecker(sigchecker) : FileChecker(NullFileChecker()) );
  start( destdir_r, media_r );
  reset();
}


}// ns repo
} // ns zypp



