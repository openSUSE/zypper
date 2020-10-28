#include "TestcaseSetupImpl.h"

namespace zypp::misc::testcase
{
  RepoData::RepoData() : _pimpl( new RepoDataImpl )
  {}

  RepoData::~RepoData()
  { }

  RepoData::RepoData(RepoDataImpl &&data) : _pimpl( new RepoDataImpl( std::move(data)) )
  { }

  TestcaseRepoType RepoData::type() const
  { return _pimpl->type; }

  const std::string &RepoData::alias() const
  { return _pimpl->alias; }

  uint RepoData::priority() const
  { return _pimpl->priority; }

  const std::string &RepoData::path() const
  { return _pimpl->path; }

  const RepoDataImpl &RepoData::data() const
  { return *_pimpl; }

  RepoDataImpl &RepoData::data()
  { return *_pimpl; }

  ForceInstall::ForceInstall() : _pimpl( new ForceInstallImpl )
  { }

  ForceInstall::~ForceInstall()
  { }

  ForceInstall::ForceInstall(ForceInstallImpl &&data) : _pimpl( new ForceInstallImpl( std::move(data) ))
  { }

  const ForceInstallImpl &ForceInstall::data() const
  { return *_pimpl; }

  ForceInstallImpl &ForceInstall::data()
  { return *_pimpl; }

  const std::string &ForceInstall::channel() const
  { return _pimpl->channel; }

  const std::string &ForceInstall::package() const
  { return _pimpl->package; }

  const std::string &ForceInstall::kind() const
  { return _pimpl->kind; }

  TestcaseSetup::TestcaseSetup() : _pimpl( new TestcaseSetupImpl )
  { }

  TestcaseSetup::~TestcaseSetup()
  { }

  Arch TestcaseSetup::architecture() const
  { return _pimpl->architecture; }

  const std::optional<RepoData> &TestcaseSetup::systemRepo() const
  { return _pimpl->systemRepo; }

  const std::vector<RepoData> &TestcaseSetup::repos() const
  { return _pimpl->repos; }

  ResolverFocus TestcaseSetup::resolverFocus() const
  { return _pimpl->resolverFocus; }

  const zypp::filesystem::Pathname &TestcaseSetup::globalPath() const
  { return _pimpl->globalPath; }

  const zypp::filesystem::Pathname &TestcaseSetup::hardwareInfoFile() const
  { return _pimpl->hardwareInfoFile; }

  const zypp::filesystem::Pathname &TestcaseSetup::systemCheck() const
  { return _pimpl->systemCheck; }

  const target::Modalias::ModaliasList &TestcaseSetup::modaliasList() const
  { return _pimpl->modaliasList; }

  const base::SetTracker<LocaleSet> &TestcaseSetup::localesTracker() const
  { return _pimpl->localesTracker; }

  const std::vector<std::vector<std::string> > &TestcaseSetup::vendorLists() const
  { return _pimpl->vendorLists; }

  const sat::StringQueue &TestcaseSetup::autoinstalled() const
  { return _pimpl->autoinstalled; }

  const std::set<std::string> &TestcaseSetup::multiversionSpec() const
  { return _pimpl->multiversionSpec; }

  const std::vector<ForceInstall> &TestcaseSetup::forceInstallTasks() const
  { return _pimpl->forceInstallTasks; }

  bool TestcaseSetup::set_licence() const
  { return _pimpl->set_licence; }

  bool TestcaseSetup::show_mediaid() const
  { return _pimpl->show_mediaid; }

  bool TestcaseSetup::ignorealreadyrecommended() const
  { return _pimpl->ignorealreadyrecommended; }

  bool TestcaseSetup::onlyRequires() const
  { return _pimpl->onlyRequires; }

  bool TestcaseSetup::forceResolve() const
  { return _pimpl->forceResolve; }

  bool TestcaseSetup::cleandepsOnRemove() const
  { return _pimpl->cleandepsOnRemove; }

  bool TestcaseSetup::allowDowngrade() const
  { return _pimpl->allowDowngrade; }

  bool TestcaseSetup::allowNameChange() const
  { return _pimpl->allowNameChange; }

  bool TestcaseSetup::allowArchChange() const
  { return _pimpl->allowArchChange; }

  bool TestcaseSetup::allowVendorChange() const
  { return _pimpl->allowVendorChange; }

  bool TestcaseSetup::dupAllowDowngrade() const
  { return _pimpl->dupAllowDowngrade; }

  bool TestcaseSetup::dupAllowNameChange() const
  { return _pimpl->dupAllowNameChange; }

  bool TestcaseSetup::dupAllowArchChange() const
  { return _pimpl->dupAllowArchChange; }

  bool TestcaseSetup::dupAllowVendorChange() const
  { return _pimpl->dupAllowVendorChange; }

  bool TestcaseSetup::applySetup( zypp::RepoManager &manager ) const
  {
    const auto &setup = data();
    if ( !setup.architecture.empty() )
    {
      MIL << "Setting architecture to '" << setup.architecture << "'" << std::endl;
      ZConfig::instance().setSystemArchitecture( setup.architecture );
      setenv ("ZYPP_TESTSUITE_FAKE_ARCH", setup.architecture.c_str(), 1);
    }

    if ( setup.systemRepo ) {
      if (!loadRepo( manager, *this, *setup.systemRepo ) )
      {
        ERR << "Can't setup 'system'" << std::endl;
        return false;
      }
    }

    if ( !setup.hardwareInfoFile.empty() ) {
      setenv( "ZYPP_MODALIAS_SYSFS", setup.hardwareInfoFile.asString().c_str(), 1 );
      MIL << "setting HardwareInfo to: " << setup.hardwareInfoFile.asString() << std::endl;
    }

    for ( const auto &channel : setup.repos ) {
      if ( !loadRepo( manager, *this, channel )  )
      {
        ERR << "Can't setup 'channel'" << std::endl;
        return false;
      }
    }

    if ( !setup.systemCheck.empty() ) {
      MIL << "setting systemCheck to: " << setup.systemCheck.asString() << std::endl;
      SystemCheck::instance().setFile( setup.systemCheck );
    }

    return true;
  }

  bool TestcaseSetup::loadRepo( zypp::RepoManager &manager, const TestcaseSetup &setup, const RepoData &data )
  {
    const auto &repoData = data.data();
    Pathname pathname = setup._pimpl->globalPath + repoData.path;
    MIL << "'" << pathname << "'" << std::endl;

    Repository repo;

    using TrType = zypp::misc::testcase::TestcaseRepoType;

    if ( repoData.type == TrType::Url ) {
      try {
        MIL << "Load from Url '" << repoData.path << "'" << std::endl;

        RepoInfo nrepo;
        nrepo.setAlias      ( repoData.alias );
        nrepo.setName       ( repoData.alias );
        nrepo.setEnabled    ( true );
        nrepo.setAutorefresh( false );
        nrepo.setPriority   ( repoData.priority );
        nrepo.addBaseUrl   ( Url(repoData.path) );

        manager.refreshMetadata( nrepo );
        manager.buildCache( nrepo );
        manager.loadFromCache( nrepo );
      }
      catch ( Exception & excpt_r ) {
        ZYPP_CAUGHT (excpt_r);
        ERR << "Couldn't load packages from Url '" << repoData.path << "'" << std::endl;
        return false;
      }
    }
    else {
      try {
        MIL << "Load from File '" << pathname << "'" << std::endl;
        zypp::Repository satRepo;

        if ( repoData.alias == "@System" ) {
          satRepo = zypp::sat::Pool::instance().systemRepo();
        } else {
          satRepo = zypp::sat::Pool::instance().reposInsert( repoData.alias );
        }

        RepoInfo nrepo;

        nrepo.setAlias      ( repoData.alias );
        nrepo.setName       ( repoData.alias );
        nrepo.setEnabled    ( true );
        nrepo.setAutorefresh( false );
        nrepo.setPriority   ( repoData.priority );
        nrepo.addBaseUrl   ( pathname.asUrl() );

        satRepo.setInfo (nrepo);
        if ( repoData.type == TrType::Helix )
          satRepo.addHelix( pathname );
        else
          satRepo.addTesttags( pathname );
        MIL << "Loaded " << satRepo.solvablesSize() << " resolvables from " << ( repoData.path.empty()?pathname.asString():repoData.path) << "." << std::endl;
      }
      catch ( Exception & excpt_r ) {
        ZYPP_CAUGHT (excpt_r);
        ERR << "Couldn't load packages from XML file '" << repoData.path << "'" << std::endl;
        return false;
      }
    }
    return true;
  }

  TestcaseSetupImpl &TestcaseSetup::data()
  {
    return *_pimpl;
  }

  const TestcaseSetupImpl &TestcaseSetup::data() const
  {
    return *_pimpl;
  }

}
