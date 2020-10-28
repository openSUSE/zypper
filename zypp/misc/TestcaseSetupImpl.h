/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_MISC_TESTCASESETUPIMPL_H
#define ZYPP_MISC_TESTCASESETUPIMPL_H

#include <zypp/misc/TestcaseSetup.h>
#include <zypp/ZConfig.h>
#include <zypp/base/LogControl.h>
#include <zypp/Repository.h>
#include <zypp/RepoManager.h>
#include <zypp/sat/Pool.h>

#define ZYPP_USE_RESOLVER_INTERNALS
#include <zypp/solver/detail/SystemCheck.h>

namespace zypp::misc::testcase
{
  struct RepoDataImpl {
    TestcaseRepoType type;
    std::string alias;
    uint priority = 99;
    std::string path;

    RepoDataImpl *clone () const { return new RepoDataImpl(*this); }
  };

  struct ForceInstallImpl {
    std::string channel;
    std::string package;
    std::string kind;

    ForceInstallImpl *clone () const { return new ForceInstallImpl(*this); }
  };

  struct TestcaseSetupImpl
  {
    Arch architecture = Arch_noarch;

    std::optional<RepoData> systemRepo;
    std::vector<RepoData> repos;

    // solver flags: default to false - set true if mentioned in <setup>
    ResolverFocus resolverFocus  = ResolverFocus::Default;

    Pathname globalPath;
    Pathname hardwareInfoFile;
    Pathname systemCheck;

    target::Modalias::ModaliasList modaliasList;
    base::SetTracker<LocaleSet> localesTracker;
    std::vector<std::vector<std::string>> vendorLists;
    sat::StringQueue autoinstalled;
    std::set<std::string> multiversionSpec;
    std::vector<ForceInstall> forceInstallTasks;

    bool set_licence = false;
    bool show_mediaid = false;

    bool ignorealreadyrecommended   = false;
    bool onlyRequires               = false;
    bool forceResolve               = false;
    bool cleandepsOnRemove          = false;

    bool allowDowngrade     = false;
    bool allowNameChange    = false;
    bool allowArchChange    = false;
    bool allowVendorChange  = false;

    bool dupAllowDowngrade     = false;
    bool dupAllowNameChange    = false;
    bool dupAllowArchChange    = false;
    bool dupAllowVendorChange  = false;

    TestcaseSetupImpl *clone () const { return new TestcaseSetupImpl(*this); }
  };
}

#endif
