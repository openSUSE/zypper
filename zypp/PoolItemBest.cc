/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PoolItemBest.cc
 *
*/
#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/PoolItemBest.h"
#include "zypp/ui/SelectableTraits.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PoolItemBest::Impl
  //
  /** PoolItemBest implementation. */
  struct PoolItemBest::Impl
  {
    Container _container;

    private:
      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const
      { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PoolItemBest
  //
  ///////////////////////////////////////////////////////////////////

  void PoolItemBest::_ctor_init()
  { _dont_use_this_use_pimpl.reset( new RWCOW_pointer<Impl>(new Impl) ); }

  const PoolItemBest::Container & PoolItemBest::container() const
  { return pimpl()->_container; }

  void PoolItemBest::add( const PoolItem & pi_r )
  {
    Container & container( pimpl()->_container );
    PoolItem & ccand( container[pi_r.satSolvable().ident()] );
    if ( ! ccand || ui::SelectableTraits::AVOrder()( pi_r, ccand ) )
      ccand = pi_r;
  }

  PoolItem PoolItemBest::find( IdString ident_r ) const
  {
    const Container & container( pimpl()->_container );
    Container::const_iterator it( container.find( ident_r ) );
    return it != container.end() ? it->second : PoolItem();
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const PoolItemBest & obj )
  {
    return dumpRange( str << "(" << obj.size() << ") ", obj.begin(), obj.end() );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
