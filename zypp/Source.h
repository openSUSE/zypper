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

#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace source
  {
    class SourceImpl;
  }
  class ResStore;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Source
  //
  /**
   * \note Source is a reference to the implementation. No COW
   * is performed.
  */
  class Source_Ref
  {
  public:
    typedef source::SourceImpl  Impl;
    typedef intrusive_ptr<Impl> Impl_Ptr;

  public:

    /** All resolvables provided by this source. */
    const ResStore & resolvables();
    /** Null implementation */
    static Source_Ref & nullimpl();

  private:
    /** Factory */
    friend class SourceFactory;
    friend class SourceManager;
  private:
    /** Factory ctor */
    Source_Ref();
    /** Factory ctor */
    explicit
    Source_Ref( const Impl_Ptr & impl_r );

  private:
    friend std::ostream & operator<<( std::ostream & str, Source_Ref obj );
    /** Stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;

  private:
    /** Pointer to implementation */
    RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _pimpl;

    static Source_Ref _nullimpl;
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

    // for ZMD
    std::string alias (void) const;
    std::string zmdname (void) const;
    std::string zmddescription (void) const;
    unsigned priority (void) const;
    unsigned priority_unsubscribed (void) const;

    // for YaST
    std::string url (void) const;
    
    /** Conversion to bool to allow pointer style tests
     *  for nonNULL \ref resolvable. */
    operator bool() const
    { return _pimpl != NULL; }

  };
  ///////////////////////////////////////////////////////////////////

  typedef Source_Ref Source;

  ///////////////////////////////////////////////////////////////////
  /** \relates Source Stream output. */
  inline std::ostream & operator<<( std::ostream & str, Source_Ref obj )
  { return obj.dumpOn( str ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_H
