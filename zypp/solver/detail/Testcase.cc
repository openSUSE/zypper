/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/solver/detail/Testcase.cc
 *
*/
#include "zypp/solver/detail/Testcase.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/PathInfo.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;

//---------------------------------------------------------------------------

Testcase::Testcase()
    :dumpPath("/var/log/YaST2/solverTestcase")
{
}

Testcase::Testcase(const std::string & path)
    :dumpPath(path)
{
}
	

Testcase::~Testcase()
{
}

bool Testcase::createTestcase(Resolver & resolver)
{
    PathInfo path (dumpPath);

    if ( !path.isExist() ) {
	if (zypp::filesystem::mkdir (dumpPath)!=0) {
	    ERR << "Cannot create directory " << dumpPath << endl;
	    return false;
	}
    } else {
	if (!path.isDir()) {
	    ERR << dumpPath << " is not a directory." << endl;
	    return false;
	}
    }
    zypp::base::LogControl::instance().logfile( dumpPath +"/y2log" );
    zypp::base::LogControl::TmpExcessive excessive; // ZYPP_FULLLOG=1
    
    resolver.resolveDependencies();
    
    return true;
}

      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////
