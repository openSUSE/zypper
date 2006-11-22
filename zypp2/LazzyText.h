/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/LazzyText.h
 *
*/
#ifndef ZYPP_LazzyText_H
#define ZYPP_LazzyText_H

#include <iosfwd>
#include <map>
#include <list>
#include <set>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : LazzyText
  //
  class LazzyText
  {
    friend std::ostream & operator<<( std::ostream & str, const LazzyText & obj );
  public:
    /** Implementation  */
    class Impl;

  public:
    LazzyText();
    LazzyText( const Pathname &path, long int start, long int offset );
    LazzyText( const std::string &value );
    ~LazzyText();
    std::string text() const;
    bool empty() const;
    std::string asString() const;
  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates LazzyText Stream output */
  inline std::ostream & operator<<( std::ostream & str, const LazzyText & obj )
  { return str << obj.asString(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_LazzyText_H
