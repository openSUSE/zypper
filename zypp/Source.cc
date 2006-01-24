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

  ///////////////////////////////////////////////////////////////////
  //
  //	Forward to SourceImpl:
  //
  ///////////////////////////////////////////////////////////////////

  const ResStore & Source::resolvables() const
  { return _pimpl->resolvables(); }

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

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
