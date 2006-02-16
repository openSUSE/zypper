/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYppCommitResult.h
 *
*/
#ifndef ZYPP_ZYPPCOMMITRESULT_H
#define ZYPP_ZYPPCOMMITRESULT_H

#include <iosfwd>
#include <list>

#include "zypp/ZYpp.h"
#include "zypp/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYpp::CommitResult
  //
  /** Result returned from ZYpp::commit.
   * \see \ref ZYpp::commit
   * \todo document fields.
  */
  struct ZYpp::CommitResult
  {
    typedef std::list<PoolItem_Ref> PoolItemList;

    int          _result;
    PoolItemList _errors;
    PoolItemList _remaining;
    PoolItemList _srcremaining;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPPCOMMITRESULT_H
