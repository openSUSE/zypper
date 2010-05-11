/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PublicKey.cc
 *
*/
#include <climits>

#include <iostream>
#include <vector>

//#include "zypp/base/Logger.h"

#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"
#include "zypp/PublicKey.h"
#include "zypp/ExternalProgram.h"
#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Logger.h"
#include "zypp/Date.h"
#include "zypp/TmpPath.h"

#include <ctime>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PublicKey::Impl
  //
  /** PublicKey implementation. */
  struct PublicKey::Impl
  {
    Impl()
    {}

    Impl( const Pathname & keyfile )
    {
      PathInfo info( keyfile );
      MIL << "Takeing pubkey from " << keyfile << " of size " << info.size() << " and sha1 " << filesystem::checksum(keyfile, "sha1") << endl;

      if ( !info.isExist() )
        ZYPP_THROW(Exception("Can't read public key from " + keyfile.asString() + ", file not found"));

      if ( copy( keyfile, _data_file.path() ) != 0 )
        ZYPP_THROW(Exception("Can't copy public key data from " + keyfile.asString() + " to " +  _data_file.path().asString() ));

      readFromFile();
    }

    Impl( const filesystem::TmpFile & sharedfile )
      : _data_file( sharedfile )
    { readFromFile(); }

    public:
      /** Offer default Impl. */
      static shared_ptr<Impl> nullimpl()
      {
        static shared_ptr<Impl> _nullimpl( new Impl );
        return _nullimpl;
      }

      std::string asString() const
      {
	return str::form( "[%s-%s] [%s] [%s] [TTL %d]",
			  id().c_str(), str::hexstring(created(),8).substr(2).c_str(),
			  name().c_str(),
			  fingerprint().c_str(),
			  daysToLive() );
      }

      std::string id() const
      { return _id; }

      std::string name() const
      { return _name; }

      std::string fingerprint() const
      { return _fingerprint; }

      std::string gpgPubkeyVersion() const
      { return _id.empty() ? _id : str::toLower( _id.substr(8,8) ); }

      std::string gpgPubkeyRelease() const
      { return _created ? str::hexstring( _created ).substr(2) : std::string(); }

      Date created() const
      { return _created; }

      Date expires() const
      { return _expires; }

      std::string expiresAsString() const
      {
	if ( !_expires )
	{ // translators: an annotation to a gpg keys expiry date
	  return _("(does not expire)");
	}
	std::string ret( _expires.asString() );
	int ttl( daysToLive() );
	if ( ttl <= 90 )
	{
	  ret += " ";
	  if ( ttl < 0 )
	  { // translators: an annotation to a gpg keys expiry date
	    ret += _("(EXPIRED)");
	  }
	  else if ( ttl == 0 )
	  { // translators: an annotation to a gpg keys expiry date
	    ret += _("(expires within 24h)");
	  }
	  else
	  { // translators: an annotation to a gpg keys expiry date
	    ret += str::form( _PL("(expires in %d day)", "(expires in %d days)", ttl ), ttl );
	  }
	}
	return ret;
      }

      Pathname path() const
      { return _data_file.path(); }

      bool expired() const
      {
	Date exp( expires() );
	return( exp && exp < Date::now() );
      }

      int daysToLive() const
      {
	Date exp( expires() );
	if ( ! expires() )
	  return INT_MAX;
	exp -= Date::now();
	return exp < 0 ? exp / Date::day - 1 : exp / Date::day;
      }

    protected:

      void readFromFile()
      {
        PathInfo info( _data_file.path() );
        MIL << "Reading pubkey from " << info.path() << " of size " << info.size() << " and sha1 " << filesystem::checksum(info.path(), "sha1") << endl;

        static filesystem::TmpDir dir;
        const char* argv[] =
        {
          "gpg",
          "-v",
          "--no-default-keyring",
          "--fixed-list-mode",
          "--with-fingerprint",
          "--with-colons",
          "--homedir",
          dir.path().asString().c_str(),
          "--quiet",
          "--no-tty",
          "--no-greeting",
          "--batch",
          "--status-fd",
          "1",
          _data_file.path().asString().c_str(),
          NULL
        };

        ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);

        std::string line;
        bool sawpub = false;
        bool sawsig = false;

        // pub:-:1024:17:A84EDAE89C800ACA:971961473:1214043198::-:SuSE Package Signing Key <build@suse.de>:
        // fpr:::::::::79C179B2E1C820C1890F9994A84EDAE89C800ACA:
        // sig:::17:A84EDAE89C800ACA:1087899198:::::[selfsig]::13x:
        // sig:::17:9E40E310000AABA4:980442706::::[User ID not found]:10x:
        // sig:::1:77B2E6003D25D3D9:980443247::::[User ID not found]:10x:
        // sub:-:2048:16:197448E88495160C:971961490:1214043258::: [expires: 2008-06-21]
        // sig:::17:A84EDAE89C800ACA:1087899258:::::[keybind]::18x:

        for ( line = prog.receiveLine(); !line.empty(); line = prog.receiveLine() )
        {
          // trim trailing NL.
          if ( line.empty() )
            continue;
          if ( line[line.size()-1] == '\n' )
            line.erase( line.size()-1 );

          // split at ':'
          std::vector<std::string> words;
          str::splitFields( line, std::back_inserter(words), ":" );
          if( words.empty() )
            continue;

          if ( words[0] == "pub" )
          {
            if ( sawpub )
              continue;
            sawpub = true;
            // take default from pub
            _id      = words[4];
            _name    = words[9];
            _created = Date(str::strtonum<Date::ValueType>(words[5]));
            _expires = Date(str::strtonum<Date::ValueType>(words[6]));

          }
          else if ( words[0] == "sig" )
          {
            if ( sawsig || words[words.size()-2] != "13x"  )
              continue;
            sawsig = true;
            // update creation and expire dates from 1st signature type "13x"
            if ( ! words[5].empty() )
              _created = Date(str::strtonum<Date::ValueType>(words[5]));
            if ( ! words[6].empty() )
              _expires = Date(str::strtonum<Date::ValueType>(words[6]));
          }
          else if ( words[0] == "fpr" )
          {
            _fingerprint = words[9];
          }
          else if ( words[0] == "uid" )
          {
            if ( ! words[9].empty() )
              _name = words[9];
          }
        }
        prog.close();

        if ( _id.size() == 0 )
          ZYPP_THROW( BadKeyException( "File " + _data_file.path().asString() + " doesn't contain public key data" , _data_file.path() ) );

        //replace all escaped semicolon with real ':'
        str::replaceAll( _name, "\\x3a", ":" );

        MIL << "Read pubkey from " << info.path() << ": " << asString() << endl;
      }

    private:
      filesystem::TmpFile _data_file;

      std::string _id;
      std::string _name;
      std::string _fingerprint;
      Date        _created;
      Date        _expires;

    private:
      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const
      { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PublicKey::PublicKey
  //	METHOD TYPE : Ctor
  //
  PublicKey::PublicKey()
  : _pimpl( Impl::nullimpl() )
  {}

  PublicKey::PublicKey( const Pathname & file )
  : _pimpl( new Impl(file) )
  {}

  PublicKey::PublicKey( const filesystem::TmpFile & sharedfile )
  : _pimpl( new Impl(sharedfile) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : PublicKey::~PublicKey
  //	METHOD TYPE : Dtor
  //
  PublicKey::~PublicKey()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to implementation:
  //
  ///////////////////////////////////////////////////////////////////

  std::string PublicKey::asString() const
  { return _pimpl->asString(); }

  std::string PublicKey::id() const
  { return _pimpl->id(); }

  std::string PublicKey::name() const
  { return _pimpl->name(); }

  std::string PublicKey::fingerprint() const
  { return _pimpl->fingerprint(); }

  std::string PublicKey::gpgPubkeyVersion() const
  { return _pimpl->gpgPubkeyVersion(); }

  std::string PublicKey::gpgPubkeyRelease() const
  { return _pimpl->gpgPubkeyRelease(); }

  Date PublicKey::created() const
  { return _pimpl->created(); }

  Date PublicKey::expires() const
  { return _pimpl->expires(); }

  std::string PublicKey::expiresAsString() const
  { return _pimpl->expiresAsString(); }

  bool PublicKey::expired() const
  { return _pimpl->expired(); }

  int PublicKey::daysToLive() const
  { return _pimpl->daysToLive(); }

  Pathname PublicKey::path() const
  { return _pimpl->path(); }

  bool PublicKey::operator==( PublicKey b ) const
  {
    return (   b.id() == id()
            && b.fingerprint() == fingerprint()
            && b.created() == created() );
  }

  bool PublicKey::operator==( std::string sid ) const
  {
    return sid == id();
  }

  std::ostream & dumpOn( std::ostream & str, const PublicKey & obj )
  {
    str << "[" << obj.name() << "]" << endl;
    str << "  fpr " << obj.fingerprint() << endl;
    str << "   id " << obj.id() << endl;
    str << "  cre " << obj.created() << endl;
    str << "  exp " << obj.expiresAsString() << endl;
    str << "  ttl " << obj.daysToLive() << endl;
    str << "  rpm " << obj.gpgPubkeyVersion() << "-" << obj.gpgPubkeyRelease() << endl;
    str << "]";
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
