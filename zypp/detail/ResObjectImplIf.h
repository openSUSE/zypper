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

#include "zypp/base/Deprecated.h"
#include "zypp/detail/ResImplTraits.h"
#include "zypp/detail/ResObjectFactory.h"
#include "zypp/Repository.h"
#include "zypp/Locale.h"
#include "zypp/ByteCount.h"
#include "zypp/Date.h"
#include "zypp/TranslatedText.h"

#include "zypp/NeedAType.h" // volatile include propagating type drafts

// will be defined =0 later
#define PURE_VIRTUAL

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Resolvable;
  class Repository;
  class DiskUsage;

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
    class ResObjectImplIf : public base::ReferenceCounted, private base::NonCopyable
    {
    public:
      /** \name Common Attributes.
       * These should be provided by each kind of Resolvable. Call the
       * default implementation if you don't have a value for it.
      */
      //@{
      /** Short label. */
      virtual TranslatedText summary() const PURE_VIRTUAL;

      /** Long description */
      virtual TranslatedText description() const PURE_VIRTUAL;

      /** \todo well define! */
      virtual TranslatedText insnotify() const PURE_VIRTUAL;

      /** \todo well define! */
      virtual TranslatedText delnotify() const PURE_VIRTUAL;

      /** */
      virtual TranslatedText licenseToConfirm() const PURE_VIRTUAL;

      /** */
      virtual Vendor vendor() const PURE_VIRTUAL;

      /** Size.  \todo well define which size. */
      virtual ByteCount size() const PURE_VIRTUAL;

      /** */
      virtual ByteCount downloadSize() const PURE_VIRTUAL;

      /** Backlink to the source providing this. */
      virtual Repository repository() const PURE_VIRTUAL;

      /** Number of the source media that provides the data
       *  required for installation. Zero, if no media access
       *  is required.
      */
      virtual unsigned mediaNr() const PURE_VIRTUAL;

      /** */
      virtual bool installOnly() const PURE_VIRTUAL;

      /** */
      virtual Date buildtime() const;

      /** Time of installation, or \c 0 */
      virtual Date installtime() const;

      /** Disk usage per directory
       * A common attribute, although mostly packages require
       * noticeable disk space. An e.g product could try to reserve
       * a certain ammount of diskspace by providing DiskUsage data.
       */
      virtual const DiskUsage & diskusage() const;
      //@}

    public:
      /** Ctor */
      ResObjectImplIf()
      : _backRef( 0 )
      {}
      /** Dtor. Makes this an abstract class. */
      virtual ~ResObjectImplIf() = 0;

    public:
      /** Test whether \c this is already connected to Resolvable. */
      bool hasBackRef() const
      { return _backRef; }

      /** Access to Resolvable data if connected. */
      const Resolvable * self() const
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
