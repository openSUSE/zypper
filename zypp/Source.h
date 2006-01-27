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
#include <string>

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
    const ResStore & resolvables();
    /** Null implementation */
    static Source & nullimpl();

  private:
    /** Factory */
    friend class SourceFactory;
    friend class SourceManager;
  private:
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

    static Source _nullimpl;
    static bool _nullimpl_initialized;

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

    std::string alias (void) const;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Source Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Source & obj )
  { return obj.dumpOn( str ); }

  /** \relates Source  */
  inline bool operator==( const Source & lhs, const Source & rhs )
  { return !lhs.alias().empty() && !rhs.alias().empty() && lhs.alias() == rhs.alias(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_H
