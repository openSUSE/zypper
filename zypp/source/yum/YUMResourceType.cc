/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <map>
#include "zypp/base/Exception.h"
#include "YUMResourceType.h"

namespace zypp
{
  namespace source
  {
    namespace yum
    {


  static std::map<std::string,YUMResourceType::Type> _table;

  const YUMResourceType YUMResourceType::REPOMD(YUMResourceType::REPOMD_e);
  const YUMResourceType YUMResourceType::PRIMARY(YUMResourceType::PRIMARY_e);
  const YUMResourceType YUMResourceType::OTHER(YUMResourceType::OTHER_e);
  const YUMResourceType YUMResourceType::FILELISTS(YUMResourceType::FILELISTS_e);
  const YUMResourceType YUMResourceType::GROUP(YUMResourceType::GROUP_e);
  const YUMResourceType YUMResourceType::PATCHES(YUMResourceType::PATCHES_e);
  const YUMResourceType YUMResourceType::PATCH(YUMResourceType::PATCH_e);
  const YUMResourceType YUMResourceType::PRODUCT(YUMResourceType::PRODUCT_e);
  const YUMResourceType YUMResourceType::PATTERN(YUMResourceType::PATTERN_e);


  YUMResourceType::YUMResourceType(const std::string & strval_r)
    : _type(parse(strval_r))
  {}

  YUMResourceType::Type YUMResourceType::parse(const std::string & strval_r)
  {
    if (_table.empty())
    {
      // initialize it
      _table["repomd"] = YUMResourceType::REPOMD_e;
      _table["primary"] = YUMResourceType::PRIMARY_e;
      _table["other"] = YUMResourceType::OTHER_e;
      _table["filelists"] = YUMResourceType::FILELISTS_e;
      _table["group"] = YUMResourceType::GROUP_e;
      _table["patches"] = YUMResourceType::PATCHES_e;
      _table["patch"] = YUMResourceType::PATCH_e;
      _table["product"] = YUMResourceType::PRODUCT_e;
      _table["patterns"] = YUMResourceType::PATTERN_e;
      _table["NONE"] = _table["none"] = YUMResourceType::NONE_e;
    }

    std::map<std::string,YUMResourceType::Type>::const_iterator it
      = _table.find(strval_r);
    if (it == _table.end())
    {
      ZYPP_THROW(Exception(
        "YUMResourceType parse: illegal string value '" + strval_r + "'"));
    }
    return it->second;
  }


  const std::string & YUMResourceType::asString() const
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
      _table[PATTERN_e]  = "patterns";
      _table[NONE_e] = "NONE";
    }
    return _table[_type];
  }


    } // ns yum
  } // ns source
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
