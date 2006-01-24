/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/SourceImpl.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/source/SourceImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(SourceImpl);

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SourceImpl::SourceImpl
    //	METHOD TYPE : Ctor
    //
    SourceImpl::SourceImpl(media::MediaAccess::Ptr & media_r,
                           const Pathname & path_r)
    : _media(media_r)
    , _path(path_r)
    , _enabled(true)
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SourceImpl::~SourceImpl
    //	METHOD TYPE : Dtor
    //
    SourceImpl::~SourceImpl()
    {}

    const Pathname SourceImpl::provideFile(const Pathname & file_r,
					   const unsigned media_nr)
    {
      _media->provideFile (file_r);
      return _media->localPath (file_r);
    }

    /** Provide a directory to local filesystem */
    const Pathname SourceImpl::provideDir(const Pathname & path_r,
					  const unsigned media_nr,
					  const bool recursive)
    {
      if (recursive)
	_media->provideDirTree(path_r);
      else
	_media->provideDir(path_r);
      return _media->localPath (path_r);
    }

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
