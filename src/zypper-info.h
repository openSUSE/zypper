#ifndef ZYPPERINFO_H_
#define ZYPPERINFO_H_

#include "zypp/PoolItem.h"

#include "zypper.h"

void printInfo(const Zypper & zypper);

void printPkgInfo(const Zypper & zypper,
                  const zypp::PoolItem & pool_item,
                  const zypp::PoolItem & ins_pool_item);

void printPatchInfo(const Zypper & zypper,
                    const zypp::PoolItem & pool_item,
                    const zypp::PoolItem & ins_pool_item);

#endif /*ZYPPERINFO_H_*/
