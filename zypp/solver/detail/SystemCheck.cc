/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/solver/detail/SystemCheck.cc
 *
*/
#include <iostream>
#include <fstream>
#include <vector>

#include "zypp/base/LogTools.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/String.h"

#include "zypp/ZYppFactory.h"
#include "zypp/ZConfig.h"
#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"
#include "zypp/solver/detail/SystemCheck.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    Pathname		_file;
    CapabilitySet	_require;
    CapabilitySet	_conflict;

    typedef vector<string> CapList;    
      
    SystemCheck::SystemCheck() {
	_file = ZConfig::instance().solver_checkSystemFile();
	try
	{
	    Target_Ptr trg( getZYpp()->target() );
	    if ( trg )
		_file = trg->root() / _file;
	}
	catch ( ... )
	{
	    // noop: Someone decided to let target() throw if the ptr is NULL ;(
	}

	PathInfo pi( _file );
	if ( ! pi.isFile() ) {
	    WAR << "Can't read " << pi << endl;
	    return;
	}
	std::ifstream infile( _file.c_str() );
	for( iostr::EachLine in( infile ); in; in.next() ) {
	    std::string l( str::trim(*in) );
	    if ( ! l.empty() && l[0] != '#' )
	    {
		CapList capList;
		str::split( l, back_inserter(capList), ":" );
		if (capList.size() == 2 ) {
		    CapList::iterator it = capList.begin();
		    if (*it == "requires") {
			_require.insert(Capability(*(it+1)));
		    } else if (*it == "conflicts") {
			_conflict.insert(Capability(*(it+1)));
		    } else {
			ERR << "Wrong parameter: " << l << endl;
		    }
		} else {
		    ERR << "Wrong line: " << l << endl;
		}
	    }
	}
	MIL << "Read " << pi << endl;
    }

    const Pathname & file() {
	return _file;
    }

    const CapabilitySet & requiredSystemCap() {
	return _require;
    }

    const CapabilitySet & conflictSystemCap() {
	return _conflict;
    }
      

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const SystemCheck & obj )
    {
      str << _file << endl;
      str << "requires" << endl;
      for (CapabilitySet::const_iterator it = _require.begin(); it != _require.end(); ++it)
	  str << "  " << *it << endl;

      str << "conflicts" << endl;
      for (CapabilitySet::const_iterator it = _conflict.begin(); it != _conflict.end(); ++it)
	  str << "  " << *it << endl;

      return str;
    }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
