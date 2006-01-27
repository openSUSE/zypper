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

    SourceImpl_Ptr SourceImpl::_nullimpl;

    /** Null implementation */
    SourceImpl_Ptr SourceImpl::nullimpl()
    {
      if (_nullimpl == 0)
	_nullimpl = new SourceImpl;
      return _nullimpl;
    }


    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SourceImpl::SourceImpl
    //	METHOD TYPE : Ctor
    //
    SourceImpl::SourceImpl(media::MediaAccess::Ptr & media_r,
                           const Pathname & path_r,
			   const std::string & alias_r)
    : _media(media_r)
    , _path(path_r)
    , _enabled(true)
    , _alias (alias_r)
    , _res_store_initialized(false)
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SourceImpl::~SourceImpl
    //	METHOD TYPE : Dtor
    //
    SourceImpl::~SourceImpl()
    {}

    const ResStore & SourceImpl::resolvables(Source_Ref source_r)
    {
      if (! _res_store_initialized)
      {
	createResolvables(source_r);
	_res_store_initialized = true;
      }
      return _store;
     }

    const Pathname SourceImpl::provideFile(const Pathname & file_r,
					   const unsigned media_nr)
    {
      if (_media == 0)
	ZYPP_THROW(Exception("Source not initialized properly"));
      _media->provideFile (file_r);
      return _media->localPath (file_r);
    }

    /** Provide a directory to local filesystem */
    const Pathname SourceImpl::provideDir(const Pathname & path_r,
					  const unsigned media_nr,
					  const bool recursive)
    {
      if (_media == 0)
	ZYPP_THROW(Exception("Source not initialized properly"));
      if (recursive)
	_media->provideDirTree(path_r);
      else
	_media->provideDir(path_r);
      return _media->localPath (path_r);
    }

    void SourceImpl::createResolvables(Source_Ref source_r)
    {}

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
