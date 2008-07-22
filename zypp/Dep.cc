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
#include "zypp/base/String.h"

#include "zypp/Dep.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {

    std::map<std::string,Dep::for_use_in_switch> _table;

    Dep::for_use_in_switch parse( const std::string & strval_r )
    {
      if ( _table.empty() )
        {
          // initialize it
          _table["provides"]    = Dep::PROVIDES_e;
          _table["prerequires"] = Dep::PREREQUIRES_e;
          _table["requires"]    = Dep::REQUIRES_e;
          _table["conflicts"]   = Dep::CONFLICTS_e;
          _table["obsoletes"]   = Dep::OBSOLETES_e;
          _table["recommends"]  = Dep::RECOMMENDS_e;
          _table["suggests"]    = Dep::SUGGESTS_e;
          _table["enhances"]    = Dep::ENHANCES_e;
          _table["supplements"]	= Dep::SUPPLEMENTS_e;
        }

      std::map<std::string,Dep::for_use_in_switch>::const_iterator it
      = _table.find( str::toLower( strval_r ) );
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
    static std::map<for_use_in_switch,std::string> _table;
    if ( _table.empty() )
      {
        // initialize it
        _table[PROVIDES_e]    = "provides";
        _table[PREREQUIRES_e] = "prerequires";
        _table[REQUIRES_e]    = "requires";
        _table[CONFLICTS_e]   = "conflicts";
        _table[OBSOLETES_e]   = "obsoletes";
        _table[RECOMMENDS_e]  = "recommends";
        _table[SUGGESTS_e]    = "suggests";
        _table[ENHANCES_e]    = "enhances";
        _table[SUPPLEMENTS_e] = "supplements";
      }
    return _table[_type];
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
