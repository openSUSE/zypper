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

  const ResStore & Source_Ref::resolvables()
  { return _pimpl->resolvables(*this); }

  std::ostream & Source_Ref::dumpOn( std::ostream & str ) const
  { return _pimpl->dumpOn( str ); }

  const Pathname Source_Ref::provideFile(const Pathname & file_r,
				     const unsigned media_nr)
  { return _pimpl->provideFile(file_r, media_nr); }

  const Pathname Source_Ref::provideDir(const Pathname & dir_r,
				    const unsigned media_nr,
				    const bool recursive)
  { return _pimpl->provideDir(dir_r, media_nr, recursive); }

  const bool Source_Ref::valid() const
  { return _pimpl!=NULL ? _pimpl->valid() : false; }

  const bool Source_Ref::enabled() const
  { return _pimpl->enabled(); }

  void Source_Ref::enable()
  { _pimpl->enable(); }

  void Source_Ref::disable()
  { _pimpl->disable(); }

  string Source_Ref::alias (void) const
  { return _pimpl->alias(); }

  string Source_Ref::zmdname (void) const
  { return _pimpl->zmdname(); }

  string Source_Ref::zmddescription (void) const
  { return _pimpl->zmddescription (); }

  unsigned Source_Ref::priority (void) const
  { return _pimpl->priority(); }

  unsigned Source_Ref::priority_unsubscribed (void) const
  { return _pimpl->priority_unsubscribed(); }

  Url Source_Ref::url (void) const
  { return _pimpl->url (); }

  const Pathname & Source_Ref::path (void) const
  { return _pimpl->path (); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
