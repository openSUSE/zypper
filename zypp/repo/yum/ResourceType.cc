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
#include "zypp/base/Exception.h"
#include "ResourceType.h"

namespace zypp
{
  namespace repo
  {
    namespace yum
    {


  static std::map<std::string,ResourceType::Type> _table;

  const ResourceType ResourceType::NONE(ResourceType::NONE_e);
  const ResourceType ResourceType::REPOMD(ResourceType::REPOMD_e);
  const ResourceType ResourceType::PRIMARY(ResourceType::PRIMARY_e);
  const ResourceType ResourceType::OTHER(ResourceType::OTHER_e);
  const ResourceType ResourceType::FILELISTS(ResourceType::FILELISTS_e);
  const ResourceType ResourceType::GROUP(ResourceType::GROUP_e);
  const ResourceType ResourceType::PATCHES(ResourceType::PATCHES_e);
  const ResourceType ResourceType::PATCH(ResourceType::PATCH_e);
  const ResourceType ResourceType::PRODUCT(ResourceType::PRODUCT_e);
  const ResourceType ResourceType::PATTERNS(ResourceType::PATTERNS_e);
  const ResourceType ResourceType::PRIMARY_DB(ResourceType::PRIMARY_DB_e);
  const ResourceType ResourceType::OTHER_DB(ResourceType::OTHER_DB_e);

  ResourceType::ResourceType(const std::string & strval_r)
    : _type(parse(strval_r))
  {}

  ResourceType::Type ResourceType::parse(const std::string & strval_r)
  {
    if (_table.empty())
    {
      // initialize it
      _table["repomd"] = ResourceType::REPOMD_e;
      _table["primary"] = ResourceType::PRIMARY_e;
      _table["other"] = ResourceType::OTHER_e;
      _table["filelists"] = ResourceType::FILELISTS_e;
      _table["group"] = ResourceType::GROUP_e;
      _table["patches"] = ResourceType::PATCHES_e;
      _table["patch"] = ResourceType::PATCH_e;
      _table["product"] = ResourceType::PRODUCT_e;
      _table["patterns"] = ResourceType::PATTERNS_e;
      _table["primary_db"] = ResourceType::PRIMARY_DB_e;
      _table["other_db"] = ResourceType::OTHER_DB_e;
      _table["NONE"] = _table["none"] = ResourceType::NONE_e;
    }

    std::map<std::string,ResourceType::Type>::const_iterator it
      = _table.find(strval_r);
    if (it == _table.end())
    {
      return ResourceType::NONE_e;
    }
    return it->second;
  }


  const std::string & ResourceType::asString() const
  {
    static std::map<Type, std::string> _table;
    if ( _table.empty() )
    {
      // initialize it
      _table[REPOMD_e]   = "repomd";
      _table[PRIMARY_e]   = "primary";
      _table[OTHER_e]   = "other";
      _table[FILELISTS_e]   = "filelists";
      _table[GROUP_e]   = "group";
      _table[PATCHES_e]   = "patches";
      _table[PATCH_e]  = "patch";
      _table[PRODUCT_e]  = "product";
      _table[PATTERNS_e]  = "patterns";
      _table[OTHER_DB_e]  = "other_db";
      _table[PRIMARY_DB_e]  = "primary_db";
      _table[NONE_e] = "NONE";
    }
    return _table[_type];
  }


    } // ns yum
  } // ns source
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
