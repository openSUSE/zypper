/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/misc/LoadTestcase.h
 *
*/
#ifndef ZYPP_MISC_LOADTESTCASE_H
#define ZYPP_MISC_LOADTESTCASE_H

#include <zypp/Arch.h>
#include <zypp/Locale.h>
#include <zypp/Pathname.h>
#include <zypp/ResolverFocus.h>
#include <zypp/Url.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/SetTracker.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/sat/Queue.h>
#include <zypp/target/modalias/Modalias.h>

#include <optional>
#include <memory>

namespace zypp {
  class RepoManager;
}

namespace zypp::misc::testcase {

  enum class TestcaseRepoType {
    Helix,
    Testtags,
    Url
  };

  struct RepoData {
    TestcaseRepoType type;
    std::string alias;
    uint priority = 99;
    std::string path;
  };

  struct ForceInstall {
    std::string channel;
    std::string package;
    std::string kind;
  };

  struct TestcaseSetup
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

    bool applySetup ( zypp::RepoManager &manager ) const;

    static bool loadRepo (zypp::RepoManager &manager, const TestcaseSetup &setup, const RepoData &data );
  };

  struct TestcaseTrial
  {
    struct Node {
      std::string name;
      std::string value;

      const std::string &getProp( const std::string &name, const std::string &def = std::string() ) const;

      std::map<std::string, std::string> properties;
      std::vector<std::shared_ptr<Node>> children;
    };
    std::vector<Node> nodes;
  };

  class LoadTestcase : private zypp::base::NonCopyable
  {
  public:
    struct Impl;

    enum Type {
      None,
      Helix,
      Yaml
    };

    LoadTestcase();
    ~LoadTestcase();

    bool loadTestcaseAt ( const zypp::Pathname &path, std::string *err );
    static Type testcaseTypeAt ( const zypp::Pathname &path );

    const TestcaseSetup &setupInfo() const;
    const std::vector<TestcaseTrial> &trialInfo() const;

  private:
    std::unique_ptr<Impl> _pimpl;
  };

}


#endif // ZYPP_MISC_LOADTESTCASE_H
