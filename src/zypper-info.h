#ifndef ZYPPERINFO_H_
#define ZYPPERINFO_H_

#include "zypp/PoolItem.h"
#include "zypp/Resolvable.h"

#include "zypper.h"

void printInfo(const Zypper & zypper, const zypp::Resolvable::Kind & kind);

void printPkgInfo(const Zypper & zypper,
                  const zypp::PoolItem & pool_item,
                  const zypp::PoolItem & ins_pool_item);

void printPatchInfo(const Zypper & zypper,
                    const zypp::PoolItem & pool_item,
                    const zypp::PoolItem & ins_pool_item);

void printPatternInfo(const Zypper & zypper,
                      const zypp::PoolItem & pool_item,
                      const zypp::PoolItem & ins_pool_item);

void printProductInfo(const Zypper & zypper,
                      const zypp::PoolItem & pool_item,
                      const zypp::PoolItem & ins_pool_item);

#endif /*ZYPPERINFO_H_*/
