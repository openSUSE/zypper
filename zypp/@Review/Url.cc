/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                                      |
|                                         (C) 2002, 2003 SuSE Linux AG |
\----------------------------------------------------------------------/

   File:       Url.cc
   Purpose:    An URL class
   Authors:    Ludwig Nussel <lnussel@suse.de>
               Cornelius Schumacher <cschum@suse.de>
   Maintainer: Cornelius Schumacher <cschum@suse.de>

/-*/

#include <cerrno>

#include <iostream>

#include <y2util/Y2SLog.h>
#include <y2util/stringutil.h>
#include <y2util/Url.h>

using namespace std;

Url::ProtocolStrings::ProtocolStrings()
{
  insert( value_type( unknown, ""      ) );
  insert( value_type( file,    "file"  ) );
  insert( value_type( ftp,     "ftp"   ) );
  insert( value_type( http,    "http"  ) );
  insert( value_type( https,   "https" ) );
  insert( value_type( cd,      "cd"    ) );
  insert( value_type( dvd,     "dvd"   ) );
  insert( value_type( nfs,     "nfs"   ) );
  insert( value_type( dir,     "dir"   ) );
  insert( value_type( hd,      "hd"    ) );
  insert( value_type( smb,     "smb"   ) );
  insert( value_type( cifs,    "cifs"  ) );
}

Url::ProtocolStrings Url::_protocolStrings;

Url::Url()
    : _protocol( unknown ), _valid( false )
{
}

Url::Url( const string & url )
{
    _valid = split( url, _protocol, _protocolString, _username, _password,
                    _host, _port, _path, _options );
    clearifinvalid(_valid);
}

bool Url::operator==( const Url &url ) const
{
  return ( saveAsString() == url.saveAsString() );
}

void Url::clearifinvalid( bool valid )
{
    if ( valid) return;

    _protocolString = _username = _password = _path = _host.erase();
    _protocol = unknown;
    _port = -1;
    _options.erase( _options.begin(), _options.end() );
}

bool Url::set( const string url )
{
    _valid = split( url, _protocol, _protocolString, _username, _password,
                    _host, _port, _path , _options);
    clearifinvalid(_valid);

    return _valid;
}

void Url::setProtocol( Protocol p )
{
  _protocol = p;
  _protocolString = protocolToString( p );
}

void Url::setProtocolString( const std::string &str )
{
  _protocolString = str;
  _protocol = stringToProtocol( str );
}

void Url::setUsername( const std::string &str )
{
  _username = str;
}

void Url::setPassword( const std::string &str )
{
  _password = str;
}

void Url::setHost( const std::string &str )
{
  _host = str;
}

void Url::setPort( int port )
{
  _port = port;
}

void Url::setPath( const string &path )
{
  _path = path;
}

string Url::asString( bool path, bool options, bool plainpassword )   const
{
    if ( _protocol == file ) {
        return _protocolString + ":" + _path;
    }

    string url( _protocolString + "://" );
    if(!_username.empty())
    {
	url+=_username;
	if(!_password.empty())
	{
	    url+=':';
	    if(plainpassword)
		url+=_password;
	    else
		url+="PASSWORD";
	}
	url+='@';
    }
    url+=_host;
    if( _port >= 0 )
    {
	url += ':';
	url += stringutil::numstring( _port );
    }

    if(path)
    {
	url += _path;
	if(options)
	{
	    for(OptionMapType::const_iterator i = _options.begin();
		i != _options.end();
		++i)
	    {
		url+=';';
		url+=i->first;
		url+='=';
		url+=i->second;
	    }
	}
    }

    return url;
}

string Url::option(const string& key) const
{
    OptionMapType::const_iterator it;
    string value;

    if((it=_options.find(key)) != _options.end())
	value=it->second;

    return value;
}

bool Url::split( const string &url,
                 Url::Protocol &protocol,
	         string &protocolString,
	         string &username,
	         string &password,
	         string &hostname,
	         int &port,
	         string &path,
	         OptionMapType &options )
{
    protocolString = username = password = hostname = path = string();
    protocol = unknown;
    port = -1;

    // protocol
    string::size_type posColon = url.find(':');
    string::size_type posSlash = url.find('/');

    bool hasScheme = ( posColon != string::npos ) &&
                     ( posSlash == string::npos || posColon < posSlash );

    if ( hasScheme ) {
        protocolString = url.substr( 0, posColon );
        protocol = stringToProtocol( protocolString );
        if ( protocolString.empty() ) return false;
    } else {
        protocol = file;
        protocolString = protocolToString( protocol );
    }

    D__ << "protocol " << protocolString << endl;

    if ( protocol == file ) {
        if ( hasScheme ) path = url.substr( posColon + 1,
                                            url.size() - posColon );
        else path = url;
    } else {
        string::size_type lastpos = posColon + 1;

        // check for hierarchical url
        if( url.substr(lastpos,2) != "//" )
	    return false;

        lastpos = lastpos + 2;

        // check if non local url
        if( url[lastpos] != '/' )
        {
	    D__ << "nonlocal url " << url.substr(lastpos) << endl;
	    // optional username&password
	    string::size_type pos = url.find('@',lastpos);
	    if ( pos != string::npos )
	    {
	        string userandpass = url.substr(lastpos,pos-lastpos);
	        // set marker behind @
	        lastpos=pos+1;
	        // optional password
	        pos = userandpass.find(':');
	        if ( pos != string::npos )
	        {
		    // no username?
		    if(pos==0) return false;

		    password = userandpass.substr(pos+1);
		    D__ << "password " << string( password.size(), '*' ) << endl;
	        }
	        username = userandpass.substr(0,pos);
	        D__ << "username " << username << endl;
	    }

	    // hostname&port
	    pos = url.find('/',lastpos);
            if ( pos == string::npos ) pos = url.size();
            if ( pos == lastpos ) return false;

	    string hostandport = url.substr(lastpos,pos-lastpos);
	    // set marker on /
	    lastpos=pos;
	    // optional port
	    pos = hostandport.find(':');
	    if ( pos != string::npos )
	    {
		// no hostname?
		if(pos==0) return false;

                string portStr = hostandport.substr(pos+1);
                if ( !portStr.empty() ) {
                    port = strtol( portStr.c_str(), 0, 10 );
                    if ( errno == ERANGE || errno == EINVAL ) port = 0;
                    D__ << "port " << port << endl;
	        }
            }
	    hostname = hostandport.substr(0,pos);
	    D__ << "hostname " << hostname << endl;
        }

        // locate options
        string::size_type pos = url.find(';',lastpos);

        path = url.substr(lastpos,pos-lastpos);
        D__ << "path " << path << endl;

        options = OptionMapType();

        if(pos != string::npos)
        {
	    string optionstr = url.substr(pos+1);
	    string::size_type pos2;
	    while( !optionstr.empty() )
	    {
	        pos2 = optionstr.find(';');
	        string option = optionstr.substr(0,pos2);
	        if( pos2 != string::npos )
		    optionstr = optionstr.substr(pos2+1);
	        else
		    optionstr.erase();

	        // options consist of key=value
	        pos2 = option.find('=');
	        if( pos2 != string::npos )
	        {
		    string key = option.substr(0,pos2);
		    string value = option.substr(pos2+1);
		    options[key]=value;
	        }
	        else
		    return false;
	    }
        }
    }

    return true;
}

Url::Protocol Url::stringToProtocol( const string &protocolString )
{
  map<Protocol,string>::const_iterator it;
  for ( it = _protocolStrings.begin(); it != _protocolStrings.end(); ++it ) {
    if ( it->second == protocolString ) return it->first;
  }

  return unknown;
}

std::string Url::protocolToString( Url::Protocol p )
{
  return _protocolStrings[ p ];
}

bool Url::isLocal() const
{
  return _host.empty();
}

bool Url::isValid() const
{
  return _valid;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
**
**	DESCRIPTION :
*/
ostream & operator<<( ostream & str, const Url & obj )
{
  return str << obj.asString();
}




// vim:sw=4
