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
#include <zypp/base/Logger.h>
#include <zypp/FileChecker.h>
#include <zypp/ZYppFactory.h>
#include <zypp/Digest.h>
#include <zypp/KeyRing.h>

using std::endl;

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
	// Remember askUserToAcceptWrongDigest decision for at most 12hrs in memory;
	// Actually we just want to prevent asking the same question again when the
	// previously downloaded file is retrieved from the disk cache.
	static std::map<std::string,std::string> exceptions;
	static Date exceptionsAge;
	Date now( Date::now() );
	if ( !exceptions.empty() && now-exceptionsAge > 12*Date::hour )
	  exceptions.clear();

	WAR << "File " <<  file << " has wrong checksum " << real_checksum << " (expected " << _checksum << ")" << endl;
	if ( !exceptions.empty() && exceptions[real_checksum.checksum()] == _checksum.checksum() )
	{
	  WAR << "User accepted " <<  file << " with WRONG CHECKSUM. (remembered)" << std::endl;
          return;
	}
        else if ( report->askUserToAcceptWrongDigest( file, _checksum.checksum(), real_checksum.checksum() ) )
        {
          WAR << "User accepted " <<  file << " with WRONG CHECKSUM." << std::endl;
	  exceptions[real_checksum.checksum()] = _checksum.checksum();
	  exceptionsAge = now;
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
    for ( std::list<FileChecker>::const_iterator it = _checkers.begin(); it != _checkers.end(); ++it )
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


  SignatureFileChecker::SignatureFileChecker()
  {}

  SignatureFileChecker::SignatureFileChecker( Pathname signature_r )
  { signature( std::move(signature_r) ); }

  void SignatureFileChecker::addPublicKey( const Pathname & publickey_r )
  { addPublicKey( PublicKey(publickey_r) ); }

  void SignatureFileChecker::addPublicKey( const PublicKey & publickey_r )
  { getZYpp()->keyRing()->importKey( publickey_r, false ); }

  void SignatureFileChecker::operator()( const Pathname & file_r ) const
  {
    const Pathname & sig { signature() };
    if ( not ( sig.empty() || PathInfo(sig).isExist() ) )
      ZYPP_THROW( ExceptionType("Signature " + sig.asString() + " not found.") );

    MIL << "Checking " << file_r << " file validity using digital signature.." << endl;
    // const_cast because the workflow is allowed to store result values here
    SignatureFileChecker & self { const_cast<SignatureFileChecker&>(*this) };
    self.file( file_r );
    if ( not getZYpp()->keyRing()->verifyFileSignatureWorkflow( self ) )
      ZYPP_THROW( ExceptionType( "Signature verification failed for "  + file_r.basename() ) );
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
