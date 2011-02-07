/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/DownloadMode.cc
 *
*/
#include <iostream>

#include "zypp/base/String.h"
#include "zypp/DownloadMode.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  bool deserialize( const std::string & str_r, DownloadMode & result_r )
  {
#define OUTS(VAL) if ( str::compareCI( str_r, #VAL ) == 0 ) { result_r = VAL; return true; }
    OUTS( DownloadOnly );
    OUTS( DownloadInAdvance );
    OUTS( DownloadInHeaps );
    OUTS( DownloadAsNeeded );
#undef OUTS
    return false;
  }

  std::ostream & operator<<( std::ostream & str, DownloadMode obj )
  {
    switch ( obj )
    {
#define OUTS(VAL) case VAL: return str << #VAL; break
      OUTS( DownloadDefault );
      OUTS( DownloadOnly );
      OUTS( DownloadInAdvance );
      OUTS( DownloadInHeaps );
      OUTS( DownloadAsNeeded );
#undef OUTS
    }
    return str << "DownloadMode(" << int(obj) << ")";
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
