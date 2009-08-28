/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Rel.cc
 *
*/
#include <iostream>
#include <map>

#include "zypp/base/Exception.h"

#include "zypp/Rel.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    std::map<std::string,Rel::for_use_in_switch> _table;

    std::map<std::string,Rel::for_use_in_switch>::const_iterator findStr( const std::string & strval_r )
    {
      if ( _table.empty() )
      {
        // initialize it
        _table["EQ"]   = _table["eq"]  = _table["=="]    = _table["="] = Rel::EQ_e;
        _table["NE"]   = _table["ne"]  = _table["!="]                  = Rel::NE_e;
        _table["LT"]   = _table["lt"]  = _table["<"]                   = Rel::LT_e;
        _table["LE"]   = _table["le"]  = _table["lte"]  = _table["<="] = Rel::LE_e;
        _table["GT"]   = _table["gt"]  = _table[">"]                   = Rel::GT_e;
        _table["GE"]   = _table["ge"]  = _table["gte"]  = _table[">="] = Rel::GE_e;
        _table["ANY"]  = _table["any"] = _table["(any)"] = _table[""]  = Rel::ANY_e;
        _table["NONE"] = _table["none"]                                = Rel::NONE_e;
      }

      return _table.find( strval_r );
    }

    Rel::for_use_in_switch parse( const std::string & strval_r )
    {
      std::map<std::string,Rel::for_use_in_switch>::const_iterator it = findStr( strval_r );
      if ( it == _table.end() )
      {
        ZYPP_THROW( Exception("Rel parse: illegal string value '"+strval_r+"'") );
      }
      return it->second;
    }

    Rel::for_use_in_switch parse( const std::string & strval_r, const Rel & default_r )
    {
      std::map<std::string,Rel::for_use_in_switch>::const_iterator it = findStr( strval_r );
      if ( it == _table.end() )
      {
        return default_r.inSwitch();
      }
      return it->second;
    }
  }
  ///////////////////////////////////////////////////////////////////

  const Rel Rel::EQ( Rel::EQ_e );
  const Rel Rel::NE( Rel::NE_e );
  const Rel Rel::LT( Rel::LT_e );
  const Rel Rel::LE( Rel::LE_e );
  const Rel Rel::GT( Rel::GT_e );
  const Rel Rel::GE( Rel::GE_e );
  const Rel Rel::ANY( Rel::ANY_e );
  const Rel Rel::NONE( Rel::NONE_e );

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Rel::Rel
  //	METHOD TYPE : Constructor
  //
  Rel::Rel( const std::string & strval_r )
  : _op( parse( strval_r ) )
  {}

  Rel::Rel( const std::string & strval_r, const Rel & default_r )
  : _op( parse( strval_r, default_r ) )
  {}

  bool Rel::parseFrom( const std::string & strval_r )
  {
    std::map<std::string,Rel::for_use_in_switch>::const_iterator it = findStr( strval_r );
    if ( it == _table.end() )
    {
      return false;
    }
    _op = it->second;
    return true;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Rel::asString
  //	METHOD TYPE : const std::string &
  //
  const std::string & Rel::asString() const
  {
    static std::map<for_use_in_switch,std::string> _table;
    if ( _table.empty() )
      {
        // initialize it
        _table[EQ_e]   = "==";
        _table[NE_e]   = "!=";
        _table[LT_e]   = "<";
        _table[LE_e]   = "<=";
        _table[GT_e]   = ">";
        _table[GE_e]   = ">=";
        _table[ANY_e]  = "ANY";
        _table[NONE_e] = "NONE";
      }
    return _table[_op];
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
