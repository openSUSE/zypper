/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp2/source/cached/CachedSourceImpl.h"

using std::endl;
using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////
namespace cached
{ /////////////////////////////////////////////////////////////////

CachedSourceImpl::CachedSourceImpl()
{

}

CachedSourceImpl::~CachedSourceImpl()
{

}


void CachedSourceImpl::factoryInit()
{
  if ( ! ( (url().getScheme() == "file") || (url().getScheme() == "dir") ) )
  {
    ZYPP_THROW( Exception( "Plaindir only supports local paths, scheme [" + url().getScheme() + "] is not local" ) );
  }

  MIL << "Plaindir source initialized." << std::endl;
  MIL << "   Url      : " << url() << std::endl;
  MIL << "   Path     : " << path() << std::endl;
}

void CachedSourceImpl::createResolvables(Source_Ref source_r)
{
  Pathname thePath = Pathname(url().getPathName()) + path();
  MIL << "Going to read dir " << thePath << std::endl;

  //extract_packages_from_directory( _store, thePath, selfSourceRef(), true );
}

      /////////////////////////////////////////////////////////////////
    } // namespace plaindir
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
