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

  void SourceManager::removeSource(const unsigned id)
  {
    SourceMap::iterator it = _sources.find(id);
    if (it != _sources.end())
    {
#warning disable the source here
      _sources.erase(it);
    }
  }

  const Pathname SourceManager::provideFile(const unsigned id,
				            const unsigned media_nr,
					    const Pathname & path_r)
  {
    RW_pointer<Source> src = findSource(id);
    return src->provideFile (path_r, media_nr);
  }

  const Pathname SourceManager::provideDir(const unsigned id,
					   const unsigned media_nr,
					   const Pathname & path_r,
					   const bool recursive)
  {
    RW_pointer<Source> src = findSource(id);
    return src->provideDir (path_r, media_nr, recursive);

  }

  const bool SourceManager::enabled(const unsigned id) const
  {
    RW_pointer<Source> src = findSource(id);
    return src->enabled();
  }

  void SourceManager::enable(const unsigned id)
  {
    RW_pointer<Source> src = findSource(id);
    src->enable();
  }

  void SourceManager::disable(const unsigned id)
  {
    RW_pointer<Source> src = findSource(id);
    src->disable();
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

  RW_pointer<Source> SourceManager::findSource(const unsigned id) const
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
