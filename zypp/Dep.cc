/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Dep.cc
 *
*/
#include <map>
#include <iostream>

#include "zypp/base/Exception.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"

#include "zypp/Dep.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    inline Dep::for_use_in_switch parse( const std::string & strval_r )
    {
      const std::map<std::string,Dep::for_use_in_switch> _table = {
	{ "provides",		Dep::PROVIDES_e		},
	{ "prerequires",	Dep::PREREQUIRES_e	},
	{ "requires",		Dep::REQUIRES_e		},
	{ "conflicts",		Dep::CONFLICTS_e	},
	{ "obsoletes",		Dep::OBSOLETES_e	},
	{ "recommends",		Dep::RECOMMENDS_e	},
	{ "suggests",		Dep::SUGGESTS_e		},
	{ "enhances",		Dep::ENHANCES_e		},
	{ "supplements",	Dep::SUPPLEMENTS_e	}
      };

      auto it = _table.find( str::toLower( strval_r ) );
      if ( it == _table.end() )
      {
	ZYPP_THROW( Exception("Dep parse: illegal string value '"+strval_r+"'") );
      }
      return it->second;
    }
  }

  ///////////////////////////////////////////////////////////////////

  const Dep Dep::PROVIDES   ( Dep::PROVIDES_e );
  const Dep Dep::PREREQUIRES( Dep::PREREQUIRES_e );
  const Dep Dep::REQUIRES   ( Dep::REQUIRES_e );
  const Dep Dep::CONFLICTS  ( Dep::CONFLICTS_e );
  const Dep Dep::OBSOLETES  ( Dep::OBSOLETES_e );
  const Dep Dep::RECOMMENDS ( Dep::RECOMMENDS_e );
  const Dep Dep::SUGGESTS   ( Dep::SUGGESTS_e );
  const Dep Dep::ENHANCES   ( Dep::ENHANCES_e );
  const Dep Dep::SUPPLEMENTS( Dep::SUPPLEMENTS_e );

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Dep::Dep
  //	METHOD TYPE : Ctor
  //
  Dep::Dep( const std::string & strval_r )
  : _type( parse( strval_r ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Dep::asString
  //	METHOD TYPE : const std::string &
  //
  const std::string & Dep::asString() const
  {
    static const std::map<for_use_in_switch,std::string> _table = {
      { PROVIDES_e,	"provides"	},
      { PREREQUIRES_e,	"prerequires"	},
      { REQUIRES_e,	"requires"	},
      { CONFLICTS_e,	"conflicts"	},
      { OBSOLETES_e,	"obsoletes"	},
      { RECOMMENDS_e,	"recommends"	},
      { SUGGESTS_e,	"suggests"	},
      { ENHANCES_e,	"enhances"	},
      { SUPPLEMENTS_e,	"supplements"	}
    };
    return _table.at(_type);
  }

  std::string Dep::asUserString() const
  {
    switch ( inSwitch() )
    {
      case PROVIDES_e:		return _("Provides");		break;
      case PREREQUIRES_e:	return _("Prerequires");	break;
      case REQUIRES_e:		return _("Requires");		break;
      case CONFLICTS_e:		return _("Conflicts");		break;
      case OBSOLETES_e:		return _("Obsoletes");		break;
      case RECOMMENDS_e:	return _("Recommends");		break;
      case SUGGESTS_e:		return _("Suggests");		break;
      case ENHANCES_e:		return _("Enhances");		break;
      case SUPPLEMENTS_e:	return _("Supplements");	break;
    }
    return "<missing translation>";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
