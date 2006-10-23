#ifndef ZYPPERINFO_H_
#define ZYPPERINFO_H_

#include <string>
#include <vector>

#include "zypp/PoolItem.h"

void printInfo(const std::string & command, const std::vector<std::string> & arguments);
void printPkgInfo(const zypp::PoolItem & pool_item,  const zypp::PoolItem & ins_pool_item);

#endif /*ZYPPERINFO_H_*/
