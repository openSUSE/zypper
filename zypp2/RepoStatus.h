/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoStatus.h
 *
*/
#ifndef ZYPP2_REPOSTATUS_H
#define ZYPP2_REPOSTATUS_H

#include <iosfwd>
#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoStatus
  //
  /** */
  class RepoStatus
  {
    friend std::ostream & operator<<( std::ostream & str, const RepoStatus & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor */
    RepoStatus();
    /** Dtor */
    ~RepoStatus();

  public:

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoStatus Stream output */
  std::ostream & operator<<( std::ostream & str, const RepoStatus & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp2
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_REPOSTATUS_H
