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

  // always download them, even if repoGpgCheck is disabled
  enqueue( OnMediaLocation( sigpath, 1 ).setOptional( true ) );
  enqueue( OnMediaLocation( keypath, 1 ).setOptional( true ) );
  start( destdir_r, media_r );
  reset();

  FileChecker checker;	// set to sigchecker if appropriate, else Null.
  SignatureFileChecker sigchecker;
  bool isSigned = PathInfo(destdir_r / sigpath).isExist();

  if ( repoInfo().repoGpgCheck() )
  {
    // only add the signature if it exists
    if ( isSigned )
      sigchecker = SignatureFileChecker( destdir_r / sigpath );

    KeyContext context;
    context.setRepoInfo( repoInfo() );
    // only add the key if it exists
    if ( PathInfo(destdir_r / keypath).isExist() )
      sigchecker.addPublicKey( destdir_r / keypath, context );
    else
      // set the checker context even if the key is not known (unsigned repo, key
      // file missing; bnc #495977)
      sigchecker.setKeyContext( context );

    checker = FileChecker( ref(sigchecker) );	// ref() to the local sigchecker is important as we want back fileValidated!
  }
  else
  {
    WAR << "Signature checking disabled in config of repository " << repoInfo().alias() << endl;
  }

  enqueue( OnMediaLocation( masterIndex_r, 1 ), checker ? checker : FileChecker(NullFileChecker()) );
  start( destdir_r, media_r );
  reset();

  // Accepted!
  _repoinfo.setMetadataPath( destdir_r );
  if ( isSigned )
    _repoinfo.setValidRepoSignature( sigchecker.fileValidated() );
  else
    _repoinfo.setValidRepoSignature( indeterminate );
}


}// ns repo
} // ns zypp



