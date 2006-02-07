/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/ResPoolProxy.h
 *
*/
#ifndef ZYPP_UI_RESPOOLPROXY_H
#define ZYPP_UI_RESPOOLPROXY_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/ResPool.h"
#include "zypp/ui/Selectable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ResPoolProxy
    //
    /**
     * \todo Make it a _Ref.
    */
    class ResPoolProxy
    {
      friend std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj );

      typedef std::set<Selectable::Ptr>                 SelectableIndex;
      typedef std::map<ResObject::Kind,SelectableIndex> SelectablePool;

    public:
      /** Implementation  */
      class Impl;

      typedef SelectableIndex::iterator       iterator;
      typedef SelectableIndex::const_iterator const_iterator;
      typedef SelectableIndex::size_type      size_type;

    public:
      /** Default ctor: no pool */
      ResPoolProxy();
      /** Ctor */
      ResPoolProxy( ResPool_Ref pool_r );
      /** Dtor */
      ~ResPoolProxy();

    public:

      /**  */
      bool empty( const ResObject::Kind & kind_r ) const;

      template<class _Res>
        bool empty() const
        { return empty( ResTraits<_Res>::kind ); }

      /**  */
      size_type size( const ResObject::Kind & kind_r ) const;

      template<class _Res>
        size_type size() const
        { return size( ResTraits<_Res>::kind ); }

      /** \name Iterate through all Selectables of a certain kind. */
      //@{
      const_iterator byKindBegin( const ResObject::Kind & kind_r ) const;

      template<class _Res>
        const_iterator byKindBegin() const
        { return byKindBegin( ResTraits<_Res>::kind ); }


      const_iterator byKindEnd( const ResObject::Kind & kind_r ) const;

      template<class _Res>
        const_iterator byKindEnd() const
        { return byKindEnd( ResTraits<_Res>::kind ); }
      //@}

    private:
      /** Pointer to implementation */
      RW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ResPoolProxy Stream output */
    std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_RESPOOLPROXY_H
