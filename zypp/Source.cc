/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Source.cc
 *
*/
#include <cassert>

#include <iostream>

#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/SourceFactory.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  const Source_Ref Source_Ref::noSource;

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Source_Ref::Source_Ref
  //	METHOD TYPE : Ctor
  //
  Source_Ref::Source_Ref()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Source_Ref::Source_Ref
  //	METHOD TYPE : Ctor
  //
  Source_Ref::Source_Ref( const Impl_Ptr & impl_r )
  : _pimpl( impl_r )
  {
    assert( impl_r );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	Forward to SourceImpl:
  //
  ///////////////////////////////////////////////////////////////////

  const ResStore & Source_Ref::resolvables() const
  { return _pimpl->resolvables(); }

  const Pathname Source_Ref::provideFile(const Pathname & file_r,
				     const unsigned media_nr)
  { return _pimpl->provideFile(file_r, media_nr); }

  const Pathname Source_Ref::provideDir(const Pathname & dir_r,
                                         const unsigned media_nr,
                                         const bool recursive)
  { return _pimpl->provideDir(dir_r, media_nr, recursive); }

  const bool Source_Ref::enabled() const
  { return _pimpl->enabled(); }

  void Source_Ref::enable()
  { _pimpl->enable(); }

  void Source_Ref::disable()
  { _pimpl->disable(); }

  void Source_Ref::storeMetadata(const Pathname & cache_dir_r)
  { _pimpl->storeMetadata(cache_dir_r); }

  string Source_Ref::alias (void) const
  { return _pimpl->alias(); }

  string Source_Ref::id (void) const
  { return _pimpl->id(); }

  void Source_Ref::setId (const std::string id_r)
  { return _pimpl->setId (id_r); }

  string Source_Ref::zmdName (void) const
  { return _pimpl->zmdName(); }

  void Source_Ref::setZmdName (const std::string name_r)
  { return _pimpl->setZmdName( name_r ); }

  string Source_Ref::zmdDescription (void) const
  { return _pimpl->zmdDescription(); }

  void Source_Ref::setZmdDescription (const std::string desc_r)
  { return _pimpl->setZmdDescription( desc_r ); }

  unsigned Source_Ref::priority (void) const
  { return _pimpl->priority(); }

  void Source_Ref::setPriority (unsigned p)
  { return _pimpl->setPriority(p); }

  unsigned Source_Ref::priorityUnsubscribed (void) const
  { return _pimpl->priorityUnsubscribed(); }

  void Source_Ref::setPriorityUnsubscribed (unsigned p)
  { return _pimpl->setPriorityUnsubscribed( p ); }

  Url Source_Ref::url (void) const
  { return _pimpl->url (); }

  const Pathname & Source_Ref::path (void) const
  { return _pimpl->path (); }

  void Source_Ref::changeMedia(const media::MediaId & media_r, const Pathname & path_r)
  { _pimpl->changeMedia(media_r, path_r); }

  void Source_Ref::redirect(unsigned media_nr, const Url & new_url)
  { _pimpl->redirect(media_nr, new_url); }
  
  media::MediaVerifierRef Source_Ref::verifier(unsigned media_nr)
  { _pimpl->verifier(media_nr); }
  
  /******************************************************************
   **
   **	FUNCTION NAME : operator<<
   **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const Source_Ref & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
