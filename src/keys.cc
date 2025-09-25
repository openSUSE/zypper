/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "Zypper.h"
#include "utils/prompt.h"
#include "utils/misc.h"
#include "utils/text.h"
#include "utils/console.h"

#include <zypp/ZYpp.h>
#include <zypp/KeyManager.h>
#include <zypp/MediaSetAccess.h>

#include <sstream>

extern ZYpp::Ptr God;

namespace  {
/**
 * Match a string argument against the key with the following approach
 * - Does the key ID start or end with the string
 * - Is the string part of the keys name
 * - Does the subkey IDs start or end with the string
 *
 * \return true if the key matches
 */
bool matchKeyToArg ( const PublicKeyData &key, const std::string &arg  )
  {
    auto idMatches = [] ( const std::string &id, const std::string &search ) {
      return str::startsWithCI( id, search ) || str::endsWithCI( id, search );
    };

    if ( idMatches( key.id(), arg ) )
      return true;
    else if ( str::containsCI( key.name(), arg ) )
      return true;
    else {
      for ( const auto &sKey : key.subkeys() )  {
        if ( idMatches( sKey.id(), arg ) )
          return true;
      }
    }
    return false;
  }
}

template <class T>
void dumpKeyList ( Zypper &zypper, const std::list<T> &keysFound_r, bool details )
{
  if ( keysFound_r.empty() )
  {
    zypper.out().warning(_("No keys found.") );
  }
  else
  {
    zypper.out().gap();
    if ( details || zypper.config().machine_readable )
    {
      for(const auto &key : keysFound_r)
      {
        if (zypper.config().machine_readable)
          dumpKeyInfo( std::cout , key );
        else
        {

          // render all keys to the console in a nice way:
          // +-------------------+
          // |         .^l.. .   |  Key ID:           A193FBB572174FC2
          // |          ^ l.: .  |  Key Name:         Virtualization OBS Project <Virtualization@build.opensuse.org>
          // |         ^ :.i ^   |  Key Fingerprint:  55A0B34D 49501BB7 CA474F5A A193FBB5 72174FC2
          // |          i.^ ?.   |  Key Created:      Sat 23 Dec 2017 05:45:11 AM CET
          // |         ..: : .E^.|  Key Expires:      Mon 02 Mar 2020 05:45:11 AM CET
          // |         S  . . .^:|  Rpm Name:         gpg-pubkey-72174fc2-5a3ddf57
          // |               ^ ^^|
          // |                ^ .|
          // |                   |
          // |                   |
          // |                   |
          // +----[72174FC2]-----+

          size_t artWidth = 0;

          PublicKey::AsciiArt::Options asciiArtOptions;
          if (zypper.config().do_colors )
            asciiArtOptions |= PublicKey::AsciiArt::USE_COLOR;

          // render the ascii art into a buffer
          std::vector<std::string> art(
                key.asciiArt().asLines( asciiArtOptions )
          );
          if ( art.size() )
          {
            artWidth = art[0].length();
          }

          std::string keyInfo;
          {
            std::stringstream infoStr;
            dumpKeyInfo ( infoStr, key );
            keyInfo = infoStr.str();
          }

          // render the key info into a buffer, but intend it with the width the ascii Art takes up
          // MbsWriteWrapped makes sure the strings are wrapped to fit the console
          std::vector<std::string> formattedInfo;
          {
            std::stringstream infoStr;
            mbs::MbsWriteWrapped writer( infoStr, artWidth, get_screen_width() );
            writer.addString( keyInfo );
            str::split( infoStr.str(), std::back_inserter(formattedInfo), "\n" );
          }

          // bring both sides together and write them to the output,
          // write not data to the first nr of lines in noDataOnFirstLinesCnt
          const size_t noDataOnFirstLinesCnt = art.size() > 0 ? 1 : 0;
          for ( size_t i = 0; i < std::max(art.size(), formattedInfo.size() + noDataOnFirstLinesCnt); i++ )
          {
            bool renderArtLine = i < art.size();
            if ( renderArtLine )
            {
              std::cout << art[i];
            }

            size_t dataIdx = i - noDataOnFirstLinesCnt;
            if ( i >= noDataOnFirstLinesCnt && formattedInfo.size() > dataIdx )
            {
              std::cout << formattedInfo[dataIdx].substr( renderArtLine ? artWidth : 0 );
            }

            std::cout << std::endl;
          }
        }
        zypper.out().gap();
      }
    } else {
      Table t;
      t << ( TableHeader()
	  /* translators: Table column header */	<< _("ID")
	  /* translators: Table column header */	<< _("Name")
	  /* translators: Table column header */	<< _("Expires"));

      for( const auto &key : keysFound_r )
      {
        t << ( TableRow()
               << key.id()
               << key.name()
               << key.expires().asString());
      }
      t.dumpTo( std::cout );
      zypper.out().gap();
    }
  }
  zypper.setExitCode( ZYPPER_EXIT_OK );
}

void listTrustedKeys ( Zypper &zypper, const std::vector<std::string> &keysToList, bool details )
{
  if ( !God || !God->target() || !God->keyRing() )
    return;

  KeyRing_Ptr keyRing = God->keyRing();

  if ( keysToList.size() ) {
    std::list<PublicKeyData> pubKeys;
    for ( const std::string &arg : keysToList )
    {
      for ( const PublicKeyData &key : keyRing->trustedPublicKeyData() ) {
        if ( matchKeyToArg( key, arg ) ) {
          //directly matched to a key, use it directly
          pubKeys.push_back( key );
        } else {
          // is the current arg a path or a ID
          filesystem::PathInfo path (arg);
          if (!path.isExist())
          {
            zypper.out().warning( str::Format(_("Argument '%1%' is not a trusted ID nor a existing file.")) % arg );
            zypper.setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
            return;
          }

          KeyManagerCtx keyMgr = KeyManagerCtx::createForOpenPGP();
          pubKeys.splice( pubKeys.end(), keyMgr.readKeyFromFile( arg ));
        }
      }
    }
    //print always in detail mode here, the user most likely wants to know more about the key
    dumpKeyList( zypper, pubKeys, true );
  }
  else {
    dumpKeyList( zypper, keyRing->trustedPublicKeyData(), details );
  }
}

void removeKey ( Zypper &zypper, const std::string &searchStr , bool removeAllMatches )
{
  if ( !God || !God->keyRing() )
    return;

  KeyRing_Ptr keyRing = God->keyRing();

  std::list<PublicKeyData> foundKeys;
  for ( const PublicKeyData &key : keyRing->trustedPublicKeyData() )
  {
    if ( matchKeyToArg( key, searchStr ))
      foundKeys.push_back( key );
  }

  if ( !foundKeys.size() ) {
    zypper.out().warning( str::Format(_("Key %1% is not known")) % searchStr );
    zypper.setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
    return;
  }

  if ( foundKeys.size() > 1 && !removeAllMatches ) {
    zypper.out().warning( str::Format(_("Query %1% matches multiple keys, use the %2% argument to remove multiple matches.")) % searchStr % "--all-matches" );
    zypper.setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
    return;
  }

   auto delKey = [&]( const PublicKeyData &key, bool askBeforeDelete ) {

      if ( askBeforeDelete ) {
        std::stringstream keyInfo;
        dumpKeyInfo ( keyInfo, key );
        std::string question = str::Format(
          "About to delete key: \n"
          "%1%\n"
          "Are you sure?\n"
        ) % keyInfo.str();

        if ( ! read_bool_answer(PROMPT_YN_GPG_REMOVE_KEY_ACCEPT, question, false) ) {
          return;
        }
      }

      try
      {
        std::cout << "Deleting key " << key.id() << std::endl;
        keyRing->deleteKey( key.id(), true );
      } catch ( const Exception & e ) {
        ZYPP_CAUGHT( e );
        zypper.out().error( e, str::Format(_("Failed to delete key: %1%")) % searchStr );
        zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
        return;
      }
  };

  bool askBeforeDelete = true;

  if ( foundKeys.size() > 1 ) {
    std::stringstream keyInfo;
    dumpKeyList ( zypper, foundKeys, false );
    std::string question = _("Found multiple keys, do you want to delete them?\n");

    PromptOptions popts;
    popts.setOptions(_("y/a/c"), 2 /* default reply */);
    popts.setOptionHelp(0, _("Remove all keys 'y'"));
    popts.setOptionHelp(1, _("Ask for each key seperately 'a'"));
    popts.setOptionHelp(2, _("Cancel 'c'"));

    zypper.out().prompt( PROMPT_YAC_GPG_REMOVE_KEYS_ACCEPT, question, popts );
    switch ( get_prompt_reply( zypper, PROMPT_YAC_GPG_REMOVE_KEYS_ACCEPT, popts) ) {
      case 0: //Remove all
        askBeforeDelete = false;
        break;
      case 1: //Seperately
        askBeforeDelete = true;
        break;
      case 2: //Cancel
        return;
    }
  }

  for ( const PublicKeyData &key : foundKeys ) {
    delKey( key, askBeforeDelete );
  }
}

void importKey (Zypper &zypper, const std::string &url_r )
{
  if ( !God || !God->keyRing() )
    return;

  Url url = make_url( url_r );
  if ( !url.isValid() )
  {
    zypper.setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
    return;
  }

  //try to get a file from the URL
  ManagedFile local;
  try
  { local = MediaSetAccess::provideFileFromUrl(url); }
  catch ( const media::MediaException & e )
  {
    ZYPP_CAUGHT( e );
    zypper.out().error( e, str::Format(_("Problem accessing the file at the specified URI: %1%")) % url,
			_("Please check if the URI is valid and accessible.") );
    zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
    return;
  }
  catch ( const Exception & e )
  {
    ZYPP_CAUGHT( e );
    zypper.out().error( e, str::Format(_("Problem encountered while trying to read the file at the specified URI %1%")) % url );
    zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
    return;
  }

  PublicKey pKey;
  try
  {
    pKey = PublicKey(local->asString());
  } catch ( const Exception & e ) {
    ZYPP_CAUGHT( e );
    zypper.out().error( e, str::Format(_("Problem encountered while parsing the file at the specified URI %1%")) % url );
    zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
    return;
  }

  //first import all the keys in keyfile into the general keyring, we do not trust yet
  KeyRing_Ptr keyRing = God->keyRing();
  keyRing->importKey( pKey, false);

  //now lets go over all imported keys and ask explicitely if the user wants to trust them
  for ( const PublicKey &key : keyRing->publicKeys() )
  {
    std::stringstream keyInfo;
    dumpKeyInfo ( keyInfo, key );
    std::string question = str::Format(
          "About to import key: \n"
          "%1%\n"
          "Do you want to continue?\n"
    ) % keyInfo.str();

    if ( read_bool_answer(PROMPT_YN_GPG_UNKNOWN_KEY_ACCEPT, question, false) )
    {
      try
      {
        keyRing->importKey( keyRing->exportPublicKey(key.keyData()), true );
      } catch ( const Exception & e ) {
        ZYPP_CAUGHT( e );
        zypper.out().error( e, str::Format(_("Failed to import key from URL: %1%")) % url );
        zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
        return;
      }
    }
  }
}
