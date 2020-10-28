/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/misc/TestcaseSetup.h
 *
*/

#ifndef ZYPP_MISC_TESTCASESETUP_H
#define ZYPP_MISC_TESTCASESETUP_H

#include <zypp/Arch.h>
#include <zypp/Locale.h>
#include <zypp/Pathname.h>
#include <zypp/ResolverFocus.h>
#include <zypp/Url.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/SetTracker.h>
#include <zypp/sat/Queue.h>
#include <zypp/target/modalias/Modalias.h>

#include <optional>
#include <vector>

namespace zypp {
  class RepoManager;
}

namespace zypp::misc::testcase
{

  enum class TestcaseRepoType {
    Helix,
    Testtags,
    Url
  };

  struct RepoDataImpl;
  struct ForceInstallImpl;
  struct TestcaseSetupImpl;

  class RepoData {
  public:
    RepoData ();
    ~RepoData ();
    RepoData ( RepoDataImpl &&data );
    TestcaseRepoType type() const;
    const std::string &alias() const;
    uint priority() const;
    const std::string &path() const;

    const RepoDataImpl &data() const;
    RepoDataImpl &data();
  private:
    RWCOW_pointer<RepoDataImpl> _pimpl;
  };

  class ForceInstall {
  public:
    ForceInstall ();
    ~ForceInstall ();
    ForceInstall ( ForceInstallImpl &&data );
    const std::string &channel () const;
    const std::string &package () const;
    const std::string &kind () const;

    const ForceInstallImpl &data() const;
    ForceInstallImpl &data();
  private:
    RWCOW_pointer<ForceInstallImpl> _pimpl;
  };

  class TestcaseSetup
  {
  public:

    TestcaseSetup();
    ~TestcaseSetup();

    Arch architecture () const;

    const std::optional<RepoData> &systemRepo() const;
    const std::vector<RepoData> &repos() const;

    // solver flags: default to false - set true if mentioned in <setup>
    ResolverFocus resolverFocus() const;

    const Pathname &globalPath() const;
    const Pathname &hardwareInfoFile() const;
    const Pathname &systemCheck() const;

    const target::Modalias::ModaliasList &modaliasList() const;
    const base::SetTracker<LocaleSet> &localesTracker() const;
    const std::vector<std::vector<std::string>> &vendorLists() const;
    const sat::StringQueue &autoinstalled() const;
    const std::set<std::string> &multiversionSpec() const;
    const std::vector<ForceInstall> &forceInstallTasks() const;

    bool set_licence() const;
    bool show_mediaid() const;

    bool ignorealreadyrecommended() const;
    bool onlyRequires() const;
    bool forceResolve() const;
    bool cleandepsOnRemove() const;

    bool allowDowngrade() const;
    bool allowNameChange() const;
    bool allowArchChange() const;
    bool allowVendorChange() const;

    bool dupAllowDowngrade() const;
    bool dupAllowNameChange() const;
    bool dupAllowArchChange() const;
    bool dupAllowVendorChange() const;

    bool applySetup ( zypp::RepoManager &manager ) const;

    static bool loadRepo (zypp::RepoManager &manager, const TestcaseSetup &setup, const RepoData &data );

    TestcaseSetupImpl &data();
    const TestcaseSetupImpl &data() const;

  private:
    RWCOW_pointer<TestcaseSetupImpl> _pimpl;
  };

}


#endif // ZYPP_MISC_TESTCASESETUPIMPL_H
