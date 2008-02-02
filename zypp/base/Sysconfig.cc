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
#include "zypp/Pathname.h"

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

    } // namespace sysconfig
  } // namespace base
} // namespace zypp
