#ifndef INSTALL_H_
#define INSTALL_H_

#include "zypp/PoolItem.h"

#include "Zypper.h"


void install_remove(Zypper & zypper,
                    const Zypper::ArgList & args,
                    bool install_not_remove,
                    const zypp::ResKind & kind);

// copied from yast2-pkg-bindings:PkgModuleFunctions::DoProvideNameKind
struct ProvideProcess
{
  zypp::PoolItem item;
  zypp::PoolItem installed_item;
  zypp::ResStatus::TransactByValue whoWantsIt;
  std::string _repo;
  zypp::Arch _architecture;

  ProvideProcess(zypp::Arch arch, const std::string & repo)
    : whoWantsIt(zypp::ResStatus::USER)
    , _repo(repo)
    , _architecture( arch )
    { }

  bool operator()( const zypp::PoolItem& provider );
};

#endif /*INSTALL_H_*/
