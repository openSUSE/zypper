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
#include <boost/format.hpp>

#include "zypp/base/Logger.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/Pathname.h"
#include "zypp/KeyRing.h"
#include "zypp/Digest.h"

#include "utils/prompt.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // KeyRingReceive
    ///////////////////////////////////////////////////////////////////

    struct KeyRingReceive : public zypp::callback::ReceiveReport<zypp::KeyRingReport>
    {
      KeyRingReceive()
          : _gopts(Zypper::instance()->globalOpts())
          , _show_alias(Zypper::instance()->config().show_alias)
          {}

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
            Zypper::instance()->out().warning(boost::str(
              boost::format(_("Accepting an unsigned file '%s'.")) % file),
              Out::HIGH);
          else
            Zypper::instance()->out().warning(boost::str(
              boost::format(_("Accepting an unsigned file '%s' from repository '%s'."))
                % file % (_show_alias ? context.repoInfo().alias() : context.repoInfo().name())),
              Out::HIGH);

          return true;
        }

        std::string question;
        if (context.empty())
          question = boost::str(boost::format(
            // TranslatorExplanation: speaking of a file
            _("File '%s' is unsigned, continue?")) % file);
        else
          question = boost::str(boost::format(
            // TranslatorExplanation: speaking of a file
            _("File '%s' from repository '%s' is unsigned, continue?"))
            % file % (_show_alias ? context.repoInfo().alias() : context.repoInfo().name()));

        return read_bool_answer(PROMPT_YN_GPG_UNSIGNED_FILE_ACCEPT, question, false);
      }

      ////////////////////////////////////////////////////////////////////

      virtual bool askUserToAcceptUnknownKey(
          const std::string & file,
          const std::string & id,
          const zypp::KeyContext & context)
      {
        if (_gopts.no_gpg_checks)
        {
          MIL
            << "Accepting file signed with an unknown key ("
            << file << "," << id << ", repo: "
            << (context.empty() ? "(unknown)" : context.repoInfo().alias())
            << ")" << std::endl;

          if (context.empty())
            Zypper::instance()->out().warning(boost::str(boost::format(
                _("Accepting file '%s' signed with an unknown key '%s'."))
                % file % id));
          else
            Zypper::instance()->out().warning(boost::str(boost::format(
                _("Accepting file '%s' from repository '%s' signed with an unknown key '%s'."))
                % file % (_show_alias ? context.repoInfo().alias() : context.repoInfo().name()) % id));

          return true;
        }

        std::string question;
        if (context.empty())
          question = boost::str(boost::format(
            // translators: the last %s is gpg key ID
            _("File '%s' is signed with an unknown key '%s'. Continue?")) % file % id);
        else
          question = boost::str(boost::format(
            // translators: the last %s is gpg key ID
            _("File '%s' from repository '%s' is signed with an unknown key '%s'. Continue?"))
             % file % (_show_alias ? context.repoInfo().alias() : context.repoInfo().name()) % id);

        return read_bool_answer(PROMPT_YN_GPG_UNKNOWN_KEY_ACCEPT, question, false);
      }

      ////////////////////////////////////////////////////////////////////

      virtual KeyRingReport::KeyTrust askUserToAcceptKey(
          const PublicKey &key, const zypp::KeyContext & context)
      {
        Zypper & zypper = *Zypper::instance();
        std::ostringstream s;
	const std::string & keyid = key.id(), keyname = key.name(),
	  fingerprint = key.fingerprint();

	s << std::endl;
	if (_gopts.gpg_auto_import_keys)
	  s << _("Automatically importing the following key:") << std::endl;
	else if (_gopts.no_gpg_checks)
          s << _("Automatically trusting the following key:") << std::endl;
        else
          s << _("New repository or package signing key received:") << std::endl;

        // gpg key info
        s
          << str::form(_("Key ID: %s"), keyid.c_str()) << std::endl
          << str::form(_("Key Name: %s"), keyname.c_str()) << std::endl
          << str::form(_("Key Fingerprint: %s"), fingerprint.c_str()) << std::endl
          << str::form(_("Key Created: %s"), key.created().asString().c_str()) << std::endl
          << str::form(_("Key Expires: %s"), key.expiresAsString().c_str()) << std::endl;
        if (!context.empty())
          s << str::form(_("Repository: %s"), _show_alias ?
              context.repoInfo().alias().c_str() :
              context.repoInfo().name().c_str())
            << std::endl;

        // if --gpg-auto-import-keys or --no-gpg-checks print info and don't ask
        if (_gopts.gpg_auto_import_keys)
        {
          MIL << boost::format("Automatically importing key id '%s', '%s', fingerprint '%s'")
              % keyid % keyname % fingerprint << std::endl;
          zypper.out().info(s.str());
          return KeyRingReport::KEY_TRUST_AND_IMPORT;
        }
        else if (_gopts.no_gpg_checks)
        {
          MIL << boost::format("Automatically trusting key id '%s', '%s', fingerprint '%s'")
              % keyid % keyname % fingerprint << std::endl;
          zypper.out().info(s.str());
          return KeyRingReport::KEY_TRUST_TEMPORARILY;
        }

        // ask the user
        s << std::endl;
        // translators: this message is shown after showing description of the key
        s << _("Do you want to reject the key, trust temporarily, or trust always?");

        // only root has access to rpm db where keys are stored
        bool canimport = geteuid() == 0 || _gopts.changedRoot;

        PromptOptions popts;
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
        // translators: help text for the 'r' option in the 'r/t/a' prompt
        popts.setOptionHelp(0, _("Don't trust the key."));
        // translators: help text for the 't' option in the 'r/t/a' prompt
        popts.setOptionHelp(1, _("Trust the key temporarily."));
        if (canimport)
          // translators: help text for the 'a' option in the 'r/t/a' prompt
          popts.setOptionHelp(2, _("Trust the key and import it into trusted keyring."));

        if (!zypper.globalOpts().non_interactive)
          clear_keyboard_buffer();
        zypper.out().prompt(PROMPT_YN_GPG_KEY_TRUST, s.str(), popts);
        unsigned prep =
          get_prompt_reply(zypper, PROMPT_YN_GPG_KEY_TRUST, popts);
        switch (prep)
        {
        case 0:
          return KeyRingReport::KEY_DONT_TRUST;
        case 1:
          return KeyRingReport::KEY_TRUST_TEMPORARILY;
        case 2:
          return KeyRingReport::KEY_TRUST_AND_IMPORT;
        default:
          return KeyRingReport::KEY_DONT_TRUST;
        }
        return KeyRingReport::KEY_DONT_TRUST;
      }

      ////////////////////////////////////////////////////////////////////

      virtual bool askUserToAcceptVerificationFailed(
          const std::string & file,
          const PublicKey & key,
          const zypp::KeyContext & context )
      {
        if (_gopts.no_gpg_checks)
        {
          MIL << boost::format("Ignoring failed signature verification for %s")
              % file << std::endl;

          std::ostringstream msg;
          if (context.empty())
            msg << boost::format(
                _("Ignoring failed signature verification for file '%s'!")) % file;
          else
            msg << boost::format(
                _("Ignoring failed signature verification for file '%s'"
                  " from repository '%s'!")) % file
                  % (_show_alias ? context.repoInfo().alias() : context.repoInfo().name());

          msg
            << std::endl
            << _("Double-check this is not caused by some malicious"
                 " changes in the file!");

          Zypper::instance()->out().warning(msg.str(), Out::QUIET);
          return true;
        }

        std::ostringstream question;
        if (context.empty())
          question << boost::format(
            _("Signature verification failed for file '%s'.")) % file;
        else
          question << boost::format(
            _("Signature verification failed for file '%s' from repository '%s'."))
              % file % (_show_alias ? context.repoInfo().alias() : context.repoInfo().name());

        question
          << std::endl
          << _("Warning: This might be caused by a malicious change in the file!\n"
               "Continuing might be risky. Continue anyway?");

        return read_bool_answer(
            PROMPT_YN_GPG_CHECK_FAILED_IGNORE, question.str(), false);
      }

    private:
      const GlobalOptions & _gopts;
      bool _show_alias;
    };

    ///////////////////////////////////////////////////////////////////
    // DigestReceive
    ///////////////////////////////////////////////////////////////////

    struct DigestReceive : public zypp::callback::ReceiveReport<zypp::DigestReport>
    {
      DigestReceive() : _gopts(Zypper::instance()->globalOpts()) {}

      ////////////////////////////////////////////////////////////////////

      virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
      {
	std::string question = boost::str(boost::format(
	    _("No digest for file %s.")) % file) + " " + _("Continue?");
        return read_bool_answer(PROMPT_GPG_NO_DIGEST_ACCEPT, question, _gopts.no_gpg_checks);
      }

      ////////////////////////////////////////////////////////////////////

      virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
      {
        std::string question = boost::str(boost::format(
            _("Unknown digest %s for file %s.")) %name % file) + " " +
            _("Continue?");
        return read_bool_answer(PROMPT_GPG_UNKNOWN_DIGEST_ACCEPT, question, _gopts.no_gpg_checks);
      }

      ////////////////////////////////////////////////////////////////////

      virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
      {
        if (_gopts.no_gpg_checks)
        {
          WAR << boost::format(
              "Ignoring failed digest verification for %s (expected %s, found %s).")
              % file % requested % found << std::endl;
          Zypper::instance()->out().warning(boost::str(boost::format(
              _("Ignoring failed digest verification for %s (expected %s, found %s)."))
              % file % requested % found),
              Out::QUIET);
          return true;
        }

	std::string question = boost::str(boost::format(
	    _("Digest verification failed for %s. Expected %s, found %s."))
	    % file.basename() % requested % found) + " " + _("Continue?");
        return read_bool_answer(PROMPT_GPG_WRONG_DIGEST_ACCEPT, question, false);
      }

    private:
      const GlobalOptions & _gopts;
    };

   ///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

class KeyRingCallbacks {

  private:
    zypp::KeyRingReceive _keyRingReport;

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
    zypp::DigestReceive _digestReport;

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


#endif // ZMD_BACKEND_KEYRINGCALLBACKS_H
// Local Variables:
// c-basic-offset: 2
// End:
