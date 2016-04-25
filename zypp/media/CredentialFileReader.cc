/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/CredentialFileReader.cc
 *
 */
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/InputStream.h"
#include "zypp/parser/IniDict.h"

#include "zypp/media/CredentialFileReader.h"

using std::endl;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace media
  {
    ///////////////////////////////////////////////////////////////////
    namespace
    {
      // Looks like INI but allows multiple sections for the same URL
      // but different user (in .cat files). So don't use an Ini
      // Also support a global section without '[URL]' which is used
      // in credential files.
      // -------------------------------------
      // username = emptyUSER
      // password = emptyPASS
      // -------------------------------------
      // [http://server/tmp/sumafake222]
      // username = USER
      // password = PASS
      //
      // [http://server/tmp/sumafake222]
      // username = USER2
      // password = PASS
      // -------------------------------------
      struct CredentialFileReaderImpl : public parser::IniParser
      {
	typedef CredentialFileReader::ProcessCredentials ProcessCredentials;

	struct StopParsing {};

	CredentialFileReaderImpl( const Pathname & input_r, const ProcessCredentials & callback_r )
	: _input( input_r )
	, _callback( callback_r )
	{
	  try
	  {
	    parse( input_r );
	  }
	  catch ( StopParsing )
	  { /* NO error but consumer aborted parsing */ }
	}

	// NO-OP; new sections are opened in consume()
	virtual void beginParse()
	{ /*EMPTY*/ }

	// start a new section [url]
	virtual void consume( const std::string & section_r )
	{
	  endParse();	// close any open section
	  _secret.reset( new AuthData );
	  try
	  {
	    _secret->setUrl( Url(section_r) );
	  }
	  catch ( const url::UrlException & )
	  {
	    ERR << "Ignore invalid URL '" << section_r << "' in file " << _input << endl;
	    _secret.reset();	// ignore this section
	  }
	}

	virtual void consume( const std::string & section_r, const std::string & key_r, const std::string & value_r )
	{
	  if ( !_secret && section_r.empty() )
	    _secret.reset( new AuthData );	// a initial global section without [URL]

	  if ( _secret )
	  {
	    if ( key_r == "username" )
	      _secret->setUsername( value_r );
	    else if ( key_r == "password" )
	      _secret->setPassword( value_r );
	    else
	      WAR << "Ignore unknown attribute '" << key_r << "=" << value_r << "' in file " << _input << endl;
	  }
	  // else: ignored section due to wrong URL
	}

	// send any valid pending section
	virtual void endParse()
	{
	  if ( _secret )
	  {
	    if ( _secret->valid() )
	    {
	      if ( !_callback( _secret ) )
		throw( StopParsing() );
	    }
	    else
	      ERR << "Ignore invalid credentials for URL '" << _secret->url() << "' in file " << _input << endl;
	  }
	}

      private:
	const Pathname & 		_input;
	const ProcessCredentials &	_callback;
	AuthData_Ptr			_secret;
      };
    } // namespace
    ///////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    //
    // CLASS NAME : CredentialFileReader
    //
    //////////////////////////////////////////////////////////////////////

    CredentialFileReader::CredentialFileReader( const Pathname & crfile_r, const ProcessCredentials & callback_r )
    { CredentialFileReaderImpl( crfile_r, callback_r ); }

    CredentialFileReader::~CredentialFileReader()
    {}

  } // namespace media
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

