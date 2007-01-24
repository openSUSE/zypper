/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ManagedFile.h
 *
*/
#ifndef ZYPP_MANAGEDFILE_H
#define ZYPP_MANAGEDFILE_H

#include <iosfwd>

#include "zypp/Pathname.h"
#include "zypp/AutoDispose.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** A Pathname plus associated cleanup code to be executed when
   *  path is no longer needed.
   */
  typedef AutoDispose<const Pathname> ManagedFile;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MANAGEDFILE_H
