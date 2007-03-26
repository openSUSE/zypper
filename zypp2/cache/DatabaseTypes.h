#ifndef ZYPP_CACHE_DB_TYPES_H
#define ZYPP_CACHE_DB_TYPES_H

#include "zypp2/cache/CacheCommon.h"
#include "zypp/NeedAType.h"
#include "zypp/Resolvable.h"
#include "zypp/base/Logger.h"
#include "zypp/Arch.h"
#include "zypp/Rel.h"

namespace zypp
{
  namespace cache
  {
    db::Rel zypp_rel2db_rel( zypp::Rel op);
    zypp::Rel db_rel2zypp_rel ( db::Rel rel);
    
    db::Arch zypp_arch2db_arch(const zypp::Arch & arch);
    zypp::Arch db_arch2zypp_arch ( db::Arch rc);
    
    std::string desc2str (const zypp::Text t);
    
    db::Kind zypp_kind2db_kind( zypp::Resolvable::Kind kind );
    zypp::Resolvable::Kind db_kind2zypp_kind( db::Kind kind );
    
    db::DependencyType zypp_deptype2db_deptype( zypp::Dep deptype );
    zypp::Dep db_deptype2zypp_deptype( db::DependencyType deptype );
  }
}

#endif

