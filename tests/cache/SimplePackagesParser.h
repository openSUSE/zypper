
#ifndef SimplePackagesParser_H
#define SimplePackagesParser_H

#include <list>
#include "zypp/NVRA.h"
#include "zypp/Pathname.h"
#include "zypp/data/ResolvableData.h"

struct MiniResolvable
{
  zypp::NVRA nvra;
  zypp::data::Dependencies deps;
};

void parse_mini_file(const zypp::Pathname &nvra_list, std::list<MiniResolvable> &res_list);

#endif
