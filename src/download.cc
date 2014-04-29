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
#include <zypp/PoolQuery.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/ui/SelectableTraits.h>
#include <zypp/target/CommitPackageCache.h>

#include "Zypper.h"
#include "PackageArgs.h"
#include "Table.h"
#include "download.h"
#include "callbacks/media.h"

///////////////////////////////////////////////////////////////////
// DownloadOptions
///////////////////////////////////////////////////////////////////

inline std::ostream & operator<<( std::ostream & str, const DownloadOptions & obj )
{ return str << boost::format( "{%1%}" ) % (obj._dryrun ? "(dry-run)" : "" ); }

///////////////////////////////////////////////////////////////////
namespace
{
  ///////////////////////////////////////////////////////////////////
  /// \class DownloadImpl
  /// \brief Implementation of download commands.
  ///////////////////////////////////////////////////////////////////
  class DownloadImpl
  {
  public:
    DownloadImpl( Zypper & zypper_r )
    : _zypper( zypper_r )
    , _options( _zypper.commandOptionsAs<DownloadOptions>() )
    { MIL << "Download " << _options << endl; }

  public:
    void download();

  private:
    Zypper & _zypper;				//< my Zypper
    shared_ptr<DownloadOptions> _options;	//< my Options
  };
  ///////////////////////////////////////////////////////////////////

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

  void DownloadImpl::download()
  {
    typedef ui::SelectableTraits::AvailableItemSet AvailableItemSet;
    typedef std::map<IdString,AvailableItemSet> Collection;
    Collection collect;

    // parse package arguments
    PackageArgs::Options argopts;
    PackageArgs args( ResKind::package, argopts );
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

      if ( q.empty() )
      {
	// translators: Label text; is followed by ': cmdline argument'
	_zypper.out().warning( str::Str() << _("Argument resolves to no package") << ": " << pkgspec.orig_str );
	continue;
      }

      AvailableItemSet & avset( collect[(*q.begin()).ident()] );
      _zypper.out().info( str::Str() << pkgspec.orig_str << ": ", Out::HIGH );
      for_( it, q.begin(), q.end() )
      {
	avset.insert( PoolItem( *it ) );
	_zypper.out().info( str::Str() << "  " << (*it).asUserString(), Out::HIGH );
      }
    }

    if ( collect.empty() )
    {
      _zypper.out().info( _("Nothing to do.") );
      return;
    }

    unsigned total = 0;
    if ( _options->_allmatches )
    {
      _zypper.out().info( str::Str() << _("No prune to best version.") << " (--all-matches)" );
     for ( const auto & ent : collect )
	total += ent.second.size();
    }
    else
    {
      _zypper.out().info( _("Prune to best version..."), Out::HIGH );
      total = collect.size();
    }

    if ( _options->_dryrun )
    {
      _zypper.out().info( str::Str() << _("Not downloading anything...") << " (--dry-run)" );
    }

    // Prepare the package cache. Pass all items requiring download.
    target::CommitPackageCache packageCache( _zypper.globalOpts().root_dir );
    //packageCache.setCommitList( steps.begin(), steps.end() );

    unsigned current = 0;
    for ( const auto & ent : collect )
    {
      for ( const auto & pi : ent.second )
      {
	++current;
	Package::constPtr pkg( pi->asKind<Package>() );
	if ( ! pkg->isCached() )
	{
	  if ( !_options->_dryrun )
	  {
	    ManagedFile localfile;
	    try
	    {
	      Out::ProgressBar report( _zypper.out(), Out::ProgressBar::noStartBar, pi.satSolvable().asUserString(), current, total );
	      report.error(); // error if provideSrcPackage throws
	      Out::DownloadProgress redirect( report );
	      localfile = packageCache.get( pi );
	      report.error( false );
	      report.print( pkg->cachedLocation().asString() );
	    }
	    catch ( const Out::Error & error_r )
	    {
	      error_r.report( _zypper );
	    }
	    catch ( const Exception & exp )
	    {
	      // TODO: Need class Out::Error support for exceptions
	      ERR << exp << endl;
	      _zypper.out().error( exp,
				   boost::str( boost::format(_("Error downloading package '%s'.") ) % pi.satSolvable().asUserString() ) );
	    }

	    //DBG << localfile << endl;
	    localfile.resetDispose();
	    if ( _zypper.out().typeXML() )
	      logXmlResult( pi, localfile );

	    if ( _zypper.exitRequested() )
	      throw( Out::Error( ZYPPER_EXIT_ON_SIGNAL ) );
	  }
	}
	else
	{
	  const Pathname &  localfile( pkg->cachedLocation() );
	  Out::ProgressBar report( _zypper.out(), localfile.asString(), current, total );
	  if ( _zypper.out().typeXML() )
	    logXmlResult( pi, localfile );
	}

	if ( !_options->_allmatches )
	  break;	// first==best version only.
      }
    }

    // finished
    cout << endl;
    if ( _zypper.exitCode() != ZYPPER_EXIT_OK )
      _zypper.out().info(_("Finished with error.") );
    else
      _zypper.out().info(_("Done.") );
  }

} // namespace
///////////////////////////////////////////////////////////////////

int download( Zypper & zypper_r )
{
  try
  {
    DownloadImpl( zypper_r ).download();
  }
  catch ( const Out::Error & error_r )
  {
    return error_r.report( zypper_r );
  }
  return zypper_r.exitCode();
}
