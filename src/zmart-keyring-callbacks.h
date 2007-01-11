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

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>

#include "AliveCursor.h"
#include "zmart-misc.h"

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
        cout << CLEARLN << file << _(" is unsigned, continue?") << " [y/n]: " << flush;
        return readBoolAnswer();
      }
      
#ifndef LIBZYPP_1xx
      virtual bool askUserToImportKey( const PublicKey &key )
      {
        if ( geteuid() != 0 )
          return false;
        
        cout << CLEARLN << _("Import key ") << key.id() << _(" to trusted keyring?") << "  [y/n]: " << flush;
        return readBoolAnswer();
      } 
#endif

#ifdef LIBZYPP_1xx
      virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &id, const std::string &/*keyname*/, const std::string &/*fingerprint*/ )
#else
      virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &id )
#endif
      {
        cout << CLEARLN << file << _(" is signed with an unknown key id: ") << id << ", " << _("continue?") << " [y/n]: " << flush;
        return readBoolAnswer();
      }

#ifdef LIBZYPP_1xx
      virtual bool askUserToTrustKey( const std::string &keyid, const std::string &keyname, const std::string &fingerprint) {
#else
      virtual bool askUserToTrustKey( const PublicKey &key ) {
	const std::string& keyid = key.id(), keyname = key.name(),
	  fingerprint = key.fingerprint();
#endif
        cout  << CLEARLN << _("Do you want to trust key id ") << keyid << " " << keyname << _(" fingerprint:") << fingerprint << " ? [y/n]: "  << flush;
        return readBoolAnswer();
      }

#ifdef LIBZYPP_1xx
      virtual bool askUserToAcceptVerificationFailed( const std::string &file, const std::string &keyid, const std::string &keyname, const std::string &fingerprint ) {
#else
      virtual bool askUserToAcceptVerificationFailed( const std::string &file,const PublicKey &key ) {
	const std::string& keyid = key.id(), keyname = key.name(),
	  fingerprint = key.fingerprint();
#endif
        cout << file << _("Signature verification for ") << file
	     << _(" with public key id ") << keyid << " " << keyname << _(" fingerprint:") << fingerprint << _(" failed, THIS IS RISKY!") << ". " << _("continue?") << " [y/n]: " << endl;
        return readBoolAnswer(); // TODO do this with format()
      }
    };


    struct DigestReceive : public zypp::callback::ReceiveReport<zypp::DigestReport>
    {
      virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
      {
	cout << CLEARLN << _("No digest for ") << file
	     << ", " << _("continue?") << " [y/n]" << flush;
        return readBoolAnswer();
      }
      virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
      {
	cout << CLEARLN << _("Unknown digest ") << name << _(" for ") << file
	     << ", " << _("continue?") << " [y/n]" << flush;
        return readBoolAnswer();
      }
      virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
      {
	cout << CLEARLN << _("Digest verification failed for ") << file
	     << _(", expected ") << requested << _(" found ") << found
	     << ", " << _("continue?") << " [y/n]" << flush;
        return readBoolAnswer();
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
