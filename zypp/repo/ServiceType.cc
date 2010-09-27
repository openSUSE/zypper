/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include <map>
#include "zypp/repo/RepoException.h"
#include "ServiceType.h"

namespace zypp
{
  namespace repo
  {


  static std::map<std::string,ServiceType::Type> _table;

  const ServiceType ServiceType::RIS(ServiceType::RIS_e);
  const ServiceType ServiceType::NONE(ServiceType::NONE_e);
  const ServiceType ServiceType::PLUGIN(ServiceType::PLUGIN_e);

  ServiceType::ServiceType(const std::string & strval_r)
    : _type(parse(strval_r))
  {}

  ServiceType::Type ServiceType::parse(const std::string & strval_r)
  {
    if (_table.empty())
    {
      // initialize it
      _table["ris"] = ServiceType::RIS_e;
      _table["RIS"] = ServiceType::RIS_e;
      _table["nu"] = ServiceType::RIS_e;
      _table["NU"] = ServiceType::RIS_e;
      _table["plugin"] = ServiceType::PLUGIN_e;
      _table["PLUGIN"] = ServiceType::PLUGIN_e;
      _table["NONE"] = _table["none"] = ServiceType::NONE_e;
    }

    std::map<std::string,ServiceType::Type>::const_iterator it
      = _table.find(strval_r);
    if (it == _table.end())
    {
      ZYPP_THROW(RepoUnknownTypeException(
        "Unknown service type '" + strval_r + "'"));
    }
    return it->second;
  }


  const std::string & ServiceType::asString() const
  {
    static std::map<Type, std::string> _table;
    if ( _table.empty() )
    {
      // initialize it
      _table[RIS_e]  = "ris";
      _table[PLUGIN_e]  = "plugin";
      _table[NONE_e] = "NONE";
    }
    return _table[_type];
  }


  } // ns repo
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
