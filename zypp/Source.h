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

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/ResStore.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  DEFINE_PTR_TYPE(Source)
  namespace source
  {
    class SourceImpl;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Source
  //
  /**
   *
  */
  class Source : public base::ReferenceCounted, private base::NonCopyable
  {
  public:
    typedef source::SourceImpl Impl;
    typedef shared_ptr<Impl>   Impl_Ptr;

  public:

    /** All resolvables provided by this source. */
    const ResStore & resolvables() const;

  private:
    /** Factory */
    friend class SourceFactory;
    /** Factory ctor */
    explicit
    Source( Impl_Ptr impl_r );

  private:
    friend std::ostream & operator<<( std::ostream & str, const Source & obj );
    /** Stream output. */
    std::ostream & dumpOn( std::ostream & str ) const;

  public:
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Source Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Source & obj )
  { return obj.dumpOn( str ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_H
