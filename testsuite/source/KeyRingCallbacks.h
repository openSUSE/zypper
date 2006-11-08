/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/KeyRingCallbacks.cc
 *
*/

#ifndef ZMD_BACKEND_KEYRINGCALLBACKS_H
#define ZMD_BACKEND_KEYRINGCALLBACKS_H

#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // KeyRingReceive
    ///////////////////////////////////////////////////////////////////
    struct KeyRingReceive : public zypp::callback::ReceiveReport<zypp::KeyRingReport>
    {
	virtual bool askUserToAcceptUnsignedFile( const std::string &file )
	{
	  XXX << "(" << file << ")" << std::endl;
	  return true;
	}
	virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &keyid )
	{
	  XXX << "(" << file << ", " << keyid << ")" << std::endl;
	  return true;
	}
	virtual bool askUserToTrustKey( const PublicKey &key )
	{
	  XXX << "(" << key << ")" << std::endl;
	  return true;
	}
	virtual bool askUserToAcceptVerificationFailed( const std::string &file, const PublicKey &key )
	{
	  XXX << "(" << file << ", " << key << ")" << std::endl;
	  return true;
	}
    };


    struct DigestReceive : public zypp::callback::ReceiveReport<zypp::DigestReport>
    {
      virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
      {
	XXX << "(" << file << ")" << std::endl;
	return true;
      }
      virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
      {
	XXX << "(" << file << ", " << name << ")" << std::endl;
	return true;
      }
      virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
      {
	XXX << "(" << file << ", " << requested << ", " << found << ")" << std::endl;
	return true;
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
