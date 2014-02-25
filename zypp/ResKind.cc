/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResKind.cc
 *
*/
#include <iostream>

#include "zypp/base/String.h"

#include "zypp/ResKind.h"
#include "zypp/ResTraits.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  const ResKind ResKind::nokind;
  const ResKind ResKind::package	( "package" );
  const ResKind ResKind::patch		( "patch" );
  const ResKind ResKind::pattern	( "pattern" );
  const ResKind ResKind::product	( "product" );
  const ResKind ResKind::srcpackage	( "srcpackage" );
  const ResKind ResKind::application	( "application" );

  template<>
    const ResKind ResTraits<Package>	::kind( ResKind::package );
  template<>
    const ResKind ResTraits<Patch>	::kind( ResKind::patch );
  template<>
    const ResKind ResTraits<Pattern>	::kind( ResKind::pattern );
  template<>
    const ResKind ResTraits<Product>	::kind( ResKind::product );
  template<>
    const ResKind ResTraits<SrcPackage>	::kind( ResKind::srcpackage );
  template<>
    const ResKind ResTraits<Application>::kind( ResKind::application );

  ResKind ResKind::explicitBuiltin( const char * str_r )
  {
    if ( str_r && str_r[0] && str_r[1] && str_r[2] )
    {
      switch ( str_r[3] )
      {
	// NOTE: it needs to be assertd that the separating ':' is present
	// if a known kind is retuirned. Dependent code relies on this!
	#define OUTS(K,S) if ( !::strncmp( str_r, ResKind::K.c_str(), S ) && str_r[S] == ':' ) return ResKind::K
	//             ----v
	case 'c': OUTS( patch, 5 );       break;
	case 'd': OUTS( product, 7 );     break;
	case 'k': OUTS( package, 7 );     break;
	case 'l': OUTS( application, 11 );break;
	case 'p': OUTS( srcpackage, 10 ); break;
	case 't': OUTS( pattern, 7 );     break;
	#undef OUTS
      }
    }
    return nokind;
  }

  std::string ResKind::satIdent( const ResKind & refers_r, const std::string & name_r )
  {
    if ( ! refers_r || refers_r == package || refers_r == srcpackage )
      return name_r;
    return str::form( "%s:%s", refers_r.c_str(), name_r.c_str() );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
