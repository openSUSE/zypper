/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_QUERYPOOL_KEYRINGCALLBACKS_H
#define ZYPP_QUERYPOOL_KEYRINGCALLBACKS_H

#include <stdlib.h>
#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/base/Sysconfig.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// KeyRingReceive
///////////////////////////////////////////////////////////////////
struct KeyRingReceive : public zypp::callback::ReceiveReport<zypp::KeyRingReport>
{
  KeyRingReceive()
  {}

  virtual bool askUserToAcceptUnsignedFile( const std::string &file )
  {
    return true;
  }
  virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &id )
  {
    return true;
  }

  virtual bool askUserToImportKey( const PublicKey &key )
  {
    DBG << "By default zypp-query-pool does not import keys for now." << std::endl;
    return false;
  }

  virtual bool askUserToTrustKey(  const PublicKey &key  )
  {
    return true;
  }
  virtual bool askUserToAcceptVerificationFailed( const std::string &file,  const PublicKey &key  )
  {
    return true;
  }
};


struct DigestReceive : public zypp::callback::ReceiveReport<zypp::DigestReport>
{
  DigestReceive()
  {}

  virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
  {
    return true;
  }
  virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
  {
    return true;
  }
  virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
  {
    // this smells like false
    return true;
  }
};

///////////////////////////////////////////////////////////////////
}
; // namespace zypp
///////////////////////////////////////////////////////////////////

class KeyRingCallbacks
{

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

class DigestCallbacks
{

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
