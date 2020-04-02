/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResolverFocus.cc
 */
#include <iostream>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/ResolverFocus.h>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  std::string asString( const ResolverFocus & val_r )
  {
    switch ( val_r )
    {
#define OUTS(V) case ResolverFocus::V: return #V; break
      OUTS( Default );
      OUTS( Job );
      OUTS( Installed );
      OUTS( Update );
#undef OUTS
    }
    // Oops!
    std::string ret { str::Str() << "ResolverFocus(" << static_cast<int>(val_r) << ")"  };
    WAR << "asString: dubious " << ret << endl;
    return ret;
  }

  bool fromString( const std::string & val_r, ResolverFocus & ret_r )
  {
    switch ( val_r[0] )
    {
#define OUTS(V) if ( ::strcasecmp( val_r.c_str(), #V ) == 0 ) { ret_r = ResolverFocus::V; return true; }
      case 'D':
      case 'd':
	OUTS( Default );
	break;

      case 'J':
      case 'j':
	OUTS( Job );
	break;

      case 'I':
      case 'i':
	OUTS( Installed );
	break;

      case 'U':
      case 'u':
	OUTS( Update );
	break;

      case '\0':
	ret_r = ResolverFocus::Default;
	return true;
	break;
#undef OUTS
    }
    // Oops!
    WAR << "fromString: unknown ResolverFocus '" << val_r << "'" << endl;
    return false;
  }
} // namespace zypp
///////////////////////////////////////////////////////////////////
