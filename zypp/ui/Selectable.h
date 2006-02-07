/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/Selectable.h
 *
*/
#ifndef ZYPP_UI_SELECTABLE_H
#define ZYPP_UI_SELECTABLE_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Iterator.h"

#include "zypp/ResObject.h"
#include "zypp/PoolItem.h"
#include "zypp/ui/Status.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    namespace ui_detail
    {
      /** Transform PoolItem to ResObject::constPtr. */
      struct TransformToResObjectPtr : public std::unary_function<PoolItem,ResObject::constPtr>
      {
        ResObject::constPtr operator()( const PoolItem & obj ) const
        { return obj; }
      };
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Selectable
    //
    /** Collects ResObject of same kind and name.
     *
     * \note There's one Selectable per installed item, in case more
     * than one item is intalled.
     *
     * \todo Make it a _Ref.
    */
    class Selectable : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const Selectable & obj );
      typedef std::set<PoolItem>               AvialableItemSet;

    public:
      typedef intrusive_ptr<Selectable>        Ptr;
      typedef intrusive_ptr<const Selectable>  constPtr;

      /** UI likes to iterate on ResObject::constPtr,
       * independent from what the implementation uses. */
      typedef transform_iterator<ui_detail::TransformToResObjectPtr,
                                 AvialableItemSet::const_iterator>
                                 available_iterator;
      typedef AvialableItemSet::size_type      size_type;

    public:
      /** The ResObjects kind. */
      ResObject::Kind kind() const;

      /** The ResObjects name.  */
      const std::string & name() const;

      /** Return the current Status */
      Status status() const;

      /** Try to set a new Status.
       * Returns \c false if the transitions is not allowed.
      */
      bool set_status( const Status state_r );

      /** Installed object. */
      ResObject::constPtr installedObj() const;

      /** Best among available objects. */
      ResObject::constPtr candidateObj() const;

      /** Best among all objects. */
      ResObject::constPtr theObj() const;

      /** Number of available objects. */
      size_type availableObjs() const;

      /** */
      available_iterator availableBegin() const;

      /** */
      available_iterator availableEnd() const;

    public:

      /** True if installed object is present. */
      bool hasInstalledObj() const
      { return installedObj(); }

      /** True if candidate object is present. */
      bool hasCandidateObj() const
      { return candidateObj(); }

    public:
      /** Implementation  */
      class Impl;
      typedef shared_ptr<Impl> Impl_Ptr;
      /** Default ctor */
      Selectable( Impl_Ptr pimpl_r );
    private:
      /** Dtor */
      ~Selectable();
    private:
      /** Pointer to implementation */
      RW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Selectable Stream output */
    std::ostream & operator<<( std::ostream & str, const Selectable & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELECTABLE_H
