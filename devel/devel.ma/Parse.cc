#include "Tools.h"

#include <zypp/base/PtrTypes.h>
#include <zypp/base/Exception.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/ProvideNumericId.h>
#include <zypp/AutoDispose.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"

#include "zypp/ZYppCallbacks.h"
#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/Package.h"
#include "zypp/Pattern.h"
#include "zypp/Language.h"
#include "zypp/Digest.h"
#include "zypp/PackageKeyword.h"
#include "zypp/ManagedFile.h"


#include "zypp/parser/TagParser.h"
#include "zypp/parser/susetags/PackagesFileReader.h"
#include "zypp/parser/susetags/PackagesLangFileReader.h"
#include "zypp/parser/susetags/PatternFileReader.h"
#include "zypp/parser/susetags/ContentFileReader.h"
#include "zypp/parser/susetags/RepoIndex.h"
#include "zypp/parser/susetags/RepoParser.h"
#include "zypp/cache/CacheStore.h"
#include "zypp/RepoManager.h"
#include "zypp/RepoInfo.h"

#include "zypp/repo/DeltaCandidates.h"
#include "zypp/repo/PackageProvider.h"
#include "zypp/repo/SrcPackageProvider.h"

#include "zypp/ui/PatchContents.h"
#include "zypp/ResPoolProxy.h"

using namespace std;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::ui;
using zypp::parser::TagParser;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/ROOT" );

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

bool queryInstalledEditionHelper( const std::string & name_r,
                                  const Edition &     ed_r,
                                  const Arch &        arch_r )
{
  if ( ed_r == Edition::noedition )
    return true;
  if ( name_r == "kernel-default" && ed_r == Edition("2.6.22.5-10") )
    return true;
  if ( name_r == "update-test-affects-package-manager" && ed_r == Edition("1.1-6") )
    return true;

  return false;
}


ManagedFile repoProvidePackage( const PoolItem & pi )
{
  ResPool _pool( getZYpp()->pool() );
  repo::RepoMediaAccess _access;

  // Redirect PackageProvider queries for installed editions
  // (in case of patch/delta rpm processing) to rpmDb.
  repo::PackageProviderPolicy packageProviderPolicy;
  packageProviderPolicy.queryInstalledCB( queryInstalledEditionHelper );

  Package::constPtr p = asKind<Package>(pi.resolvable());

  // Build a repository list for repos
  // contributing to the pool
  repo::DeltaCandidates deltas( repo::makeDeltaCandidates( _pool.knownRepositoriesBegin(),
                                                           _pool.knownRepositoriesEnd() ) );
  repo::PackageProvider pkgProvider( _access, p, deltas, packageProviderPolicy );
  return pkgProvider.providePackage();
}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

void dbgDu( Selectable::Ptr sel )
{
  if ( sel->installedPoolItem() )
  {
    DBG << "i: " << sel->installedPoolItem() << endl
        << sel->installedPoolItem()->diskusage() << endl;
  }
  if ( sel->candidatePoolItem() )
  {
    DBG << "c: " << sel->candidatePoolItem() << endl
        << sel->candidatePoolItem()->diskusage() << endl;
  }
  INT << sel << endl
      << getZYpp()->diskUsage() << endl;
}

///////////////////////////////////////////////////////////////////

struct Xprint
{
  bool operator()( const PoolItem & obj_r )
  {
    if ( obj_r.status().isLocked() )
      SEC << obj_r << endl;

//     handle( asKind<Package>( obj_r ) );
//     handle( asKind<Patch>( obj_r ) );
//     handle( asKind<Pattern>( obj_r ) );
//     handle( asKind<Product>( obj_r ) );
    return true;
  }

  void handle( const Package_constPtr & p )
  {
    if ( !p )
      return;

    WAR << p->size() << endl;
    MIL << p->diskusage() << endl;
  }

  void handle( const Patch_constPtr & p )
  {
    if ( !p )
      return;
  }

  void handle( const Pattern_constPtr & p )
  {
    if ( !p )
      return;

    if ( p->vendor().empty() )
      ERR << p << endl;
    else if ( p->vendor() == "SUSE (assumed)" )
      SEC << p << endl;
  }

  void handle( const Product_constPtr & p )
  {
    if ( !p )
      return;

    USR << p << endl;
    USR << p->vendor() << endl;
    USR << p->type() << endl;
  }

  template<class _C>
  bool operator()( const _C & obj_r )
  {
    return true;
  }
};

///////////////////////////////////////////////////////////////////
struct SetTransactValue
{
  SetTransactValue( ResStatus::TransactValue newVal_r, ResStatus::TransactByValue causer_r )
  : _newVal( newVal_r )
  , _causer( causer_r )
  {}

  ResStatus::TransactValue   _newVal;
  ResStatus::TransactByValue _causer;

  bool operator()( const PoolItem & pi ) const
  {
    bool ret = pi.status().setTransactValue( _newVal, _causer );
    if ( ! ret )
      ERR << _newVal <<  _causer << " " << pi << endl;
    return ret;
  }
};

struct StatusReset : public SetTransactValue
{
  StatusReset()
  : SetTransactValue( ResStatus::KEEP_STATE, ResStatus::USER )
  {}
};

struct StatusInstall : public SetTransactValue
{
  StatusInstall()
  : SetTransactValue( ResStatus::TRANSACT, ResStatus::USER )
  {}
};

///////////////////////////////////////////////////////////////////

bool solve( bool establish = false )
{
  if ( establish )
  {
    bool eres = false;
    {
      zypp::base::LogControl::TmpLineWriter shutUp;
      eres = getZYpp()->resolver()->establishPool();
    }
    if ( ! eres )
    {
      ERR << "establish " << eres << endl;
      return false;
    }
    MIL << "establish " << eres << endl;
  }

  bool rres = false;
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    return false;
  }
  MIL << "resolve " << rres << endl;
  return true;
}

bool install()
{
  SEC << getZYpp()->commit( ZYppCommitPolicy() ) << endl;
  return true;
}

///////////////////////////////////////////////////////////////////

struct ConvertDbReceive : public callback::ReceiveReport<target::ScriptResolvableReport>
{
  virtual void start( const Resolvable::constPtr & script_r,
                      const Pathname & path_r,
                      Task task_r )
  {
    SEC << __FUNCTION__ << endl
    << "  " << script_r << endl
    << "  " << path_r   << endl
    << "  " << task_r   << endl;
  }

  virtual bool progress( Notify notify_r, const std::string & text_r )
  {
    SEC << __FUNCTION__ << endl
    << "  " << notify_r << endl
    << "  " << text_r   << endl;
    return true;
  }

  virtual void problem( const std::string & description_r )
  {
    SEC << __FUNCTION__ << endl
    << "  " << description_r << endl;
  }

  virtual void finish()
  {
    SEC << __FUNCTION__ << endl;
  }

};
///////////////////////////////////////////////////////////////////

struct DigestReceive : public callback::ReceiveReport<DigestReport>
{
  DigestReceive()
  {
    connect();
  }

  virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
  {
    USR << endl;
    return false;
  }
  virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
  {
    USR << endl;
    return false;
  }
  virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
  {
    USR << "fle " << PathInfo(file) << endl;
    USR << "req " << requested << endl;
    USR << "fnd " << found << endl;

    waitForInput();

    return false;
  }
};

struct KeyRingSignalsReceive : public callback::ReceiveReport<KeyRingSignals>
{
  KeyRingSignalsReceive()
  {
    connect();
  }
  virtual void trustedKeyAdded( const PublicKey &/*key*/ )
  {
    USR << endl;
  }
  virtual void trustedKeyRemoved( const PublicKey &/*key*/ )
  {
    USR << endl;
  }
};

///////////////////////////////////////////////////////////////////

struct MediaChangeReceive : public callback::ReceiveReport<media::MediaChangeReport>
{
  virtual Action requestMedia( Url & source
                               , unsigned mediumNr
                               , Error error
                               , const std::string & description )
  {
    SEC << __FUNCTION__ << endl
    << "  " << source << endl
    << "  " << mediumNr << endl
    << "  " << error << endl
    << "  " << description << endl;
    return IGNORE;
  }
};

///////////////////////////////////////////////////////////////////

namespace container
{
  template<class _Tp>
    bool isIn( const std::set<_Tp> & cont, const typename std::set<_Tp>::value_type & val )
    { return cont.find( val ) != cont.end(); }
}

///////////////////////////////////////////////////////////////////

struct AddResolvables
{
  bool operator()( const Repository & src ) const
  {
    getZYpp()->addResolvables( src.resolvables() );
    return true;
  }
};

///////////////////////////////////////////////////////////////////


std::ostream & operator<<( std::ostream & str, const iostr::EachLine & obj )
{
  str << "(" << obj.valid() << ")[" << obj.lineNo() << "|" << obj.lineStart() << "]{" << *obj << "}";
  return str;

}

///////////////////////////////////////////////////////////////////

#define for_(IT,BEG,END) for ( typeof(BEG) IT = BEG; IT != END; ++IT )

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////


  void Vtst( const std::string & lhs, const std::string & rhs )
  {
    (VendorAttr::instance().equivalent( lhs, rhs )?MIL:ERR) << lhs << " <==> "<< rhs << endl;

  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

using namespace zypp;

void tt( std::string dd )
{
  unsigned level = 3;
  std::string::size_type pos = dd.find( "/" );
  while ( --level && pos != std::string::npos )
  {
    pos = dd.find( "/", pos+1 );
  }
  if ( pos != std::string::npos )
    dd.erase( pos+1 );
  DBG << dd << "\t" << level << " " << pos << endl;
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "log.restrict" );
  INT << "===[START]==========================================" << endl;
  setenv( "ZYPP_CONF", "/Local/ROOT/zypp.conf", 1 );

  DigestReceive foo;
  KeyRingSignalsReceive baa;

  RepoManager repoManager( makeRepoManager( "/Local/ROOT" ) );

  RepoInfoList repos = repoManager.knownRepositories();
  SEC << "/Local/ROOT " << repos << endl;

  for ( RepoInfoList::iterator it = repos.begin(); it != repos.end(); ++it )
  {
    RepoInfo & nrepo( *it );
    if ( ! nrepo.enabled() )
      continue;

    if ( ! repoManager.isCached( nrepo ) || 0 )
    {
      if ( repoManager.isCached( nrepo ) )
      {
	SEC << "cleanCache" << endl;
	repoManager.cleanCache( nrepo );
      }
      SEC << "refreshMetadata" << endl;
      //repoManager.refreshMetadata( nrepo, RepoManager::RefreshForced );
      repoManager.refreshMetadata( nrepo );
      SEC << "buildCache" << endl;
      repoManager.buildCache( nrepo );
    }

    SEC << nrepo << endl;
    Repository nrep( repoManager.createFromCache( nrepo ) );
    const zypp::ResStore & store( nrep.resolvables() );

    dumpPoolStats( SEC << "Store: " << endl,
		   store.begin(), store.end() ) << endl;
    getZYpp()->addResolvables( store );
  }

  ResPool pool( getZYpp()->pool() );
  USR << "pool: " << pool << endl;
  SEC << pool.knownRepositoriesSize() << endl;

  if ( 0 )
  {
    {
      zypp::base::LogControl::TmpLineWriter shutUp;
      //getZYpp()->initTarget( sysRoot );
      getZYpp()->initTarget( "/" );
    }
    MIL << "Added target: " << pool << endl;
  }


  //std::for_each( pool.begin(), pool.end(), Xprint() );

  PoolItem pi = getPi<Patch>( "fetchmsttfonts.sh" );
  USR << pi << endl;

  //pi.status().setTransact( true, ResStatus::USER );
  //install();

 ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}

