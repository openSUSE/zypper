/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceFeed.h
 *
*/
#ifndef ZYPP_SOURCEFEED_H
#define ZYPP_SOURCEFEED_H

#include <iosfwd>
#include <set>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class ResPoolManager;
  class Source_Ref;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceFeed_Ref
  //
  /** Feed Sources into a ResPool.
   * \todo Clean up implementation.
  */
  class SourceFeed_Ref
  {
    friend std::ostream & operator<<( std::ostream & str, const SourceFeed_Ref & obj );
    typedef std::set<Source_Ref> ContainerT;

  public:
    /** Implementation  */
    class Impl;

    typedef ContainerT::size_type      size_type;
    typedef ContainerT::iterator       iterator;
    typedef ContainerT::const_iterator const_iterator;

  public:
    /** Default ctor */
    SourceFeed_Ref( ResPoolManager pool_r );
    /** Dtor */
    ~SourceFeed_Ref();

  public:

    /**  Add one Source. */
    void addSource( Source_Ref src_r )
    {
      ContainerT sources;
      sources.insert( src_r );
      insert( sources );
    }

    /** Add Sources from some container.  */
    template <class _InputIterator>
      void addSource( _InputIterator first_r, _InputIterator last_r )
      {
        ContainerT sources( first_r, last_r );
        insert( sources );
      }

    // Add defaults from sourcemanager

    // Set sources

  public:

    /** Remove a Source. */
    void removeSource( Source_Ref src_r )
    {
      ContainerT sources;
      sources.insert( src_r );
      erase( sources );
    }

    /** Remove all Sources mentioned in container. */
    template <class _InputIterator>
      void removeSource( _InputIterator first_r, _InputIterator last_r )
      {
        ContainerT sources( first_r, last_r );
        erase( sources );
      }

    /** Remove all Sources. */
    void removeAllSources()
    {
      removeSource( sourceBegin(), sourceEnd() );
    }

  public:

    /** Iterate and query */
    const_iterator sourceBegin() const;
    const_iterator sourceEnd() const;

  private:

    void insert( ContainerT & sources_r );

    void erase( ContainerT & sources_r );

  private:
    /** Pointer to implementation: _Ref */
    shared_ptr<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SourceFeed Stream output */
  std::ostream & operator<<( std::ostream & str, const SourceFeed_Ref & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCEFEED_H
