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

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Source::Source
  //	METHOD TYPE : Ctor
  //
  Source::Source()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Source::Source
  //	METHOD TYPE : Ctor
  //
  Source::Source( const Impl_Ptr & impl_r )
  : _pimpl( impl_r )
  {
    assert( impl_r );
  }

  Source Source::_nullimpl;
  bool Source::_nullimpl_initialized = false;

  /** Null implementation */
  Source & Source::nullimpl()
  {
    if (! _nullimpl_initialized)
    {
      _nullimpl = SourceFactory().createFrom(source::SourceImpl::nullimpl());
      _nullimpl_initialized = true;
    }
    return _nullimpl;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //	Forward to SourceImpl:
  //
  ///////////////////////////////////////////////////////////////////

  const ResStore & Source::resolvables()
  { return _pimpl->resolvables(*this); }

  std::ostream & Source::dumpOn( std::ostream & str ) const
  { return _pimpl->dumpOn( str ); }

  const Pathname Source::provideFile(const Pathname & file_r,
				     const unsigned media_nr)
  { return _pimpl->provideFile(file_r, media_nr); }

  const Pathname Source::provideDir(const Pathname & dir_r,
				    const unsigned media_nr,
				    const bool recursive)
  { return _pimpl->provideDir(dir_r, media_nr, recursive); }

  const bool Source::enabled() const
  { return _pimpl->enabled(); }

  void Source::enable()
  { _pimpl->enable(); }

  void Source::disable()
  { _pimpl->disable(); }

  string Source::name (void) const
  { _pimpl->name(); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
