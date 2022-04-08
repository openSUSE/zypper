/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZMART_KEYRINGCALLBACKS_H
#define ZMART_KEYRINGCALLBACKS_H

#include <stdlib.h>
#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>

#include "Zypper.h"
#include "Table.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  {
    inline void hintFingerprint()
    {
      Zypper::instance().out().notePar( 4, _("A GPG pubkey is clearly identified by its fingerprint. Do not rely on the key's name. If you are not sure whether the presented key is authentic, ask the repository provider or check their web site. Many providers maintain a web page showing the fingerprints of the GPG keys they are using.") );
    }

    inline void hintUnsignedData()
    {
      Zypper::instance().out().notePar( 4, _("Signing data enables the recipient to verify that no modifications occurred after the data were signed. Accepting data with no, wrong or unknown signature can lead to a corrupted system and in extreme cases even to a system compromise.") );
    }

    inline void hintIfMasterIndex( const std::string & file_r )
    {
      if ( file_r == "repomd.xml" || file_r == "content" )
      {
        // translator: %1% is a file name
        Zypper::instance().out().notePar( 4, str::Format(_("File '%1%' is the repositories master index file. It ensures the integrity of the whole repo.") ) % file_r );
      }
    }

    inline void warnCanNotVerifyFile()
    {
      Zypper::instance().out().warningPar( 4, str::Format(_("We can't verify that no one meddled with this file, so it might not be trustworthy anymore! You should not continue unless you know it's safe.") ) );
    }

    inline void warnFileModifiedAfterSigning()
    {
      Zypper::instance().out().warningPar( 4, str::Format(_("This file was modified after it has been signed. This may have been a malicious change, so it might not be trustworthy anymore! You should not continue unless you know it's safe.") ) );
    }

    std::ostream & dumpKeyInfo( std::ostream & str, const PublicKeyData & key, const KeyContext & context = KeyContext() )
    {
      Zypper & zypper = Zypper::instance();
      if ( zypper.out().type() == Out::TYPE_XML )
      {
        {
          xmlout::Node parent( str, "gpgkey-info", xmlout::Node::optionalContent );

          if ( !context.empty() )
          {
            dumpAsXmlOn( *parent, context.repoInfo().asUserString(), "repository" );
          }
          dumpAsXmlOn( *parent, key.name(), "key-name" );
          dumpAsXmlOn( *parent, key.fingerprint(), "key-fingerprint" );
          dumpAsXmlOn( *parent, key.algoName(), "key-algorithm" );
          dumpAsXmlOn( *parent, key.created(), "key-created" );
          dumpAsXmlOn( *parent, key.expires(), "key-expires" );
          dumpAsXmlOn( *parent, key.rpmName(), "rpm-name" );
        }
        return str;
      }

      Table t;
      t.lineStyle( none );
      if ( !context.empty() )
      {
        t << ( TableRow() << "" << _("Repository:") << context.repoInfo().asUserString() );
      }
      t << ( TableRow() << "" << _("Key Fingerprint:") << str::gapify( key.fingerprint(), 4 ) )
        << ( TableRow() << "" << _("Key Name:") << key.name() )
        << ( TableRow() << "" << _("Key Algorithm:") << key.algoName() )
        << ( TableRow() << "" << _("Key Created:") << key.created() )
        << ( TableRow() << "" << _("Key Expires:") << key.expiresAsString() );
      for ( const PublicSubkeyData & sub : key.subkeys() )
        t << ( TableRow() << "" << _("Subkey:") << sub.asString() );
      t << ( TableRow() << "" << _("Rpm Name:") << key.rpmName() );

      return str << t;
    }

    inline std::ostream & dumpKeyInfo( std::ostream & str, const PublicKey & key, const KeyContext & context = KeyContext() )
    { return dumpKeyInfo( str, key.keyData(), context ); }
  } // namespace
  ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // KeyRingReceive
    ///////////////////////////////////////////////////////////////////

    struct KeyRingReceive : public callback::ReceiveReport<KeyRingReport>
    {
      KeyRingReceive()
          : _gopts(Zypper::instance().config())
          {}

      ////////////////////////////////////////////////////////////////////

      virtual void infoVerify( const std::string & file_r, const PublicKeyData & keyData_r, const KeyContext & context = KeyContext() )
      {
        if ( keyData_r.expired() )
        {
          Zypper::instance().out().warning( str::Format(_("The gpg key signing file '%1%' has expired.")) % file_r );
          dumpKeyInfo( (std::ostream&)ColorStream(std::cout,ColorContext::MSG_WARNING), keyData_r, context );
        }
        else if ( keyData_r.daysToLive() < 15 )
        {
          Zypper::instance().out().info( str::Format(
            PL_( "The gpg key signing file '%1%' will expire in %2% day.",
                 "The gpg key signing file '%1%' will expire in %2% days.",
                 keyData_r.daysToLive() )) % file_r %  keyData_r.daysToLive() );
          dumpKeyInfo( std::cout, keyData_r, context );
        }
        else if ( Zypper::instance().out().verbosity() > Out::NORMAL )
        {
          dumpKeyInfo( std::cout, keyData_r, context );
        }
      }

      ////////////////////////////////////////////////////////////////////
      virtual bool askUserToAcceptUnsignedFile(
          const std::string & file, const KeyContext & context)
      {
        if (_gopts.no_gpg_checks)
        {
          MIL << "Accepting unsigned file (" << file << ", repo: "
              << (context.empty() ? "(unknown)" : context.repoInfo().alias())
              << ")" << std::endl;

          if (context.empty())
            Zypper::instance().out().warning(
              str::Format(_("Accepting an unsigned file '%s'.")) % file,
              Out::HIGH);
          else
            Zypper::instance().out().warning(
              str::Format(_("Accepting an unsigned file '%s' from repository '%s'."))
              % file % context.repoInfo().asUserString(),
              Out::HIGH);

          return true;
        }

        std::string msg;
        if ( context.empty() )
          // translator: %1% is a file name
          msg = str::Format(_("File '%1%' is unsigned.") ) % file;
        else
          // translator: %1% is a file name, %2% a repositories name
          msg = str::Format(_("File '%1%' from repository '%2%' is unsigned.") ) % file % context.repoInfo().asUserString();
        Zypper::instance().out().warning( msg );

        hintUnsignedData();

        if ( !context.empty() )
          hintIfMasterIndex( file );

        warnCanNotVerifyFile();

        Zypper::instance().out().gap();
        // TODO: use text::join( msg, text::qContinue() )
        // once the above texts for mgs are translated
        std::string question;
        if (context.empty())
          question = str::Format(
            // TranslatorExplanation: speaking of a file
            _("File '%s' is unsigned, continue?")) % file;
        else
          question = str::Format(
            // TranslatorExplanation: speaking of a file
            _("File '%s' from repository '%s' is unsigned, continue?"))
            % file % context.repoInfo().asUserString();

        return read_bool_answer(PROMPT_YN_GPG_UNSIGNED_FILE_ACCEPT, question, false);
      }

      ////////////////////////////////////////////////////////////////////

      virtual bool askUserToAcceptUnknownKey(
          const std::string & file,
          const std::string & id,
          const KeyContext & context)
      {
        if (_gopts.no_gpg_checks)
        {
          MIL
            << "Accepting file signed with an unknown key ("
            << file << "," << id << ", repo: "
            << (context.empty() ? "(unknown)" : context.repoInfo().alias())
            << ")" << std::endl;

          if (context.empty())
            Zypper::instance().out().warning(str::Format(
                _("Accepting file '%s' signed with an unknown key '%s'."))
                % file % id);
          else
            Zypper::instance().out().warning(str::Format(
                _("Accepting file '%s' from repository '%s' signed with an unknown key '%s'."))
                % file % context.repoInfo().asUserString() % id);

          return true;
        }

        std::string msg;
        if ( context.empty() )
          // translator: %1% is a file name, %2% is a gpg key ID
          msg = str::Format(_("File '%1%' is signed with an unknown key '%2%'.") ) % file % id;
        else
          // translator: %1% is a file name, %2% is a gpg key ID, %3% a repositories name
          msg = str::Format(_("File '%1%' from repository '%3%' is signed with an unknown key '%2%'.") ) % file % id % context.repoInfo().asUserString();
        Zypper::instance().out().warning( msg );

        hintUnsignedData();

        if ( !context.empty() )
          hintIfMasterIndex( file );

        warnCanNotVerifyFile();

        Zypper::instance().out().gap();
        std::string question;
        // TODO: use text::join( msg, text::qContinue() )
        // once the above texts for mgs are translated
        if (context.empty())
          question = str::Format(
            // translators: the last %s is gpg key ID
            _("File '%s' is signed with an unknown key '%s'. Continue?")) % file % id;
        else
          question = str::Format(
            // translators: the last %s is gpg key ID
            _("File '%s' from repository '%s' is signed with an unknown key '%s'. Continue?"))
             % file % context.repoInfo().asUserString() % id;

        return read_bool_answer(PROMPT_YN_GPG_UNKNOWN_KEY_ACCEPT, question, false);
      }

      virtual KeyRingReport::KeyTrust askUserToAcceptKey(
          const PublicKey &key, const KeyContext & context)
      {
        return askUserToAcceptKey( key, context, true );
      }

      KeyRingReport::KeyTrust askUserToAcceptKey(
                const PublicKey &key_r, const KeyContext & context_r, bool canTrustTemporarily_r )
      {
        Zypper & zypper = Zypper::instance();

        std::ostringstream s;
        s << std::endl;
        if (_gopts.gpg_auto_import_keys)
          s << _("Automatically importing the following key:") << std::endl;
        else if ( _gopts.no_gpg_checks && canTrustTemporarily_r  )
          s << _("Automatically trusting the following key:") << std::endl;
        else
          s << _("New repository or package signing key received:") << std::endl;

        // gpg key info
        dumpKeyInfo( s << std::endl, key_r, context_r )  << std::endl;

        // if --gpg-auto-import-keys or --no-gpg-checks print info and don't ask
        if (_gopts.gpg_auto_import_keys)
        {
          MIL << "Automatically importing key " << key_r << std::endl;
          zypper.out().info(s.str());
          hintFingerprint();
          return KeyRingReport::KEY_TRUST_AND_IMPORT;
        }
        else if (_gopts.no_gpg_checks && canTrustTemporarily_r )
        {
          MIL << "Automatically trusting key " << key_r << std::endl;
          zypper.out().info(s.str());
          hintFingerprint();
          return KeyRingReport::KEY_TRUST_TEMPORARILY;
        }

        // ask the user
        // FIXME: The embedded key table will reset the prompt color and
        // the actual question after the table is rendered normal. That's
        // why we print the 1st part here in PROMPT color and just pass the
        // question to propmpt().
        zypper.out().info( ColorString( s.str(), ColorContext::PROMPT ).str() );
        hintUnsignedData();
        hintFingerprint();
        zypper.out().gap();
        s.str( std::string() );	// clear the string

        if ( canTrustTemporarily_r ) {
          // translators: this message is shown after showing description of the key
          s << _("Do you want to reject the key, trust temporarily, or trust always?");
        } else {
          // translators: this message is shown after showing description of the key
          s << _("Do you want to reject the key, or trust always?");
        }


        // only root has access to rpm db where keys are stored
        bool canimport = geteuid() == 0 || _gopts.changedRoot;

        if ( !canimport && !canTrustTemporarily_r )
          return KeyRingReport::KEY_DONT_TRUST;

        PromptOptions popts;
        if ( canTrustTemporarily_r ) {
          if (canimport)
            // translators: r/t/a stands for Reject/TrustTemporarily/trustAlways(import)
            // translate to whatever is appropriate for your language
            // The anserws must be separated by slash characters '/' and must
            // correspond to reject/trusttemporarily/trustalways in that order.
            // The answers should be lower case letters.
            popts.setOptions(_("r/t/a/"), 0);
          else
            // translators: the same as r/t/a, but without 'a'
            popts.setOptions(_("r/t"), 0);
        } else {
          // translators: r/a stands for Reject/trustAlways(import)
          // translate to whatever is appropriate for your language
          // The anserws must be separated by slash characters '/' and must
          // correspond to reject/trusttemporarily/trustalways in that order.
          // The answers should be lower case letters.
          popts.setOptions(_("r/a/"), 0);
        }

        int off = 0;

        // translators: help text for the 'r' option in the 'r/t/a' prompt
        popts.setOptionHelp( off, _("Don't trust the key.") );

        if ( canTrustTemporarily_r ) {
          // translators: help text for the 't' option in the 'r/t/a' prompt
          popts.setOptionHelp( (++off), _("Trust the key temporarily.") );
        }

        if (canimport)
          // translators: help text for the 'a' option in the 'r/t/a' prompt
          popts.setOptionHelp( (++off), _("Trust the key and import it into trusted keyring.") );

        zypper.out().prompt(PROMPT_YN_GPG_KEY_TRUST, s.str(), popts);
        unsigned prep =
          get_prompt_reply(zypper, PROMPT_YN_GPG_KEY_TRUST, popts);

        if ( !canTrustTemporarily_r && prep == 1 )
          return KeyRingReport::KEY_TRUST_AND_IMPORT;
        else {
          switch (prep)
          {
          case 1:
              return KeyRingReport::KEY_TRUST_TEMPORARILY;
          case 2:
              return KeyRingReport::KEY_TRUST_AND_IMPORT;
          case 0:
          default:
            break;
          }
        }
        return KeyRingReport::KEY_DONT_TRUST;
      }

      ////////////////////////////////////////////////////////////////////

      virtual bool askUserToAcceptVerificationFailed(
          const std::string & file,
          const PublicKey & key,
          const KeyContext & context )
      {
        if (_gopts.no_gpg_checks)
        {
          MIL << str::Format("Ignoring failed signature verification for %s")
              % file << std::endl;

          std::ostringstream msg;
          if (context.empty())
            msg << str::Format(
                _("Ignoring failed signature verification for file '%s'!")) % file;
          else
            msg << str::Format(
                _("Ignoring failed signature verification for file '%s'"
                  " from repository '%s'!")) % file
                  % context.repoInfo().asUserString();

          msg
            << std::endl
            << _("Double-check this is not caused by some malicious"
                 " changes in the file!");

          Zypper::instance().out().warning(msg.str(), Out::QUIET);
          return true;
        }

        std::string msg;
        if ( context.empty() )
          // translator: %1% is a file name
          msg = str::Format(_("Signature verification failed for file '%1%'.") ) % file;
        else
          // translator: %1% is a file name, %2% a repositories name
          msg = str::Format(_("Signature verification failed for file '%1%' from repository '%2%'.") ) % file % context.repoInfo().asUserString();
        Zypper::instance().out().error( msg );

        hintUnsignedData();

        if ( !context.empty() )
          hintIfMasterIndex( file );

        warnFileModifiedAfterSigning();

        Zypper::instance().out().gap();
        return read_bool_answer( PROMPT_YN_GPG_CHECK_FAILED_IGNORE, text::join( msg, text::qContinue() ), false);
      }

      ////////////////////////////////////////////////////////////////////

      virtual void report ( const UserData & data )
      {
        if ( data.type() == zypp::ContentType( KeyRingReport::ACCEPT_PACKAGE_KEY_REQUEST ) )
          return askUserToAcceptPackageKey( data );
        else if ( data.type() == zypp::ContentType( KeyRingReport::KEYS_NOT_IMPORTED_REPORT ) )
          return reportKeysNotImportedReport( data );
        else if ( data.type() == zypp::ContentType( KeyRingReport::REPORT_AUTO_IMPORT_KEY ) )
          return reportAutoImportKey( data );
        WAR << "Unhandled report() call" << endl;
      }

      void askUserToAcceptPackageKey( const UserData & data )
      {
        if ( !data.hasvalue("PublicKey") || !data.hasvalue(("KeyContext")) ) {
          WAR << "Missing arguments in report call for content type: " << data.type() << endl;
          return;
        }
        const PublicKey &key  = data.get<PublicKey>("PublicKey");
        const KeyContext &ctx = data.get<KeyContext>("KeyContext");
        KeyRingReport::KeyTrust res = askUserToAcceptKey(key,ctx, false);
        data.set("TrustKey", res == KeyRingReport::KEY_TRUST_AND_IMPORT);
        return;
      }

      void reportKeysNotImportedReport( const UserData & data )
      {
        if ( !data.hasvalue("Keys") )
        {
          WAR << "Missing arguments in report call for content type: " << data.type() << endl;
          return;
        }
        Zypper & zypper = Zypper::instance();

        zypper.out().notePar(_("The rpm database seems to contain old V3 version gpg keys which are meanwhile obsolete and considered insecure:") );

        zypper.out().gap();
        for ( const Edition & ed : data.get( "Keys", std::set<Edition>() ) )
          zypper.out().info( str::Str() << /*indent8*/"        gpg-pubkey-" << ed );

        Zypper::instance().out().par( 4,
                                      str::Format(_("To see details about a key call '%1%'.") )
                                      % "rpm -qi GPG-PUBKEY-VERSION" );

        Zypper::instance().out().par( 4,
                                      str::Format(_("Unless you believe the key in question is still in use, you can remove it from the rpm database calling '%1%'.") )
                                      % "rpm -e GPG-PUBKEY-VERSION" );

        zypper.out().gap();
      }

      void reportAutoImportKey( const UserData & data_r )
      {
        if ( not ( data_r.hasvalue("KeyDataList") && data_r.hasvalue("KeySigning") && data_r.hasvalue("KeyContext") ) ) {
          WAR << "Missing arguments in report call for content type: " << data_r.type() << endl;
          return;
        }
        const std::list<PublicKeyData> & keyDataList { data_r.get<std::list<PublicKeyData>>("KeyDataList") };
        const PublicKeyData &            keySigning  { data_r.get<PublicKeyData>("KeySigning") };
        const KeyContext &               context     { data_r.get<KeyContext>("KeyContext") };

        Zypper & zypper { Zypper::instance() };

        // translator: %1% is the number of keys, %2% the name of a repository
        zypper.out().notePar( str::Format( PL_( "Received %1% new package signing key from repository %2%:",
                                                "Received %1% new package signing keys from repository %2%:",
                                         keyDataList.size() )) % keyDataList.size() % context.repoInfo().asUserString() );

        zypper.out().par( 2,_("Those additional keys are usually used to sign packages shipped by the repository. In order to validate those packages upon download and installation the new keys will be imported into the rpm database.") );

        auto newTag { HIGHLIGHTString(_("New:") ) };
        for ( const auto & kd : keyDataList ) {
          zypper.out().gap();
          dumpKeyInfo( std::cout << "  " << newTag << endl, kd );
        }

        zypper.out().par( 2,HIGHLIGHTString(_("The repository metadata introducing the new keys have been signed and validated by the trusted key:")) );
        zypper.out().gap();
        dumpKeyInfo( std::cout, keySigning, context );

        zypper.out().gap();
      }

    private:
      const Config & _gopts;
    };

    ///////////////////////////////////////////////////////////////////
    // DigestReceive
    ///////////////////////////////////////////////////////////////////

    struct DigestReceive : public callback::ReceiveReport<DigestReport>
    {
      DigestReceive() : _gopts(Zypper::instance().config()) {}

      ////////////////////////////////////////////////////////////////////

      virtual bool askUserToAcceptNoDigest( const Pathname &file )
      {
        std::string question = (str::Format(_("No digest for file %s.")) % file).str() + " " + text::qContinue();
        return read_bool_answer(PROMPT_GPG_NO_DIGEST_ACCEPT, question, _gopts.no_gpg_checks);
      }

      ////////////////////////////////////////////////////////////////////

      virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
      {
        std::string question = (str::Format(_("Unknown digest %s for file %s.")) %name % file).str() + " " + text::qContinue();
        return read_bool_answer(PROMPT_GPG_UNKNOWN_DIGEST_ACCEPT, question, _gopts.no_gpg_checks);
      }

      ////////////////////////////////////////////////////////////////////

      virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
      {
        Zypper & zypper = Zypper::instance();
        std::string unblock( found.substr( 0, 4 ) );

        zypper.out().gap();
        // translators: !!! BOOST STYLE PLACEHOLDERS ( %N% - reorder and multiple occurrence is OK )
        // translators: %1%      - a file name
        // translators: %2%      - full path name
        // translators: %3%, %4% - checksum strings (>60 chars), please keep them aligned
        zypper.out().warning( str::Format(_(
                "Digest verification failed for file '%1%'\n"
                "[%2%]\n"
                "\n"
                "  expected %3%\n"
                "  but got  %4%\n" ) )
                % file.basename()
                % file
                % requested
                % found
        );

        zypper.out().info( MSG_WARNINGString(_(
                "Accepting packages with wrong checksums can lead to a corrupted system "
                "and in extreme cases even to a system compromise." ) ).str()
        );
        zypper.out().gap();

        // translators: !!! BOOST STYLE PLACEHOLDERS ( %N% - reorder and multiple occurrence is OK )
        // translators: %1%      - abbreviated checksum (4 chars)
        zypper.out().info( str::Format(_(
                "However if you made certain that the file with checksum '%1%..' is secure, correct\n"
                "and should be used within this operation, enter the first 4 characters of the checksum\n"
                "to unblock using this file on your own risk. Empty input will discard the file.\n" ) )
                % unblock
        );

        // translators: A prompt option
        PromptOptions popts( unblock+"/"+_("discard"), 1 );
        // translators: A prompt option help text
        popts.setOptionHelp( 0, _("Unblock using this file on your own risk.") );
        // translators: A prompt option help text
        popts.setOptionHelp( 1, _("Discard the file.") );
        popts.setShownCount( 1 );

        // translators: A prompt text
        zypper.out().prompt( PROMPT_GPG_WRONG_DIGEST_ACCEPT, _("Unblock or discard?"), popts );
        int reply = get_prompt_reply( zypper, PROMPT_GPG_WRONG_DIGEST_ACCEPT, popts );
        return( reply == 0 );
      }

    private:
      const Config & _gopts;
    };

   ///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

class KeyRingCallbacks {

  private:
    KeyRingReceive _keyRingReport;

  public:
    KeyRingCallbacks()
    {
      _keyRingReport.connect();
    }

    ~KeyRingCallbacks()
    {
      _keyRingReport.disconnect();
    }

};

class DigestCallbacks {

  private:
    DigestReceive _digestReport;

  public:
    DigestCallbacks()
    {
      _digestReport.connect();
    }

    ~DigestCallbacks()
    {
      _digestReport.disconnect();
    }

};


#endif // ZMART_KEYRINGCALLBACKS_H
// Local Variables:
// c-basic-offset: 2
// End:
