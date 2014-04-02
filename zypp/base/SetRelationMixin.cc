/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/SetRelationMixin.cc
 */

#include <map>
#include "zypp/base/SetRelationMixin.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  const std::string & _SetCompareDef::asString( Enum val_r )
  {
    static std::map<Enum,std::string> _table = {
      { uncomparable,		"{?}" },
      { equal,		"{=}" },
      { properSubset,		"{<}" },
      { properSuperset,	"{>}" },
      { disjoint,		"{ }" },
    };
    return _table[val_r];
  }

  const std::string & _SetRelationDef::asString( Enum val_r )
  {
    static std::map<Enum,std::string> _table = {
      { uncomparable,		"{??}" },
      { equal,		"{==}" },
      { properSubset,		"{<<}" },
      { properSuperset,	"{>>}" },
      { disjoint,		"{  }" },
      { subset,		"{<=}" },
      { superset,		"{>=}" },
    };
    return _table[val_r];
  }
} // namespace zypp
///////////////////////////////////////////////////////////////////
