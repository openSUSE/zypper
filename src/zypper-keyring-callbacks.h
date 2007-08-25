/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_KEYRINGCALLBACKS_H
#define ZMART_KEYRINGCALLBACKS_H

#include <stdlib.h>
#include <iostream>
#include <boost/format.hpp>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>

#include "AliveCursor.h"
#include "zypper-callbacks.h"

///////////////////////////////////////////////////////////////////
namespace zypp {
/////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // KeyRingReceive
    ///////////////////////////////////////////////////////////////////
    struct KeyRingReceive : public zypp::callback::ReceiveReport<zypp::KeyRingReport>
    {
      virtual bool askUserToAcceptUnsignedFile( const std::string &file )
      {
        if (gSettings.no_gpg_checks)
        {
          MIL << "Accepting unsigned file (" << file << ")" << endl;
          cout_v << boost::format(_("Warning: Accepting an unsigned file %s.")) % file;
          return true;
        }

        // TranslatorExplanation: speaking of a file
        std::string question = boost::str(boost::format(
            _("%s is unsigned, continue?")) % file);
        return read_bool_answer(question, false);
      }

      virtual bool askUserToImportKey( const PublicKey &key )
      {
        if ( geteuid() != 0 )
          return false;

        std::string question = boost::str(boost::format(
            _("Import key %s to trusted keyring?")) % key.id());
        return read_bool_answer(question, false);
      }

      virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &id )
      {
        if (gSettings.no_gpg_checks)
        {
          MIL << "Accepting file signed with an unknown key (" << file << "," << id << ")" << endl;
          cout_n << boost::format(
              _("Warning: Accepting file %s signed with an unknown key %s."))
              % file % id;
          return true;
        }

        // TranslatorExplanation: speaking of a file
        std::string question = boost::str(boost::format(
            _("%s is signed with an unknown key %s. Continue?")) % file % id);
        return read_bool_answer(question, false);
      }

      virtual bool askUserToTrustKey( const PublicKey &key )
      {
	const std::string& keyid = key.id(), keyname = key.name(),
	  fingerprint = key.fingerprint();

        if (gSettings.no_gpg_checks)
        {
          MIL << boost::format("Automatically trusting key id %s, %s, fingerprint %s")
              % keyid % keyname % fingerprint << endl;
          cout_n << boost::format(
              _("Automatically trusting key id %s, %s, fingerprint %s"))
              % keyid % keyname % fingerprint << endl;
          return true;
        }

        std::string question = boost::str(boost::format(
	    _("Do you want to trust key id %s, %s, fingerprint %s"))
	    % keyid % keyname % fingerprint);
        return read_bool_answer(question, false);
      }

      virtual bool askUserToAcceptVerificationFailed( const std::string &file,const PublicKey &key )
      {
	const std::string& keyid = key.id(), keyname = key.name(),
	  fingerprint = key.fingerprint();

        if (gSettings.no_gpg_checks)
        {
          MIL << boost::format(
              "Ignoring failed signature verification for %s"
              " with public key id %s, %s, fingerprint %s")
              % file % keyid % keyname % fingerprint << endl;
          cerr << boost::format(
              _("Warning: Ignoring failed signature verification for %s"
                " with public key id %s, %s, fingerprint %s!") +
		string("\n") +
               _("Double-check this is not caused by some malicious"
                " changes in the file!"))
              %file % keyid % keyname % fingerprint << endl;
          return true;
        }

        std::string question = boost::str(boost::format(
            _("Signature verification failed for %s"
              " with public key id %s, %s, fingerprint %s.") +
	      string("\n") +
             _("Warning: This might be caused by a malicious change in the file!") +
	      string("\n") +
             _("Continuing is risky! Continue anyway?"))
            % file % keyid % keyname % fingerprint);
        return read_bool_answer(question, false);
      }
    };

    struct DigestReceive : public zypp::callback::ReceiveReport<zypp::DigestReport>
    {
      virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
      {
	std::string question = boost::str(boost::format(
	    _("No digest for file %s.")) % file) + " " + _("Continue?");
        return read_bool_answer(question, gSettings.no_gpg_checks);
      }

      virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
      {
        std::string question = boost::str(boost::format(
            _("Unknown digest %s for file %s.")) %name % file) + " " +
            _("Continue?");
        return read_bool_answer(question, gSettings.no_gpg_checks);
      }

      virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
      {
        if (gSettings.no_gpg_checks)
        {
          WAR << boost::format(
              "Ignoring failed digest verification for %s (expected %s, found %s).")
              % file % requested % found << endl;
          cerr << boost::format(
              _("Ignoring failed digest verification for %s (expected %s, found %s)."))
              % file % requested % found << endl;
          return true;
        }

	std::string question = boost::str(boost::format(
	    _("Digest verification failed for %s. Expected %s, found %s."))
	    % file.basename() % requested % found) + " " + _("Continue?");
        return read_bool_answer(question, false);
      }
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
