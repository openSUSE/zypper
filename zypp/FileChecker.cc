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
        ZYPP_THROW( FileCheckException( file.basename() + " has no checksum" ) );
      }
    }
    else
    {
      CheckSum real_checksum( _checksum.type(), filesystem::checksum( file, _checksum.type() ));
      if ( (real_checksum != _checksum) )
      {
        if ( report->askUserToAcceptWrongDigest( file, _checksum.checksum(), real_checksum.checksum() ) )
        {
          WAR << "User accepted " <<  file << " with WRONG CHECKSUM." << std::endl;
          return;
        }
        else
        {
          ZYPP_THROW( FileCheckException( file.basename() + " has wrong checksum" ) );
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
    ZYpp::Ptr z = getZYpp();

    if ( (! PathInfo(_signature).isExist()) && (!_signature.empty()))
    {
      ZYPP_THROW(FileCheckException("Signature " + _signature.asString() + " not found."));
    }

    MIL << "checking " << file << " file validity using digital signature.." << endl;
    bool valid = z->keyRing()->verifyFileSignatureWorkflow( file, file.basename(), _signature, _context);

    if (!valid)
      ZYPP_THROW( FileCheckException( "Signature verification failed for "  + file.basename() ) );
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
