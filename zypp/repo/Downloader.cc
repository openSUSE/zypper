/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/base/Gettext.h>

#include "Downloader.h"
#include <zypp/KeyContext.h>
#include <zypp/ZConfig.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ZYppCallbacks.h>

#include <zypp/parser/yum/RepomdFileReader.h>

using std::endl;

namespace zypp
{
namespace repo
{
  class ExtraSignatureFileChecker : public SignatureFileChecker
  {
  public:
    /** Called after download but before verifying the file. */
    typedef function<void( const Pathname & file_r )> PreCheckCB;

    ExtraSignatureFileChecker()
    {}

    ExtraSignatureFileChecker( Pathname signature_r )
    : SignatureFileChecker( std::move(signature_r) )
    {}

    void preCheckCB( PreCheckCB cb_r )
    { _preCheckCB = std::move(cb_r); }

    void operator()( const Pathname & file_r ) const
    {
      if ( _preCheckCB )
        _preCheckCB( file_r );
      SignatureFileChecker::operator()( file_r );
    }

  private:
    PreCheckCB _preCheckCB;
  };

  // bsc#1184326: Check and handle extra gpg keys delivered with trusted signed master index.
  void checkExtraKeysInRepomd( MediaSetAccess & media_r, const Pathname & destdir_r, const Pathname & repomd_r, SignatureFileChecker & sigchecker_r )
  {
    std::vector<std::pair<std::string,std::string>> keyhints { zypp::parser::yum::RepomdFileReader(repomd_r).keyhints() };
    if ( keyhints.empty() )
      return;
    DBG << "Check keyhints: " << keyhints.size() << endl;

    auto keyRing { getZYpp()->keyRing() };
    for ( const auto & p : keyhints ) try {
      const std::string & file  { p.first };
      const std::string & keyid { p.second };

      if ( keyRing->trustedPublicKeyData( keyid ) ) {
        DBG << "Keyhint is already trusted: " << keyid << " (" << file << ")" << endl;
        continue;	// already a trusted key
      }

      DBG << "Keyhint search key " << keyid << " (" << file << ")" << endl;
      PublicKeyData keyData = keyRing->publicKeyData( keyid );
      if ( not keyData ) {
        // try to get it from cache or download it...

        // TODO: Enhance the key caching in general...
        const ZConfig & conf = ZConfig::instance();
        Pathname cacheFile = conf.repoManagerRoot() / conf.pubkeyCachePath() / file;

        PublicKey key { PublicKey::noThrow( cacheFile ) };
        if ( not key.fileProvidesKey( keyid ) ) {

          key = PublicKey::noThrow( media_r.provideOptionalFile( file ) );
          if ( not key.fileProvidesKey( keyid ) ) {

            WAR << "Keyhint " << file << " does not contain a key with id " << keyid << ". Skipping it." << endl;
            continue;
          }
          // Try to cache it...
          filesystem::hardlinkCopy( key.path(), cacheFile );
        }

        keyRing->importKey( key, false );		// store in general keyring (not trusted!)
        keyData = keyRing->publicKeyData( keyid );	// fetch back from keyring in case it was a hidden key
      }

      if ( not PublicKey::isSafeKeyId( keyid ) ) {
        WAR << "Keyhint " << keyid << " for " << keyData << " is not strong enough for auto import. Just caching it." << endl;
        continue;
      }

      DBG << "Keyhint remember buddy " << keyData << endl;
      sigchecker_r.addBuddyKey( keyid );
    }
    catch ( const Exception & exp )
    { ZYPP_CAUGHT(exp); }
    catch ( const std::exception & exp )
    { ZYPP_CAUGHT(exp); }
    catch (...)
    { INT << "Oops!" << endl; }
    MIL << "Check keyhints done. Buddy keys: " << sigchecker_r.buddyKeys().size() << endl;
  }


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
  // always download them, even if repoGpgCheck is disabled
  Pathname sigpath = masterIndex_r.extend( ".asc" );
  Pathname keypath = masterIndex_r.extend( ".key" );

  //enable precache for next start() call
  setMediaSetAccess( media_r );
  enqueue( OnMediaLocation( sigpath, 1 ).setOptional( true ).setDownloadSize( ByteCount( 20, ByteCount::MB ) ) );
  enqueue( OnMediaLocation( keypath, 1 ).setOptional( true ).setDownloadSize( ByteCount( 20, ByteCount::MB ) ) );
  start( destdir_r );
  reset();

  // The local files are in destdir_r, if they were present on the server
  Pathname sigpathLocal { destdir_r/sigpath };
  Pathname keypathLocal { destdir_r/keypath };


  CompositeFileChecker checkers;

  if ( _pluginRepoverification && _pluginRepoverification->isNeeded() )
    checkers.add( _pluginRepoverification->getChecker( sigpathLocal, keypathLocal, repoInfo() ) );

  ExtraSignatureFileChecker sigchecker;
  bool isSigned = PathInfo(sigpathLocal).isExist();
  if ( repoInfo().repoGpgCheck() )
  {
    if ( isSigned || repoInfo().repoGpgCheckIsMandatory() )
    {
      // only add the signature if it exists
      if ( isSigned )
        sigchecker.signature( sigpathLocal );

      // only add the key if it exists
      if ( PathInfo(keypathLocal).isExist() )
        sigchecker.addPublicKey( keypathLocal );

      // set the checker context even if the key is not known
      // (unsigned repo, key file missing; bnc #495977)
      sigchecker.keyContext( repoInfo() );

      // bsc#1184326: Check and handle extra gpg keys delivered with trusted signed master index.
      if ( masterIndex_r.basename() == "repomd.xml" ) {
        sigchecker.preCheckCB( [&]( const Pathname & file_r )->void {
          // Take care no exception escapes! Main job is the signature verification.
          try {
            checkExtraKeysInRepomd( media_r, destdir_r, file_r, sigchecker );
          }
          catch ( const Exception & exp )
          { ZYPP_CAUGHT(exp); }
          catch ( const std::exception & exp )
          { ZYPP_CAUGHT(exp); }
          catch (...)
          { INT << "Oops!" << endl; }
        });
      }
      checkers.add( std::ref(sigchecker) );	// ref() to the local sigchecker is important as we want back fileValidated!
    }
    else
    {
      WAR << "Accept unsigned repository because repoGpgCheck is not mandatory for " << repoInfo().alias() << endl;
    }
  }
  else
  {
    WAR << "Signature checking disabled in config of repository " << repoInfo().alias() << endl;
  }

  enqueue( OnMediaLocation( masterIndex_r, 1 ).setDownloadSize( ByteCount( 20, ByteCount::MB ) ), checkers );
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
