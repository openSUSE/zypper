/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include <zypp/base/LogTools.h>
#include <zypp/Package.h>
#include <zypp/ResPool.h>
#include <zypp/Pathname.h>
#include <zypp/PoolQuery.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/ui/SelectableTraits.h>
#include <zypp/target/CommitPackageCache.h>

#include "commands/conditions.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"
#include "Zypper.h"
#include "PackageArgs.h"
#include "Table.h"
#include "download.h"
#include "callbacks/media.h"
#include "global-settings.h"

using namespace zypp;

namespace
{
  namespace env {
    /** XDG_CACHE_HOME: base directory relative to which user specific non-essential data files should be stored.
     * http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
     */
    inline filesystem::Pathname XDG_CACHE_HOME()
    {
      filesystem::Pathname ret;
      const char * envp = getenv( "XDG_CACHE_HOME" );
      if ( envp && *envp )
        ret = envp;
      else
      {
        ret = getenv( "HOME" );
        ret /= ".cache";
      }
      return ret;
    }
  } //namespace env

  inline bool isPackageType( const sat::Solvable & slv_r )
  { return( slv_r.isKind<Package>() || slv_r.isKind<SrcPackage>() ); }

  // Valid for Package and SrcPackage
  bool isCached( const PoolItem & pi_r )
  { return ( pi_r.isKind<Package>() && pi_r->asKind<Package>()->isCached() )
        || ( pi_r.isKind<SrcPackage>() && pi_r->asKind<SrcPackage>()->isCached() ); }

  // Valid for Package and SrcPackage; assumes isPackageType() == true
  Pathname cachedLocation( const PoolItem & pi_r )
  { return( pi_r.isKind<Package>() ? pi_r->asKind<Package>()->cachedLocation() : pi_r->asKind<SrcPackage>()->cachedLocation() ); }

  inline void logXmlResult( const PoolItem & pi_r, const Pathname & localfile_r )
  {
    //   <download-result>
    //     <solvable>
    //       <kind>package</kind>
    //       <name>glibc</name>
    //       <edition epoch="0" version="2.18" release="4.11.1"/>
    //       <arch>i586</arch>
    //       <repository name="repo-oss-update" alias="repo-oss-update (13.1)"/>
    //     </solvable>
    //     <localfile path="/tmp/chroot/repo-oss-update/i586/glibc-2.18-4.11.1.i586.rpm"/>
    //     <!-- <localfile/> on error -->
    //   </download-result>
    xmlout::Node guard( cout, "download-result" );
    dumpAsXmlOn( *guard, pi_r.satSolvable() );
    {
      if ( localfile_r.empty() )
	xmlout::Node( *guard, "localfile", xmlout::Node::optionalContent );
      else
	xmlout::Node( *guard, "localfile", xmlout::Node::optionalContent,
		      { "path", xml::escape( localfile_r.asString() ) } );
    }
  }

  /** Whether user may create \a dir_r or has rw-access to it. */
  inline bool userMayUseDir( const Pathname & dir_r )
  {
    bool mayuse = true;
    if ( dir_r.empty()  )
      mayuse = false;
    else
    {
      PathInfo pi( dir_r );
      if ( pi.isExist() )
      {
	if ( ! ( pi.isDir() && pi.userMayRWX() ) )
	  mayuse = false;
      }
      else
	mayuse = userMayUseDir( dir_r.dirname() );
    }
    return mayuse;
  }

  class EnsureWriteableCacheCondition : public BaseCommandCondition
  {
    // BaseCommandCondition interface
  public:
    int check( std::string &err_r ) override
    {
      auto &zypp = Zypper::instance();

      // Check for a usable pkg-cache-dir
      if ( geteuid() != 0 )
      {
        const auto &gOpts = zypp.config();
        bool mayuse = userMayUseDir( gOpts.rm_options.repoPackagesCachePath );

        if ( ! mayuse && /* is the default path: */
             gOpts.rm_options.repoPackagesCachePath == RepoManagerOptions( gOpts.root_dir ).repoPackagesCachePath )
        {
          zypp.configNoConst().rm_options.repoPackagesCachePath = env::XDG_CACHE_HOME() / "zypp/packages";
          mayuse = userMayUseDir( gOpts.rm_options.repoPackagesCachePath );
        }

        if ( ! mayuse )
        {
          err_r = str::Format(_("Insufficient privileges to use download directory '%s'.")) % gOpts.rm_options.repoPackagesCachePath;
          return ( ZYPPER_EXIT_ERR_PRIVILEGES );
        }
      }
      return ZYPPER_EXIT_OK;
    }
  };
} // namespace

DownloadCmd::DownloadCmd(std::vector<std::string> &&commandAliases_r ) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("download [OPTIONS] <PACKAGES>..."),
    // translators: command summary: download
    _("Download rpms specified on the commandline to a local directory."),
    (
      str::Format( "%1%\n\n%2%" )
      // translators: command description
      % _("Download rpms specified on the commandline to a local directory. Per default packages are downloaded to the libzypp package cache (/var/cache/zypp/packages;"
          " for non-root users $XDG_CACHE_HOME/zypp/packages), but this can be changed by using the global --pkg-cache-dir option.")
      // translators: command description
      % _("In XML output a <download-result> node is written for each package zypper tried to download."
          " Upon success the local path is is found in 'download-result/localpath@path'.")
    ),
    ResetRepoManager | InitTarget | InitRepos | LoadResolvables
  )
{  }

ZyppFlags::CommandGroup DownloadCmd::cmdOptions() const
{
  auto that = const_cast<DownloadCmd *>(this);
  return {{
      {
        "all-matches", '\0',  ZyppFlags::NoArgument,
         ZyppFlags::BoolType( &that->_allMatches, ZyppFlags::StoreTrue, _allMatches ),
         // translators: --all-matches
         _("Download all versions matching the commandline arguments. Otherwise only the best version of each matching package is downloaded.")
      }
  }};
}

void DownloadCmd::doReset()
{
  _allMatches = false;
}

std::vector<BaseCommandConditionPtr> DownloadCmd::conditions() const
{
  return {
    std::make_shared<EnsureWriteableCacheCondition>()
  };
}

int DownloadCmd::execute( Zypper &zypper , const std::vector<std::string> &positionalArgs_r )
{
    if ( positionalArgs_r.empty() )
    {
      report_required_arg_missing( zypper.out(), help() );
      return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
    }

    typedef ui::SelectableTraits::AvailableItemSet AvailableItemSet;
    typedef std::map<IdString,AvailableItemSet> Collection;
    Collection collect;


    // parse package arguments
    PackageArgs::Options argopts;
    PackageArgs args( positionalArgs_r, ResKind::package, argopts );
    for ( const auto & pkgspec : args.dos() )
    {
      const Capability & cap( pkgspec.parsed_cap );
      const CapDetail & capDetail( cap.detail() );

      PoolQuery q;
      q.setMatchGlob();
      q.setUninstalledOnly();
      q.addKind( ResKind::package );
      if ( ! pkgspec.repo_alias.empty() )
	q.addRepo( pkgspec.repo_alias );
      //for_ ( it, repos.begin(), repos.end() ) q.addRepo(*it);
      // try matching names first
      q.addDependency( sat::SolvAttr::name,
		       capDetail.name().asString(),
		       capDetail.op(),			// defaults to Rel::ANY (NOOP) if no versioned cap
		       capDetail.ed(),
		       Arch( capDetail.arch() ) );	// defaults Arch_empty (NOOP) if no arch in cap

      // no natch on names, do try provides
      if ( q.empty() )
	q.addDependency( sat::SolvAttr::provides,
			 capDetail.name().asString(),
			 capDetail.op(),		// defaults to Rel::ANY (NOOP) if no versioned cap
			 capDetail.ed(),
			 Arch( capDetail.arch() ) );	// defaults Arch_empty (NOOP) if no arch in cap

      if ( q.empty() || !isPackageType( *q.begin() ) )
      {
	// translators: Label text; is followed by ': cmdline argument'
	zypper.out().warning( str::Str() << _("Argument resolves to no package") << ": " << pkgspec.orig_str );
	continue;
      }

      AvailableItemSet & avset( collect[(*q.begin()).ident()] );
      zypper.out().info( str::Str() << pkgspec.orig_str << ": ", Out::HIGH );
      for_( it, q.begin(), q.end() )
      {
	avset.insert( PoolItem( *it ) );
	zypper.out().info( str::Str() << "  " << (*it).asUserString(), Out::HIGH );
      }
    }

    if ( collect.empty() )
    {
      zypper.out().info( _("Nothing to do.") );
      return ZYPPER_EXIT_OK;
    }

    unsigned total = 0;
    if ( _allMatches )
    {
      zypper.out().info( str::Str() << _("No prune to best version.") << " (--all-matches)" );
     for ( const auto & ent : collect )
	total += ent.second.size();
    }
    else
    {
      zypper.out().info( _("Prune to best version..."), Out::HIGH );
      total = collect.size();
    }

    if (DryRunSettings::instance().isEnabled() )
    {
      zypper.out().info( str::Str() << _("Not downloading anything...") << " (--dry-run)" );
    }

    // Prepare the package cache. Pass all items requiring download.
    target::CommitPackageCache packageCache;

    unsigned current = 0;
    zypper.runtimeData().commit_pkgs_total = total; // fix DownloadResolvableReport total counter
    for ( const auto & ent : collect )
    {
      for ( const auto & pi : ent.second )
      {
	++current;

	if ( ! isCached( pi ) )
	{
	  if ( !DryRunSettings::instance().isEnabled() )
	  {
	    ManagedFile localfile;
	    try
	    {
	      Out::ProgressBar report( zypper.out(), Out::ProgressBar::noStartBar, pi.asUserString(), current, total );
	      report.error(); // error if provideSrcPackage throws
	      Out::DownloadProgress redirect( report );
	      localfile = packageCache.get( pi );
	      report.error( false );
	      report.print( cachedLocation( pi ).asString() );
	    }
	    catch ( const Out::Error & error_r )
	    {
	      error_r.report( zypper );
	    }
	    catch ( const AbortRequestException & ex )
	    {
	      ZYPP_CAUGHT( ex );
	      zypper.out().error( ex.asUserString() );
	      break;
	    }
	    catch ( const Exception & exp )
	    {
	      // TODO: Need class Out::Error support for exceptions
	      ERR << exp << endl;
	      zypper.out().error( exp,
				   str::Format(_("Error downloading package '%s'.")) % pi.asUserString() );
	    }

	    //DBG << localfile << endl;
	    localfile.resetDispose();
	    if ( zypper.out().typeXML() )
	      logXmlResult( pi, localfile );

	    if ( zypper.exitRequested() )
	      return ZYPPER_EXIT_ON_SIGNAL;
	  }
	  else
	  {
	    zypper.out().info( str::Str()
	                        << str::Format(_("Not downloading package '%s'.")) % pi.asUserString()
				<< " (--dry-run)" );
	  }
	}
	else
	{
	  const Pathname &  localfile( cachedLocation( pi ) );
	  Out::ProgressBar report( zypper.out(), localfile.asString(), current, total );
	  if ( zypper.out().typeXML() )
	    logXmlResult( pi, localfile );
	}

	if ( !_allMatches )
	  break;	// first==best version only.
      }
    }
    return ZYPPER_EXIT_OK;
}
