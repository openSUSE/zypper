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

    Pathname		_file = "";
    Pathname            _dir = "";
    CapabilitySet	_require;
    CapabilitySet	_conflict;

    typedef vector<string> CapList;

    const SystemCheck & SystemCheck::instance()
    {
	static SystemCheck _val;
	return _val;
    }


  SystemCheck::SystemCheck() {
	if (_file.empty()) {
	    _file = ZConfig::instance().solver_checkSystemFile();
	    loadFile(_file);
	}
        if (_dir.empty()) {
          _dir = ZConfig::instance().solver_checkSystemFileDir();
          loadFiles();
        }
    }

    bool SystemCheck::setFile(const Pathname & file) const{
	MIL << "Setting checkFile to : " << file << endl;
	_file = file;
	loadFile(_file);
	return true;
    }

    bool SystemCheck::setDir(const Pathname & dir) const {
      MIL << "Setting checkFile directory to : " << dir << endl;
      loadFile(_file);
      _dir = dir;
      loadFiles();
      return true;
    }

    const Pathname & SystemCheck::file() {
	return _file;
    }

    const Pathname & SystemCheck::dir() {
	return _dir;
    }

    const CapabilitySet & SystemCheck::requiredSystemCap() const{
	return _require;
    }

    const CapabilitySet & SystemCheck::conflictSystemCap() const{
	return _conflict;
    }

    bool SystemCheck::loadFile(Pathname & file, bool reset_caps) const{
        Target_Ptr trg( getZYpp()->getTarget() );
        if ( trg )
          file = trg->assertRootPrefix( file );

	PathInfo pi( file );
	if ( ! pi.isFile() ) {
	    WAR << "Can't read " << file << " " << pi << endl;
	    return false;
	}

        if (reset_caps) {
          _require.clear();
          _conflict.clear();
        }

	std::ifstream infile( file.c_str() );
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
	return true;
    }

  bool SystemCheck::loadFiles() const {

    filesystem::dirForEach(_dir,
                           [this](const Pathname & dir_r, const char *const & name_r)->bool
                           {
                             const std::string wanted = ".check";
                             Pathname pth = dir_r/name_r;
                             if (pth.extension() != wanted) {
                               MIL << "Skipping " << pth << " (not a *.check file)" << endl;
                               return true;
                             }
                             else {
                               MIL << "Reading " << pth << endl;
                               return loadFile(pth, false /* do not reset caps */);
                             }
                           });
    return true;
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
