/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/solver/detail/ItemCapKind.h
 *
*/

#ifndef ZYPP_SOLVER_DETAIL_ITEMCAPKIND_H
#define ZYPP_SOLVER_DETAIL_ITEMCAPKIND_H
#ifndef ZYPP_USE_RESOLVER_INTERNALS
#error Do not directly include this file!
#else

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace solver
  {
    ///////////////////////////////////////////////////////////////////
    namespace detail
    {
      ///////////////////////////////////////////////////////////////////
      /// \class ItemCapKind
      ///////////////////////////////////////////////////////////////////
      struct ItemCapKind
      {
      public:
	ItemCapKind() : _pimpl( new Impl ) {}

	ItemCapKind( PoolItem i, Capability c, Dep k, bool initial ) : _pimpl( new Impl( i, c, k, initial ) ) {}

	/** Capability which has triggerd this selection */
	Capability cap() const
	{ return _pimpl->_cap; }

	/** Kind of that capability */
	Dep capKind() const
	{ return _pimpl->_capKind; }

	/** Item which has triggered this selection */
	PoolItem item() const
	{ return _pimpl->_item; }

	/** This item has triggered the installation (Not already fullfilled requierement only). */
	bool initialInstallation() const
	{ return _pimpl->_initialInstallation; }

      private:
	struct Impl
	{
	  Impl()
	  : _capKind( Dep::PROVIDES )
	  , _initialInstallation( false )
	  {}

	  Impl( PoolItem i, Capability c, Dep k, bool initial )
	  : _cap( c )
	  , _capKind( k )
	  , _item( i )
	  , _initialInstallation( initial )
	  {}

	  Capability	_cap;
	  Dep		_capKind;
	  PoolItem	_item;
	  bool		_initialInstallation;

	private:
	  friend Impl * rwcowClone<Impl>( const Impl * rhs );
	  /** clone for RWCOW_pointer */
	  Impl * clone() const
	  { return new Impl( *this ); }
	};
	RWCOW_pointer<Impl> _pimpl;
      };

      typedef std::multimap<PoolItem,ItemCapKind> ItemCapKindMap;
      typedef std::list<ItemCapKind> ItemCapKindList;

    } // namespace detail
    ///////////////////////////////////////////////////////////////////
  } // namespace solver
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_USE_RESOLVER_INTERNALS
#endif // ZYPP_SOLVER_DETAIL_ITEMCAPKIND_H
