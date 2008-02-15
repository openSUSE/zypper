/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/NameKindProxy.h
 *
*/
#ifndef ZYPP_NAMEKINDPROXY_H
#define ZYPP_NAMEKINDPROXY_H

#include <iosfwd>
#include <set>

#include "zypp/ResPool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace name_kind_proxy_details
  {
    struct IOrder : public std::binary_function<PoolItem,PoolItem,bool>
    {
      // NOTE: operator() provides LESS semantics to order the set.
      // So LESS means 'prior in set'. We want the last item installed
      // first.
      /** \todo let ResObject provide installtime */
      bool operator()( const PoolItem & lhs, const PoolItem & rhs ) const
      {
        // top should be installtime!

        int res = lhs->arch().compare( rhs->arch() );
        if ( res )
          return res > 0;
        res = lhs->edition().compare( rhs->edition() );
        if ( res )
          return res > 0;

        // no more criteria, still equal:
        // use the ResObject::constPtr (the poiner value)
        // (here it's arbitrary whether < or > )
        return lhs.resolvable() < rhs.resolvable();
      }
    };

    struct AOrder : public std::binary_function<PoolItem,PoolItem,bool>
    {
      // NOTE: operator() provides LESS semantics to order the set.
      // So LESS means 'prior in set'. We want 'better' archs and
      // 'better' editions at the beginning of the set. So we return
      // TRUE if (lhs > rhs)!
      //
      bool operator()( const PoolItem & lhs, const PoolItem & rhs ) const
      {
        int res = lhs->arch().compare( rhs->arch() );
        if ( res )
          return res > 0;
        res = lhs->edition().compare( rhs->edition() );
        if ( res )
          return res > 0;

        // no more criteria, still equal:
        // use the ResObject::constPtr (the poiner value)
        // (here it's arbitrary whether < or > )
        return lhs.resolvable() < rhs.resolvable();
      }
    };
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : NameKindProxy
  //
  /** Retrieve and maintain PoolItem of a certain name and kind.
   *
   * Installed PoolItems are sorted according to their installtime
   * (last installed first).
   *
   * Available PoolItems are sorted 'best first'.
   *
   * \todo provide status query and manipulation methods
  */
  class NameKindProxy
  {
    typedef name_kind_proxy_details::IOrder IOrder;
    typedef name_kind_proxy_details::AOrder AOrder;
  public:
    typedef std::set<PoolItem,IOrder>    InstalledSet;
    typedef InstalledSet::iterator       Installed_iterator;
    typedef InstalledSet::const_iterator Installed_const_iterator;
    typedef InstalledSet::size_type      Installed_size_type;

    typedef std::set<PoolItem,AOrder>    AvailableSet;
    typedef AvailableSet::iterator       Available_iterator;
    typedef AvailableSet::const_iterator Available_const_iterator;
    typedef AvailableSet::size_type      Available_size_type;

  public:
    NameKindProxy( ResPool pool_r, IdString name_r, ResKind kind_r );
    NameKindProxy( ResPool pool_r, const C_Str & name_r, ResKind kind_r );

  public:
    ResKind kind() const
    { return _kind; }

    IdString name() const
    { return _name; }

  public:
    Installed_size_type installedSize() const
    { return _installed.size(); }

    bool installedEmpty() const
    { return _installed.empty(); }

    Installed_const_iterator installedBegin() const
    { return _installed.begin(); }

    Installed_const_iterator installedEnd() const
    { return _installed.end(); }

  public:
    Available_size_type availableSize() const
    { return _available.size(); }

    bool availableEmpty() const
    { return _available.empty(); }

    Available_const_iterator availableBegin() const
    { return _available.begin(); }

    Available_const_iterator availableEnd() const
    { return _available.end(); }

  public:

    // status query and manip stuff...

  private:
    ResKind      _kind;
    IdString     _name;
    InstalledSet _installed;
    AvailableSet _available;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates NameKindProxy Stream output. */
  std::ostream & operator<<( std::ostream & str, const NameKindProxy & obj );

  /** \relates NameKindProxy Convenience construction. */
  template<class _Res>
    inline NameKindProxy nameKindProxy( ResPool pool_r, IdString name_r )
    { return NameKindProxy( pool_r, name_r, ResTraits<_Res>::kind ); }
  template<class _Res>
    inline NameKindProxy nameKindProxy( ResPool pool_r, const C_Str & name_r )
    { return NameKindProxy( pool_r, name_r, ResTraits<_Res>::kind ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_NAMEKINDPROXY_H
