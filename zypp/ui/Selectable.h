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

#include "zypp/ResObject.h"
#include "zypp/ui/Status.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Selectable
    //
    /**
     * \note There's one Selectable per installed item, in case more
     * than one item is intalled.
     *
     * \todo Make it a _Ref.
    */
    class Selectable : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const Selectable & obj );

    public:
      typedef intrusive_ptr<Selectable>        Ptr;
      typedef intrusive_ptr<const Selectable>  constPtr;
      typedef unsigned                         size_type;

      typedef ResObject                        Object;
      typedef Object::constPtr                 Object_Ptr;

    public:
      /**  */
      Object::Kind kind() const;

      /**  */
      const std::string & name() const;

      /**  */
      Status status() const;

      /**  */
      bool set_status( const Status state_r );

      /** Installed object. */
      Object_Ptr installedObj() const;

      /** Best among available objects. */
      Object_Ptr candidateObj() const;

      /** Best among all objects. */
      Object_Ptr theObj() const;

      /** Number of available objects. */
      size_type availableObjs() const;

      // iterators

    public:
      /** Implementation  */
      class Impl;
      typedef shared_ptr<Impl> Impl_Ptr;
      /** Default ctor */
      Selectable( Impl_Ptr pimpl_r );
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
