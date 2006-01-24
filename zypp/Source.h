/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Source.h
 *
*/
#ifndef ZYPP_SOURCE_H
#define ZYPP_SOURCE_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/ResStore.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace source
  {
    class SourceImpl;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Source
  //
  /**
   * \note Source is a reference to the implementation. No COW
   * is performed.
  */
  class Source
  {
  public:
    typedef source::SourceImpl  Impl;
    typedef intrusive_ptr<Impl> Impl_Ptr;

  public:

    /** All resolvables provided by this source. */
    const ResStore & resolvables() const;

  private:
    /** Factory */
    friend class SourceFactory;
    friend class SourceManager;
    /** Factory ctor */
    Source();
    /** Factory ctor */
    explicit
    Source( const Impl_Ptr & impl_r );

  private:
    friend std::ostream & operator<<( std::ostream & str, const Source & obj );
    /** Stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;

  private:
    /** Pointer to implementation */
    RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _pimpl;

  public:
    /** Provide a file to local filesystem */
    const Pathname provideFile(const Pathname & file_r,
			       const unsigned media_nr = 1);
    const Pathname provideDir(const Pathname & dir_r,
		              const unsigned media_nr = 1,
			      const bool recursive = false);

    const bool enabled() const;

    void enable();

    void disable();
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Source Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Source & obj )
  { return obj.dumpOn( str ); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_H
