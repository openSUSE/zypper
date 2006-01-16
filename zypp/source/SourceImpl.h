/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/SourceImpl.h
 *
*/
#ifndef ZYPP_SOURCE_SOURCEIMPL_H
#define ZYPP_SOURCE_SOURCEIMPL_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/ResStore.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SourceImpl
    //
    /** Base class for concrete Source implementations.
     *
     * Constructed by \ref SourceFactory. Public access via \ref Source
     * interface.
    */
    class SourceImpl : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const SourceImpl & obj );

    public:
      /** Ctor. */
      SourceImpl();
      /** Dtor. */
      virtual ~SourceImpl();

    public:

      /** All resolvables provided by this source. */
      const ResStore & resolvables() const
      { return _store; }

      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const
      { return str << "SourceImpl"; }

    protected:
      /** All resolvables provided by this source. */
      ResStore _store;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates SourceImpl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const SourceImpl & obj )
    { return obj.dumpOn( str ); }

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SOURCEIMPL_H
