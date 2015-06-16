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

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "FileChecker"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ChecksumFileChecker::ChecksumFileChecker( const CheckSum &checksum )
    : _checksum(checksum)
  {}

  void ChecksumFileChecker::operator()( const Pathname &file ) const
  {
    //MIL << "checking " << file << " file against checksum '" << _checksum << "'" << endl;
    callback::SendReport<DigestReport> report;

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
        ZYPP_THROW( ExceptionType( file.basename() + " has no checksum" ) );
      }
    }
    else
    {
      CheckSum real_checksum( _checksum.type(), filesystem::checksum( file, _checksum.type() ));
      if ( (real_checksum != _checksum) )
      {
	WAR << "File " <<  file << " has wrong checksum " << real_checksum << " (expected " << _checksum << ")" << endl;
        if ( report->askUserToAcceptWrongDigest( file, _checksum.checksum(), real_checksum.checksum() ) )
        {
          WAR << "User accepted " <<  file << " with WRONG CHECKSUM." << std::endl;
          return;
        }
        else
        {
          ZYPP_THROW( ExceptionType( file.basename() + " has wrong checksum" ) );
        }
      }
    }
  }

  void NullFileChecker::operator()(const Pathname &file ) const
  {
    MIL << "+ null check on " << file << endl;
    return;
  }

  void CompositeFileChecker::operator()(const Pathname &file ) const
  {
    //MIL << _checkers.size() << " checkers" << endl;
    for ( list<FileChecker>::const_iterator it = _checkers.begin(); it != _checkers.end(); ++it )
    {
      if ( *it )
      {
        //MIL << "+ chk" << endl;
        (*it)(file);
      }
      else
      {
        ERR << "Invalid checker" << endl;
      }
    }
  }

  void CompositeFileChecker::add( const FileChecker &checker )
  { _checkers.push_back(checker); }


  SignatureFileChecker::SignatureFileChecker( const Pathname & signature )
       : _signature(signature)
  {}

  SignatureFileChecker::SignatureFileChecker()
  {}

  void SignatureFileChecker::setKeyContext(const KeyContext & keycontext)
  { _context = keycontext; }

  void SignatureFileChecker::addPublicKey( const Pathname & publickey, const KeyContext & keycontext )
  { addPublicKey( PublicKey(publickey), keycontext ); }

  void SignatureFileChecker::addPublicKey( const PublicKey & publickey, const KeyContext & keycontext )
  {
    getZYpp()->keyRing()->importKey(publickey, false);
    _context = keycontext;
  }

  void SignatureFileChecker::operator()(const Pathname &file ) const
  {
    if ( (! PathInfo(_signature).isExist()) && (!_signature.empty()) )
    {
      ZYPP_THROW( ExceptionType("Signature " + _signature.asString() + " not found.") );
    }

    MIL << "checking " << file << " file validity using digital signature.." << endl;
    _fileValidated = false;
    _fileAccepted = getZYpp()->keyRing()->verifyFileSignatureWorkflow( file, file.basename(), _signature, _fileValidated, _context );

    if ( !_fileAccepted )
      ZYPP_THROW( ExceptionType( "Signature verification failed for "  + file.basename() ) );
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
