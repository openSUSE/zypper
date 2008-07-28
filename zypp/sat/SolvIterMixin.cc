/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/SolvIterMixin.cc
 *
*/
//#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/sat/SolvIterMixin.h"
#include "zypp/sat/Solvable.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/pool/PoolTraits.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    namespace solvitermixin_detail
    {
      bool UnifyByIdent::operator()( const Solvable & solv_r ) const
      {
        // Need to use pool::ByIdent because packages and srcpackages have the same id.
        return( solv_r && _uset->insert( pool::ByIdent( solv_r ).get() ).second );
      }
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    Selectable_Ptr asSelectable::operator()( const sat::Solvable & sov_r ) const
    {
      return ResPool::instance().proxy().lookup( sov_r );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
