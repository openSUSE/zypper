/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include <zypp/base/LogTools.h>
#include <zypp/ResPool.h>
#include <zypp/Package.h>
#include <zypp/SrcPackage.h>
#include <zypp/target/rpm/RpmHeader.h>
#include <zypp/repo/SrcPackageProvider.h>
// #include <zypp/ZYppCallbacks.h>

#include "Zypper.h"
#include "Table.h"
#include "source-download.h"

///////////////////////////////////////////////////////////////////
// SourceDownloadOptions
///////////////////////////////////////////////////////////////////

const Pathname SourceDownloadOptions::_defaultDirectory( "/var/cache/zypper/source-download" );
const std::string SourceDownloadOptions::_manifestName( "MANIFEST" );

inline std::ostream & operator<<( std::ostream & str, const SourceDownloadOptions & obj )
{
  return str << boost::format( "{%1%|%2%%3%}" )
	      % obj._directory
// 	      % (obj._manifest ? 'M' : 'm' )
	      % (obj._delete ? 'D' : 'd' )
	      % (obj._dryrun ? "(dry-run)" : "" );
}

///////////////////////////////////////////////////////////////////
namespace
{

  ///////////////////////////////////////////////////////////////////
  /// \class SourceDownloadImpl
  /// \brief Implementation of source-download commands.
  ///////////////////////////////////////////////////////////////////
  class SourceDownloadImpl
  {
  public:
    SourceDownloadImpl( Zypper & zypper_r )
    : _zypper( zypper_r )
    , _options( _zypper.commandOptionsAs<SourceDownloadOptions>() )
    , _dnlDir( Pathname::assertprefix( _zypper.globalOpts().root_dir, _options->_directory ) )
    { MIL << "SourceDownload " << _options << endl; }

  public:

    ///////////////////////////////////////////////////////////////////
    /// \class SourceDownloadImpl::SourcePkg
    /// \brief A Manifest entry.
    ///////////////////////////////////////////////////////////////////
    /** Manifest entry */
    struct SourcePkg
    {
      enum Status
      {
	S_EMPTY,	//< !needed && !downloaded
	S_SUPERFLUOUS,	//< !needed &&  downloaded
	S_MISSING,	//<  needed && !downloaded
	S_OK,		//<  needed &&  downloaded
      };

      SourcePkg( const std::string & longname_r = std::string() )
      : _longname( longname_r ) {}

      bool needed() const
      { return !_packages.empty(); }

      bool downloaded() const
      { return !_localFile.empty(); }

      Status status() const
      {
	if ( needed() )
	  return downloaded() ? S_OK : S_MISSING;
	return downloaded() ? S_SUPERFLUOUS : S_EMPTY;
      }

      PoolItem lookupSrcPackage()
      {
	if ( ! _srcPackage )
	{
	  ResPool pool( ResPool::instance() );
	  Package::constPtr pkg( _packages.front()->asKind<Package>() );
	  for_( it, pool.byIdentBegin<SrcPackage>( pkg->sourcePkgName() ), pool.byIdentEnd<SrcPackage>( pkg->sourcePkgName() ) )
	  {
	    if ( (*it)->edition() == pkg->sourcePkgEdition() )
	    {
	      _srcPackage = *it;
	      break;
	    }
	  }
	}
	return _srcPackage;
      }

      std::string _longname;	//< Key: name-version-release.type
      std::string _localFile;	//< name of srpm if downloaded
      PoolItem _srcPackage;	//< available SrcPackage providning the srpm
      std::vector<PoolItem> _packages;	//< installed Packages built from this srpm

      static std::string makeLongname( const std::string & name_r, Edition edition_r, bool nosrc_r )
      { return str::Str() << name_r << '-' << edition_r << '.' << (nosrc_r ? "nosrc" : "src"); }
   };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /// \class SourceDownloadImpl::Manifest
    /// \brief A set of SourcePkg
    ///////////////////////////////////////////////////////////////////
    struct Manifest : public std::map<std::string, SourceDownloadImpl::SourcePkg>
    {
      typedef std::map<SourcePkg::Status, DefaultIntegral<unsigned,0U> > StatusMap;

      /** Return \ref SourcePkg for \a key_r (assert \c SourcePkg::_longname is set) */
      SourcePkg & get( const std::string & key_r )
      {
	SourcePkg & ret( operator[]( key_r ) );
	if ( ret._longname.empty() )
	  ret._longname = key_r;
	return ret;
      }

      void updateStatus( StatusMap & status_r ) const
      {
	StatusMap status;
	for_( it, begin(), end() )
	{
	  const SourcePkg & src( it->second );
	  ++status[src.status()];
	}
	status_r.swap( status );
      }
    };
    ///////////////////////////////////////////////////////////////////

  public:
    void sourceDownload();

  private:
    /** Startup and build manifest. */
    void buildManifest();

    std::ostream & dumpManifestSumary( std::ostream & str, Manifest::StatusMap & status );
    std::ostream & dumpManifestTable( std::ostream & str );

  private:
    Zypper & _zypper;				//< my Zypper
    shared_ptr<SourceDownloadOptions> _options;	//< my Options
  private:
    Pathname _dnlDir;	//< download directory (incl. root prefix)
    Manifest _manifest;
    DefaultIntegral<unsigned,0U> _installedPkgCount;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SourceDownloadImpl::SourcePkg::Status String representation */
  inline std::string asString( SourceDownloadImpl::SourcePkg::Status obj )
  {
    switch ( obj )
    {
      case SourceDownloadImpl::SourcePkg::S_EMPTY:	return "?"; break;
      case SourceDownloadImpl::SourcePkg::S_SUPERFLUOUS:return "+"; break;
      case SourceDownloadImpl::SourcePkg::S_MISSING:	return "-"; break;
      case SourceDownloadImpl::SourcePkg::S_OK:		return " "; break;
    }
    return "?";
  }

  /** \relates SourceDownloadImpl::SourcePkg::Status Stream output */
  inline std::ostream & operator<<( std::ostream & str, SourceDownloadImpl::SourcePkg::Status obj )
  { return str << asString( obj ); }

  /** \relates SourceDownloadImpl::SourcePkg Stream output */
  inline std::ostream & operator<<( std::ostream & str, const SourceDownloadImpl::SourcePkg & obj )
  {
    str << obj.status() << " " << obj._longname << endl;
    str << "  spkg:  " << obj._srcPackage << endl;
    str << "  pkgs:  " << obj._packages.size();
    return str;
  }

  ///////////////////////////////////////////////////////////////////
  /// class SourceDownloadImpl
  ///////////////////////////////////////////////////////////////////

  void SourceDownloadImpl::buildManifest()
  {
    PathInfo pi( _dnlDir );
    if ( pi.error() == ENOENT )
    {
      // try to create it on the fly
      filesystem::assert_dir( pi.path() );
      pi();
    }
    DBG << "Download directory: " << pi << endl;

    if ( ! pi.isDir() )
    {
      ERR << "Can't create or access download directory" << endl;
      throw( Out::Error( ZYPPER_EXIT_ERR_BUG,
			 boost::format(_("Can't create or access download directory '%s'.")) % pi.asString(),
			 Errno( pi.error() ).asString() ) );
      return;
    }

    if ( ! ( pi.userMayR() && ( _options->_dryrun || pi.userMayW() ) ) )
    {
      ERR << "Download directory is not readable." << endl;
      throw( Out::Error( ZYPPER_EXIT_ERR_PRIVILEGES,
			 boost::format(_("Insufficient privileges to use download directory '%s'.")) % pi.asString() ) );
      return;
    }

    _zypper.out().info(
      boost::format(_("Using download directory at '%s'.")) % pi.asString()
    );

    // scan download directory to manifest
    {
      std::list<std::string> todolist;
      int ret = readdir( todolist, pi.path(), /*dots*/false );
      if ( ret != 0 )
      {
	ERR << "Failed to read download directory." << endl;
	throw( Out::Error( ZYPPER_EXIT_ERR_BUG,
			   _("Failed to read download directory"),
			   Errno().asString() ) );
	return;
      }

      Out::ProgressBar report( _zypper.out(), _("Scanning download directory") );
      report->range( todolist.size() );
      for ( const auto & file : todolist )
      {
	report->incr();	// fast enough to count in advance.

	if ( file == _options->_manifestName )
	  continue;

	using target::rpm::RpmHeader;
	Pathname path( pi.path() / file );
	RpmHeader::constPtr pkg( RpmHeader::readPackage( path, RpmHeader::NOVERIFY ) );

	if ( ! ( pkg && pkg->isSrc() ) )
	  continue;

	SourcePkg & spkg( _manifest.get( SourcePkg::makeLongname( pkg->tag_name(), pkg->tag_edition(), pkg->isNosrc() ) ) );
	spkg._localFile = file;
      }
    }

    // scan installed packages to manifest
    {
      if ( _zypper.defaultLoadSystem( _options->_dryrun ? Zypper::NO_REPOS : Zypper::LoadSystemFlags() ) != ZYPPER_EXIT_OK )
      {
	ERR << "Startup returns " << _zypper.exitCode() << endl;
	throw( Out::Error(_("Failed to read download directory"),
			  Errno().asString() ) );
      }

      Out::ProgressBar report( _zypper.out(), _("Scanning installed packages") );
      ResPool pool( ResPool::instance() );
      for_( it, pool.byKindBegin<Package>(), pool.byKindEnd<Package>() )
      {
	if ( ! it->status().isInstalled() )
	  continue;

	SourcePkg & spkg( _manifest.get( (*it)->asKind<Package>()->sourcePkgLongName() ) );
	spkg._packages.push_back( *it );
	++_installedPkgCount;	// on the fly count installed packages
      }
    }
  }

  std::ostream & SourceDownloadImpl::dumpManifestSumary( std::ostream & str, Manifest::StatusMap & status )
  {
    {
      Table t;
      t.lineStyle( none );
      { TableRow tr; tr << _("Installed packages:")
                        << str::numstring( _installedPkgCount, 5 )
                        ; t << tr; }
      { TableRow tr; tr << _("Required source packages:")
                        << str::numstring( _manifest.size() - status[SourcePkg::S_SUPERFLUOUS] - status[SourcePkg::S_EMPTY], 5 )
                        ; t << tr; }
      str << t << endl;
    }
    {
      Table t;
      t.lineStyle( none );
      { TableRow tr; tr << '[' + asString( SourcePkg::S_OK ) + ']'
                        << _("Required source packages available in download directory:")
                        << str::numstring( status[SourcePkg::S_OK], 5 );
                        t << tr; }
      { TableRow tr; tr << '[' + asString( SourcePkg::S_MISSING ) + ']'
                        << _("Required source packages to be downloaded:")
                        << str::numstring( status[SourcePkg::S_MISSING], 5 );
                        t << tr; }
      { TableRow tr; tr << '[' + asString( SourcePkg::S_SUPERFLUOUS ) + ']'
                        << _("Superfluous source packages in download directory:")
                        << str::numstring( status[SourcePkg::S_SUPERFLUOUS], 5 );
                        t << tr; }
      str << t << endl;
    }
    return str;
  }


  std::ostream & SourceDownloadImpl::dumpManifestTable( std::ostream & str )
  {
    Table t;
    TableHeader th;
    // translators: table headers
    th << "#" << _("Source package") << _("Installed package");
    t << th;

    for ( const auto & item : _manifest )
    {
      const SourcePkg & spkg( item.second );
      std::string l1( asString( spkg.status() ) );
      std::string l2( spkg._longname );
      if ( spkg._packages.empty() )
      {
	TableRow tr;
	tr << l1 << l2 << "-----";
	t << tr;
      }
      else
      {
	for ( const auto & pkg : spkg._packages )
	{
	  TableRow tr;
	  tr << l1 << l2 << pkg.satSolvable();
	  t << tr;
	  if ( ! l1.empty() )
	  {
	    l1.clear();
	    l2 = "  --\"--";
	  }
	}
      }
    }
    str << t << endl;
    return str;
  }


  void SourceDownloadImpl::sourceDownload()
  {
    buildManifest();
    Manifest::StatusMap status;
    _manifest.updateStatus( status );

    cout << endl;
    dumpManifestSumary( cout, status );

    if ( _options->_dryrun )
    {
      if ( _zypper.out().verbosity() > Out::NORMAL )
      {
	dumpManifestTable( cout );
      }
      else
      {
	_zypper.out().info(_("Use '--verbose' option for a full list of required source packages.") );
      }
      return;	// --> dry run ends here.
    }

    // delete superfluous source packages

    if ( status[SourcePkg::S_SUPERFLUOUS] && _options->_delete )
    {
      Out::ProgressBar report( _zypper.out(), _("Deleting superfluous source packages") );
      report->range( status[SourcePkg::S_SUPERFLUOUS] );
      for ( auto & item : _manifest )
      {
	SourcePkg & spkg( item.second );
	if ( spkg.status() != SourcePkg::S_SUPERFLUOUS )
	  continue;

	int res = filesystem::unlink( _dnlDir / spkg._localFile );
	if ( res != 0 )
	{
	  throw( Out::Error( boost::format(_("Failed to remove source package '%s'") ) % (_dnlDir / spkg._localFile),
			    Errno().asString() ) );
	}
	MIL << spkg << endl;
	spkg._localFile.clear();
	DBG << spkg << endl;
	report->incr();
      }
    }
    else
    {
      std::string msg(_("No superfluous source packages to delete.") );
      if ( status[SourcePkg::S_SUPERFLUOUS] )
	msg += " (--no-delete)";
      _zypper.out().info( msg );
    }

    // download missing packages

    if ( status[SourcePkg::S_MISSING] )
    {
      _zypper.out().info(_("Downloading required source packages...") );
      repo::RepoMediaAccess access;
      repo::SrcPackageProvider prov( access );
      unsigned current = 0;
      for ( auto & item : _manifest )
      {
	SourcePkg & spkg( item.second );
	if ( spkg.status() != SourcePkg::S_MISSING )
	  continue;
	++current;

	try
	{
	  Out::ProgressBar report( _zypper.out(), spkg._longname, current, status[SourcePkg::S_MISSING] );

	  if ( ! spkg.lookupSrcPackage() )
	  {
	    report.error();
	    throw( Out::Error( ZYPPER_EXIT_ERR_BUG,
			       boost::format(_("Source package '%s' is not provided by any repository.") ) % spkg._longname ) );
	  }
	  report.print( str::form( "%s (%s)",  spkg._longname.c_str(), spkg._srcPackage->repository().name().c_str() ) );
	  MIL << spkg._srcPackage << endl;

	  ManagedFile localfile;
	  {
	    report.error(); // error if provideSrcPackage throws
	    Out::DownloadProgress redirect( report );
	    localfile = prov.provideSrcPackage( spkg._srcPackage->asKind<SrcPackage>() );
	    DBG << localfile << endl;
	    report.error( false );
	  }

	  if ( filesystem::hardlinkCopy( localfile, _dnlDir / (spkg._longname+".rpm") ) != 0 )
	  {
	    ERR << "Can't hardlink/copy " << localfile << " to " <<  (_dnlDir / spkg._longname) << endl;
	    report.error();
	    throw( Out::Error( ZYPPER_EXIT_ERR_BUG,
			       boost::format(_("Error downloading source package '%s'.") ) % spkg._longname ),
			       Errno().asString() );
	  }
	  spkg._localFile = spkg._longname;
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
			       boost::str( boost::format(_("Error downloading source package '%s'.") ) % spkg._longname ) );

	  //throw( Out::Error( ZYPPER_EXIT_ERR_BUG ) );
	}

	if ( _zypper.exitRequested() )
	  throw( Out::Error( ZYPPER_EXIT_ON_SIGNAL ) );
      }
    }
    else
    {
      _zypper.out().info(_("No source packages to download.") );
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

int sourceDownload( Zypper & zypper_r )
{
  try
  {
    SourceDownloadImpl( zypper_r ).sourceDownload();
  }
  catch ( const Out::Error & error_r )
  {
    return error_r.report( zypper_r );
  }
  return zypper_r.exitCode();
}
