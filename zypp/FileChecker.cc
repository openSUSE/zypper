/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/FileChecker.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/FileChecker.h"
#include "zypp/ZYppFactory.h"
#include "zypp/Digest.h"
#include "zypp/KeyRing.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ChecksumFileChecker::ChecksumFileChecker( const CheckSum &checksum )
    : _checksum(checksum)
  {
  }

  void ChecksumFileChecker::operator()( const Pathname &file ) const
  {
    callback::SendReport<DigestReport> report;
    CheckSum real_checksum( _checksum.type(), filesystem::checksum( file, _checksum.type() ));
    
    if ( _checksum.empty() )
    {
      MIL << "File " <<  file << " has no checksum available." << std::endl;
      if ( report->askUserToAcceptNoDigest(file) )
      {
        MIL << "User accepted " <<  file << " with no checksum." << std::endl;
        return;
      }
      else
      {
        ZYPP_THROW(FileCheckException("No checksum available"));
      }
    }
    else
    {
      if ( (real_checksum != _checksum) )
      {
        if ( report->askUserToAcceptWrongDigest( file, _checksum.checksum(), real_checksum.checksum() ) )
        {
          WAR << "User accepted " <<  file << " with WRONG CHECKSUM." << std::endl;
          return;
        }
        else
        {
          ZYPP_THROW(FileCheckException("Wrong checksum"));
        }
      }
    }
  }

  void NullFileChecker::operator()(const Pathname &file ) const
  {
    return;
  }

  void CompositeFileChecker::operator()(const Pathname &file ) const
  {
    MIL << _checkers.size() << " checkers" << endl;
    for ( list<FileChecker>::const_iterator it = _checkers.begin(); it != _checkers.end(); ++it )
    { 
      MIL << "checking..." << endl;
      (*it)(file);
    }
  }
  
  void CompositeFileChecker::add( const FileChecker &checker )
  {
    //MIL << "||# " << _checkers.size() << endl;
    _checkers.push_back(checker);
    //MIL << "||* " << _checkers.size() << endl;
    
  }

  SignatureFileChecker::SignatureFileChecker( const Pathname &signature )
    : _signature(signature)
  {
  }
  
  SignatureFileChecker::SignatureFileChecker()
  {
  }
  
  void SignatureFileChecker::addPublicKey( const Pathname &publickey )
  {
    ZYpp::Ptr z = getZYpp();
    z->keyRing()->importKey(publickey, false);
  }
  
  void SignatureFileChecker::operator()(const Pathname &file ) const
  {
    ZYpp::Ptr z = getZYpp();
    MIL << "checking " << file << " file validity using digital signature.." << endl;
    bool valid = z->keyRing()->verifyFileSignatureWorkflow( file, string(), _signature);
    if (!valid)
      ZYPP_THROW(FileCheckException("Signature verification failed"));
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const FileChecker & obj )
  {
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
