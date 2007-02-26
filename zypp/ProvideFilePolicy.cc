/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <sstream>
#include "zypp/base/Logger.h"

#include "zypp/ProvideFilePolicy.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ProvideFilePolicy
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    bool yes() { return true; }
    bool no()  { return false; }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ProvideFilePolicy & ProvideFilePolicy::failOnChecksumErrorCB( bool yesno_r )
  {
    _failOnChecksumErrorCB = (yesno_r ? &yes : &no);
    return *this;
  }

  bool ProvideFilePolicy::progress( int value ) const
  {
    if ( _progressCB )
      return _progressCB( value );
    return true;
  }

  bool ProvideFilePolicy::failOnChecksumError() const
  {
    if ( _failOnChecksumErrorCB )
      return _failOnChecksumErrorCB();
    return true;
  }

} // namespace zypp
///////////////////////////////////////////////////////////////////
