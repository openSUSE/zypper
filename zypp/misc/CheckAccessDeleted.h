/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/misc/CheckAccessDeleted.h
 *
*/
#ifndef ZYPP_MISC_CHECKACCESSDELETED_H
#define ZYPP_MISC_CHECKACCESSDELETED_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace misc
  { /////////////////////////////////////////////////////////////////

    /**
     * Check for running programms which access deleted files or libraries.
     *
     * Executed after commit, this gives a hint which programms/services
     * need to be restarted.
     */

    /////////////////////////////////////////////////////////////////
  } // namespace misc
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MISC_CHECKACCESSDELETED_H
