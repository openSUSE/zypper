/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/Sysconfig.cc
 *
*/

#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/StrMatcher.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/InputStream.h"
#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"

#include "zypp/base/Sysconfig.h"

using namespace std;
using namespace zypp::base;

namespace zypp {
  namespace base {
    namespace sysconfig {

      map<string,string> read( const Pathname & _path )
      {
	DBG << "Load '" << _path << "'" << endl;
	map<string,string> ret;

	string line;
	ifstream in( _path.asString().c_str() );
	if ( in.fail() ) {
	  WAR << "Unable to load '" << _path << "'" << endl;
	  return ret;
	}

	while( getline( in, line ) ) {
	  if ( *line.begin() != '#' ) {

	    string::size_type pos = line.find( '=', 0 );

	    if ( pos != string::npos ) {

	      string key = str::trim( line.substr( 0, pos ) );
	      string value = str::trim( line.substr( pos + 1, line.length() - pos - 1 ) );

	      if ( value.length() >= 2
		   && *(value.begin()) == '"'
		   && *(value.rbegin()) == '"' )
	      {
		value = value.substr( 1, value.length() - 2 );
	      }
	      if ( value.length() >= 2
		   && *(value.begin()) == '\''
		   && *(value.rbegin()) == '\'' )
	      {
		value = value.substr( 1, value.length() - 2 );
	      }
	      XXX << "KEY: '" << key << "' VALUE: '" << value << "'" << endl;
	      ret[key] = value;

	    } // '=' found

	  } // not comment

	} // while getline
	MIL << "done reading '" << _path << "'" << endl;
	return ret;
      }

      bool write( const Pathname & path_r, const std::string & key_r, const std::string & val_r, const std::string & newcomment_r )
      {
	if ( key_r.empty() )
	{
	  WAR << "Empty key in write " << path_r << endl;
	  return false;
	}

	PathInfo pi( path_r );
	if ( ! pi.isFile() )
	  ZYPP_THROW( Exception( str::Str() << path_r << ": " << Errno(ENOENT) ) );
	if ( ! pi.userMayRW() )
	  ZYPP_THROW( Exception( str::Str() << path_r << ": " << Errno(EACCES) ) );

	bool found = false;
	filesystem::TmpFile tmpf( filesystem::TmpFile::makeSibling( path_r ) );
	{
	  StrMatcher matches( "^[ \t]*"+key_r+"[ \t]*=", Match::REGEX );
	  std::ofstream o( tmpf.path().c_str() );
	  iostr::forEachLine( InputStream( path_r ),
			      [&]( int num_r, std::string line_r )->bool
			      {
				if ( !found && matches( line_r ) )
				{
				  o << key_r << '=' << val_r << endl;
				  found = true;
				  MIL << path_r << ": " << key_r << '=' << val_r << " changed on line " << num_r << endl;
				}
				else
				  o << line_r << endl;
				return true;
			      } );
	  if ( !found )
	  {
	    if ( newcomment_r.empty() )
	    {
	      WAR << path_r << ": " << key_r << '=' << val_r << " can not be added (no comment provided)." << endl;
	    }
	    else
	    {
	      std::vector<std::string> lines;
	      str::split( newcomment_r, std::back_inserter(lines), "\r\n" );
	      o << endl;
	      for ( const std::string & line : lines )
	      {
		if ( line[0] != '#' )
		  o << "# ";
		o << line << endl;
	      }
	      o << key_r << '=' << val_r << endl;
	      found = true;
	      MIL << path_r << ": " << key_r << '=' << val_r << " appended. " << endl;
	    }
	  }

	  if ( ! o )
	    ZYPP_THROW( Exception( str::Str() << tmpf.path() << ": " << Errno(EIO) ) );
	}

	// If everything is fine, exchange the files:
	int res = exchange( tmpf.path(), path_r );
	if ( res )
	{
	  ZYPP_THROW( Exception( str::Str() << tmpf.path() << ": " << Errno(res) ) );
	}
	return found;
      }

      bool writeStringVal( const Pathname & path_r, const std::string & key_r, const std::string & val_r, const std::string & newcomment_r )
      {
	return write( path_r, key_r, str::Str() << '"' << str::escape( val_r, '"' )<< '"', newcomment_r );
      }

    } // namespace sysconfig
  } // namespace base
} // namespace zypp
