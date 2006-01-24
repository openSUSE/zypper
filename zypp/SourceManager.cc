/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceManager.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/SourceManager.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"


using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceManager::SourceManager
  //	METHOD TYPE : Ctor
  //
  SourceManager::SourceManager()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceManager::~SourceManager
  //	METHOD TYPE : Dtor
  //
  SourceManager::~SourceManager()
  {}

  unsigned SourceManager::addSource(const Url & url_r, const Pathname & path_r)
  {
    Source src = SourceFactory().createFrom(url_r, path_r);
    RW_pointer<Source> src_ptr = RW_pointer<Source>(new Source(src));
    _sources[_next_id] = src_ptr;
    return _next_id++;
  }

  unsigned SourceManager::removeSource(const unsigned id)
  {

  }

  const Pathname SourceManager::provideFile(const unsigned id,
				            const unsigned media_nr,
					    const Pathname & path_r)
  {
    RW_pointer<Source> src = findSource(id);
    return src->provideFile (path_r, media_nr);
  }

  const Pathname provideDir(const unsigned id,
			    const unsigned media_nr,
			    const Pathname & path_r,
			    const bool recursive = false)
  {

  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const SourceManager & obj )
  {
    return str << "Source Manager has " << " sources" << endl;
  }

  unsigned SourceManager::_next_id = 0;

  RW_pointer<Source> SourceManager::findSource(const unsigned id)
  {
    SourceMap::const_iterator it = _sources.find(id);
    if (it == _sources.end())
    {
      ZYPP_THROW(Exception("Unknown source ID"));
    }
    return it->second;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
