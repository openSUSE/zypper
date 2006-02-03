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
#include "zypp/base/Logger.h"

#include "zypp/Pathname.h"
#include "zypp/Url.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace source
  {
    class SourceImpl;
    DEFINE_PTR_TYPE(SourceImpl);
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
    friend bool operator==( const Source_Ref & lhs, const Source_Ref & rhs );
    friend bool operator<( const Source_Ref & lhs, const Source_Ref & rhs );

  public:
    typedef source::SourceImpl     Impl;
    typedef source::SourceImpl_Ptr Impl_Ptr;

  public:

    /** Default ctor: noSource.
     * Real Sources are to be created via SourceFactory.
    */
    Source_Ref();

    /** A dummy Source providing nothing, doing nothing.
     * \todo provide a _constRef
    */
    static const Source_Ref noSource;

  public:

    /** All resolvables provided by this source. */
    const ResStore & resolvables() const;

    /** Provide a file to local filesystem */
    const Pathname provideFile(const Pathname & file_r,
			       const unsigned media_nr = 1);
    const Pathname provideDir(const Pathname & dir_r,
		              const unsigned media_nr = 1,
			      const bool recursive = false);

    const bool enabled() const;

    void enable();

    void disable();

    void storeMetadata(const Pathname & cache_dir_r);

    // for ZMD
    std::string alias (void) const;
    std::string zmdname (void) const;
    std::string zmddescription (void) const;
    unsigned priority (void) const;
    unsigned priority_unsubscribed (void) const;

    // for YaST
    Url url (void) const;
    const Pathname & path (void) const;

  public:
    /** Conversion to bool to allow pointer style tests
     *  for nonNULL \ref source impl.
     * \todo fix by providing a safebool basecalss, doing the 'nasty'
     * things.
    */
    // see http://www.c-plusplus.de/forum/viewtopic-var-t-is-113762-and-start-is-0-and-postdays-is-0-and-postorder-is-asc-and-highlight-is-.html
    // for the glory details

    typedef void (Source_Ref::*unspecified_bool_type)();

    operator unspecified_bool_type() const
    {
      if ( *this == noSource )
        return static_cast<unspecified_bool_type>(0);
      return &Source_Ref::enable;	// return pointer to a void() function since the typedef is like this
    }

  private:
    /** Factory */
    friend class SourceFactory;
    friend class SourceManager;

  private:
    /** Factory ctor */
    explicit
    Source_Ref( const Impl_Ptr & impl_r );

  private:
    friend std::ostream & operator<<( std::ostream & str, Source_Ref obj );
    /** Stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;

  private:
    /** Pointer to implementation */
    Impl_Ptr _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Source Stream output. */
  inline std::ostream & operator<<( std::ostream & str, Source_Ref obj )
  { return obj.dumpOn( str ); }

  /** \relates Source_Ref Equal if same implementation class. */
  inline bool operator==( const Source_Ref & lhs, const Source_Ref & rhs )
  { return lhs._pimpl == rhs._pimpl; }

  /** \relates Source_Ref Order in std::conainer based on _pimpl. */
  inline bool operator<( const Source_Ref & lhs, const Source_Ref & rhs )
  { return lhs._pimpl < rhs._pimpl; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_H
