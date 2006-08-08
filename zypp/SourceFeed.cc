/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceFeed.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/ResPoolManager.h"
#include "zypp/SourceFeed.h"
#include "zypp/ResStore.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceFeed_Ref::Impl
  //
  /** SourceFeed implementation. */
  struct SourceFeed_Ref::Impl
  {


    Impl(  ResPoolManager pool_r )
    : _pool( pool_r )
    {}

#if 0
    void addSource( Source_Ref src_r );
    void removeSource( Source_Ref src_r );
#endif

    const_iterator sourceBegin() const
    { return _sources.begin(); }

    const_iterator sourceEnd() const
    { return _sources.end(); }

    void insert( ContainerT & sources_r )
    {
      for ( const_iterator it = sources_r.begin(); it != sources_r.end(); ++it )
        {
          _sources.insert( *it );
          _pool.insert( it->resolvables().begin(), it->resolvables().end() );
        }
    }

    void erase( ContainerT & sources_r )
    {
      for ( const_iterator it = sources_r.begin(); it != sources_r.end(); ++it )
        {
          //_pool.erase( it->resolvables().begin(), it->resolvables().end() );
          _sources.erase( *it );
        }
    }

    /** Pool to feed. */
    ResPoolManager _pool;

    ContainerT _sources;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SourceFeed::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const SourceFeed_Ref::Impl & obj )
  {
    return str << "SourceFeed::Impl";
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceFeed_Ref
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceFeed_Ref::SourceFeed_Ref
  //	METHOD TYPE : Ctor
  //
  SourceFeed_Ref::SourceFeed_Ref( ResPoolManager pool_r )
  : _pimpl( new Impl( pool_r ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceFeed_Ref::~SourceFeed_Ref
  //	METHOD TYPE : Dtor
  //
  SourceFeed_Ref::~SourceFeed_Ref()
  {}

  ///////////////////////////////////////////////////////////////////
#if 0
  void SourceFeed_Ref::addSource( Source_Ref src_r )
  { _pimpl->addSource( src_r ); }

  void SourceFeed_Ref::removeSource( Source_Ref src_r )
  { _pimpl->removeSource( src_r ); }

#endif

  void SourceFeed_Ref::insert( ContainerT & sources_r )
  { _pimpl->insert( sources_r ); }

  void SourceFeed_Ref::erase( ContainerT & sources_r )
  { _pimpl->erase( sources_r ); }

  SourceFeed_Ref::const_iterator SourceFeed_Ref::sourceBegin() const
  { return _pimpl->sourceBegin(); }

  SourceFeed_Ref::const_iterator SourceFeed_Ref::sourceEnd() const
  { return _pimpl->sourceEnd(); }

  ///////////////////////////////////////////////////////////////////

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const SourceFeed_Ref & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
