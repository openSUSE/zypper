/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/ResObjectImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_RESOBJECTIMPLIF_H
#define ZYPP_DETAIL_RESOBJECTIMPLIF_H

#include <list>
#include <string>

#include "zypp/detail/ResObjectFactory.h"
#include "zypp/ByteCount.h"
#include "zypp/Date.h"

#include "zypp/NeedAType.h" // volatile include propagating type drafts

// will be defined =0 later
#define PURE_VIRTUAL

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Resolvable;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ResObjectImplIf
    //
    /** Abstact ResObject implementation interface.
     * \todo We should rename the detail::*Impl classes, and classify
     * them into Dumb (holding no real data, provided the ImplIf dtor is
     * the only prure virtual) and FullStore (providing a protected variable
     * and interface methods returning them for each datum). The todo hook is
     * here, because it#s the common base of the *Impl classes.
    */
    class ResObjectImplIf
    {
    public:
      /** \name Common Attributes.
       * These should be provided by each kind of Resolvable. Call the
       * default implementation if you don't have a value for it.
       * \todo The UI likes to work on ResObject level, but some of
       * the values actually make no sense for several kinds of Resolvable,
       * or may have completely different semantics. See wheter we can get
       * rid of ome stuff.
       * \todo Some of these are actually tranlated or translatable.
       * offer some concept to express it.
       * \todo LICENSE HANDLING!
      */
      //@{
      /** Short label. */
      virtual Label summary() const PURE_VIRTUAL;

      /** Long description */
      virtual Text description() const PURE_VIRTUAL;

      /** \todo well define! */
      virtual Text insnotify() const PURE_VIRTUAL;

      /** \todo well define! */
      virtual Text delnotify() const PURE_VIRTUAL;

      /** Size.  \todo well define which size. */
      virtual ByteCount size() const PURE_VIRTUAL;

      /** Wheter there are src.rpm available too. */
      virtual bool providesSources() const PURE_VIRTUAL;

      /** \name deprecated
       * \todo These should be replaced by a offering a
       * Ptr to the Source.
      */
      //@{
      /** \deprecated */
      virtual Label instSrcLabel() const PURE_VIRTUAL;
      /** \deprecated */
      virtual Vendor instSrcVendor() const PURE_VIRTUAL;
      //@}

      //@}
    public:
      /** Ctor */
      ResObjectImplIf()
      : _backRef( 0 )
      {}
      /** Dtor. Makes this an abstract class. */
      virtual ~ResObjectImplIf() = 0;

      /** Test wheter \c this is already connected to Resolvable. */
      bool hasBackRef() const
      { return _backRef; }

    public:
      /** Access to Resolvable data if connected. */
      const Resolvable *const self() const
      { return _backRef; }
      /** Access to Resolvable data if connected. */
      Resolvable *const self()
      { return _backRef; }

    private:
      /** Manages _backRef when glued to a Resolvable. */
      template<class _Res>
        friend class _resobjectfactory_detail::ResImplConnect;
      /** Backlink to Resolvable.*/
      Resolvable * _backRef;
    };
    /////////////////////////////////////////////////////////////////

    /* Implementation of pure virtual dtor is required! */
    inline ResObjectImplIf::~ResObjectImplIf()
    {}

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_RESOBJECTIMPLIF_H
