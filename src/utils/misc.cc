/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <sstream>
#include <iostream>
#include <unistd.h>          // for getcwd()

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/Easy.h>
#include <zypp/base/Regex.h>
#include <zypp/media/MediaManager.h>
#include <zypp/ExternalProgram.h>
#include "zypp/parser/ProductFileReader.h"

#include <zypp/ZYpp.h>
#include <zypp/Target.h>
#include <zypp/PoolItem.h>
#include <zypp/Product.h>
#include <zypp/Pattern.h>

#include "main.h"
#include "Zypper.h"
#include "Table.h"
#include "repos.h"

#include "utils/misc.h"
#include "utils/XmlFilter.h"

extern ZYpp::Ptr God;

bool runningOnEnterprise()
{
  bool ret = false;
  if ( Target_Ptr target = God->getTarget() )
    if ( Product::constPtr platform = target->baseProduct() )
    {
      static const CpeId enterprise( "cpe:/o:suse:sle%02" );
      if ( compare( platform->cpeId(), enterprise, SetRelation::subset ) )
	ret = true;
    }
  return ret;
}

// ----------------------------------------------------------------------------

ResKind string_to_kind( std::string skind )
{
  skind = str::toLower( skind );
  if ( skind == "package" )
    return ResKind::package;
  if ( skind == "pattern" )
    return ResKind::pattern;
  if ( skind == "product" )
    return ResKind::product;
  if ( skind == "patch" )
    return ResKind::patch;
  if ( skind == "srcpackage" )
    return ResKind::srcpackage;
  if ( skind == "application" )
    return ResKind::application;
  // not recognized
  return ResKind::nokind;
}

// ----------------------------------------------------------------------------

ResKindSet kindset_from( const std::list<std::string> & kindstrings )
{
  ResKind kind;
  ResKindSet kinds;
  for_( it, kindstrings.begin(), kindstrings.end() )
  {
    kind = string_to_kind(*it);
    if ( kind == ResKind::nokind )
      continue;
    kinds.insert( kind );
  }
  return kinds;
}

// ----------------------------------------------------------------------------

std::string kind_to_string_localized( const ResKind & kind, unsigned long count )
{
  if ( kind == ResKind::package )
    return PL_("package", "packages", count);
  if ( kind == ResKind::pattern )
    return PL_("pattern", "patterns", count);
  if ( kind == ResKind::product )
    return PL_("product", "product", count);
  if ( kind == ResKind::patch )
    return PL_("patch", "patches", count);
  if ( kind == ResKind::srcpackage )
    return PL_("srcpackage", "srcpackages", count);
  if ( kind == ResKind::application )
    return PL_("application", "applications", count);
  // default
  return PL_("resolvable", "resolvables", count);
}

// ----------------------------------------------------------------------------
// PATCH related strings for various purposes
// ----------------------------------------------------------------------------
std::string i18nPatchStatus( const PoolItem & pi_r )
{
  // Patch status: i18n + color
  static const char * tUnwanted		= /* translator: patch status */ _("unwanted");
  static const char * tOptional		= /* translator: patch status */ _("optional");
  static const char * tNeeded		= /* translator: patch status */ _("needed");
  static const char * tApplied		= /* translator: patch status */ _("applied");
  static const char * tNotneeded	= /* translator: patch status */ _("not needed");
  static const char * tUndetermined	= /* translator: patch status */ _("undetermined");	// pi_r is no patch!
  switch ( pi_r.status().validate() )
  {
    case ResStatus::BROKEN:
      if ( pi_r.isUnwanted() )
	// Translator: Patch status: needed, optional, unwanted, applied, not needed
	return HIGHLIGHTString( tUnwanted ).str();
      if ( Zypper::instance().globalOpts().exclude_optional_patches && pi_r->asKind<Patch>()->categoryEnum() == Patch::CAT_OPTIONAL )
	return LOWLIGHTString( tOptional ).str();
      return tNeeded;
      break;
    case ResStatus::SATISFIED:		return POSITIVEString( tApplied ).str();	break;
    case ResStatus::NONRELEVANT:	return POSITIVEString( tNotneeded ).str();	break;

    case ResStatus::UNDETERMINED:	// fall through
    default:
      break;
  }
  return NEGATIVEString( tUndetermined ).str();
}

const char * textPatchStatus( const PoolItem & pi_r )
{
  // Patch status: plain text noWS
  static const char * tUnwanted		= "unwanted";
  static const char * tOptional		= "optional";
  static const char * tNeeded		= "needed";
  static const char * tApplied		= "applied";
  static const char * tNotneeded	= "not-needed";
  static const char * tUndetermined	= "undetermined";	// pi_r is no patch!

  switch ( pi_r.status().validate() )
  {
    case ResStatus::BROKEN:
      if ( pi_r.isUnwanted() )
	return tUnwanted;
      if ( Zypper::instance().globalOpts().exclude_optional_patches && pi_r->asKind<Patch>()->categoryEnum() == Patch::CAT_OPTIONAL )
	return tOptional;
      return tNeeded;
      break;
    case ResStatus::SATISFIED:		return tApplied;	break;
    case ResStatus::NONRELEVANT:	return tNotneeded;	break;

    case ResStatus::UNDETERMINED:	// fall through
    default:
      break;
  }
  return tUndetermined;
}

std::string patchHighlightCategory( const Patch & patch_r )
{
  std::string ret;
  switch ( patch_r.categoryEnum() )	// switch by enum as it matches multiple strings (optional==feature==enhancement)
  {
    case Patch::CAT_SECURITY:
      ret = HIGHLIGHTString( patch_r.category() ).str();
      break;
    case Patch::CAT_OPTIONAL:
      ret = LOWLIGHTString( patch_r.category() ).str();
      break;
      // else: fallthrough
    default:
      ret = patch_r.category();
      break;
  }
  return ret;
}

std::string patchHighlightSeverity( const Patch & patch_r )
{
  std::string ret;
  if ( patch_r.isSeverity( Patch::SEV_CRITICAL ) )
    ret = HIGHLIGHTString( patch_r.severity() ).str();
  else
    ret = patch_r.severity();
  return ret;
}

std::string patchInteractiveFlags( const Patch & patch_r )
{
  static const Patch::InteractiveFlags kRestart( 0x1000 ); // an artificial flag to indicate update stack patches in 'Interactive'
  std::string ret;

  Patch::InteractiveFlags flags = patch_r.interactiveFlags();
  if ( patch_r.restartSuggested() )
    flags |= kRestart;

  if ( flags )
  {
    ret = stringify( flags, {
      { Patch::Reboot,	"reboot" },
      { Patch::Message,	"message" },
      { Patch::License,	"licence" },
      { kRestart,	HIGHLIGHTString( "restart" ).str() }
    }, "", ",", "" );
  }
  else
  {
    ret = "---";
  }
  return ret;
}

/** Default format patches table */
FillPatchesTable::FillPatchesTable( Table & table_r, TriBool inst_notinst_r )
: _table( &table_r )
, _inst_notinst( inst_notinst_r )
{
  table_r << ( TableHeader()
  << _("Repository")
  << _("Name")
  << _("Category")
  << _("Severity")
  << _("Interactive")
  << _("Status")
  << _("Summary")
  );
  table_r.defaultSortColumn( 1 );	// by Name
  // if ( ! Zypper::instance().globalOpts().no_abbrev ) table.allowAbbrev( 6 );	// Summary
};

bool FillPatchesTable::operator()( const PoolItem & pi_r ) const
{
  if ( pi_r.isSatisfied() && _inst_notinst == false )
  { return true; }	// print only not installed
  if ( !pi_r.isSatisfied() && _inst_notinst == true )
  { return true; }	// print only installed

  Patch::constPtr patch = asKind<Patch>(pi_r);

  *_table << ( TableRow()
  /* Repository		*/ << patch->repoInfo().asUserString()
  /* Name		*/ << patch->name()
  /* Category		*/ << patchHighlightCategory( *patch )
  /* Severity		*/ << patchHighlightSeverity( *patch )
  /* Interactive	*/ << patchInteractiveFlags( *patch )
  /* Status		*/ << i18nPatchStatus( pi_r )
  /* Summary		*/ << patch->summary()
  );
  return true;
}


/** Patches table when searching for issues */
FillPatchesTableForIssue::FillPatchesTableForIssue( Table & table_r )
: _table( &table_r )
{
  table_r << ( TableHeader()
  //<< _("Repository")
  << _("Issue")
  << _("No.")
  << _("Patch")	// Name
  << _("Category")
  << _("Severity")
  << _("Interactive")
  << _("Status")
  << _("Summary")
  );
  table_r.defaultSortColumn( 2 );	// by Name
}
bool FillPatchesTableForIssue::operator()( const PoolItem & pi_r, std::string issue_r, std::string issueNo_r ) const
{
  Patch::constPtr patch = asKind<Patch>(pi_r);

  *_table << ( TableRow()
  ///* Repository	*/ << patch->repoInfo().asUserString()
  /* Issue		*/ << std::move(issue_r)
  /* No.		*/ << std::move(issueNo_r)
  /* Patch/Name		*/ << patch->name()
  /* Category		*/ << patchHighlightCategory( *patch )
  /* Severity		*/ << patchHighlightSeverity( *patch )
  /* Interactive	*/ << patchInteractiveFlags( *patch )
  /* Status		*/ << i18nPatchStatus( pi_r )
  /* Summary		*/ << patch->summary()
  );


  return true;
}

// ----------------------------------------------------------------------------

bool looks_like_url( const std::string& s )
{
  std::string::size_type pos = s.find (':');
  if ( pos != std::string::npos )
  {
    std::string scheme( s, 0, pos );
    if ( Url::isRegisteredScheme( scheme ) || scheme == "obs" )
      return true;
  }
  return false;
}

// ----------------------------------------------------------------------------

Url make_url( const std::string & url_s )
{
  Url u;
  std::string urlstr( url::encode( url_s, URL_SAFE_CHARS ) );

  if ( !url_s.empty() && !looks_like_url(url_s) )
  {
    DBG << "'" << url_s << "' does not look like a URL, trying to treat it like a local path" << endl;

    Pathname path;
    // make an url from absolute path
    if ( url_s[0] == '/' )
      path = url_s;
    // make absolute path url from relative path
    else
    {
      char buf[PATH_MAX];
      if ( ::getcwd(buf, PATH_MAX) != NULL )
      {
        DBG <<  "current working directory: " << buf << endl;
        path = std::string(buf) + "/" + url_s;
      }
    }

    if ( PathInfo(path).isExist() )
    {
      urlstr = "dir:" + url::encode( path.absolutename().asString(), "/" );
      MIL <<  "resulting url: " << urlstr << endl;
    }
    else
    {
      Zypper::instance().out().error(_("Specified local path does not exist or is not accessible."));
      ERR << "specified local path does not exist or is not accessible" << endl;
      return u;
    }
  }

  try {
    u = Url(urlstr);
  }
  catch ( const Exception & excpt_r )
  {
    ZYPP_CAUGHT( excpt_r );
    Zypper::instance().out().error( str::Str() << _("Given URI is invalid") << ": " << urlstr << " (" << excpt_r.asUserString() << ")" );
  }
  return u;
}

// ----------------------------------------------------------------------------

// in the Estonian locale, a-z excludes t, for example. #302525
// http://en.wikipedia.org/wiki/Estonian_alphabet
#define ALNUM "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0-9_"

// valid OBS project name regex from OBS' ApplicationController.valid_project_name? method:
// http://gitorious.org/opensuse/build-service/blobs/master/src/webui/app/controllers/application_controller.rb
#define OBS_PROJECT_NAME_RX "[" ALNUM "][-+" ALNUM "\\.:]+"

///////////////////////////////////////////////////////////////////
namespace
{
  parser::ProductFileData baseproductdata( const Pathname & root_r )
  {
    parser::ProductFileData ret;
    PathInfo baseproduct( Pathname::assertprefix( root_r, "/etc/products.d/baseproduct" ) );

    if ( baseproduct.isFile() )
    {
      try
      {
	ret = parser::ProductFileReader::scanFile( baseproduct.path() );
      }
      catch ( const Exception & excpt )
      {
	ZYPP_CAUGHT( excpt );
      }
    }
    return ret;
  }

} // namespace
///////////////////////////////////////////////////////////////////

Url make_obs_url( const std::string & obsuri, const Url & base_url, const std::string & default_platform )
{
  // obs-server ==> < base_url, default_platform >
  static std::map<std::string, std::pair<Url,std::string>> wellKnownServers({
    { "build.opensuse.org",	{ Url("https://download.opensuse.org/repositories/"),	std::string() } }
  });

  static str::regex obs_uri_rx("^obs://(" OBS_PROJECT_NAME_RX ")(/(.*)?)?$");
  str::smatch what;

  if ( str::regex_match( obsuri, what, obs_uri_rx ) )
  {
    std::string project( what[1] );
    std::string platform( what[3] );

    // in case the project matches a well known obs://server,
    // strip it and use it's base_url and platform defaults.
    // If the servers platform default is empty, use the
    // global one.
    auto it( wellKnownServers.find( project ) );
    if ( it != wellKnownServers.end() )
    {
      static str::regex obs_uri2_rx("^(" OBS_PROJECT_NAME_RX ")(/(.*)?)?$");
      if ( str::regex_match( platform, what, obs_uri2_rx ) )
      {
	project = what[1];
	platform = ( what[3].empty() ? it->second.second : what[3] );
      }
      else
	it = wellKnownServers.end();	// we stay with the 1st match, thus will use the global defaults
    }

    // Now bulid the url; ':' in project is replaced by ':/'
    Url ret( it == wellKnownServers.end() ? base_url : it->second.first );

    Pathname path( ret.getPathName() );
    path /= str::gsub( project, ":", ":/" );

    if ( platform.empty() )
    {
      Zypper & zypper( Zypper::instance() );

      if ( default_platform.empty() )
      {
	// Try to guess platform from baseproduct....
	const parser::ProductFileData & pdata( baseproductdata( zypper.globalOpts().root_dir ) );
	if ( pdata.empty() )
	{
	  // Guess failed:
			     // translators: don't translate '<platform>'
	  zypper.out().error(_("Unable to guess a value for <platform>."),
			     _("Please use obs://<project>/<platform>") );
	  zypper.out().info(str::form(_("Example: %s"), "obs://server:http/openSUSE_11.3"));
	  return Url();	// FAIL!
	}

	platform = pdata.name().asString();
	if ( platform == "openSUSE"  )
	{

	  if ( pdata.productline() == "Leap" )
	    platform += "_Leap_$releasever";
	  else if ( str::containsCI( pdata.summary(), "Tumbleweed" ) )
	    platform += "_Tumbleweed";
	  else
	    platform += "_$releasever";
	}
	else
	  platform += "_$releasever";

	zypper.out().info( "Guessed: platform = " + platform );
	path /= platform;
      }
      else
      {
	zypper.out().info( "zypper.conf: obs.platform = " + default_platform );
	path /= default_platform;
      }
    }
    else
      path /= platform;

    ret.setPathName( path.asString() );

    return ret;
  }
  else
  {
    Zypper::instance().out().error(_("Invalid OBS URI."), _("Correct form is obs://<project>/[platform]"));
    Zypper::instance().out().info(str::form(_("Example: %s"), "obs://server:http/openSUSE_11.3"));
  }

  return Url();
}

// ----------------------------------------------------------------------------

bool looks_like_rpm_file( const std::string & s )
{
  // don't even bother to check strings shorter than 4 chars.
  if ( s.size() <= 4 )
    return false;

  if ( s.rfind(".rpm") == s.size() - 4	// ends with .rpm
    || s.find("./") == 0 		// starts with ./ or ../ indicating
    || s.find("../") == 0 )		//  a local path
    return true;

  return false;
}

// ----------------------------------------------------------------------------

Pathname cache_rpm( const std::string & rpm_uri_str, const Pathname & cache_dir )
{
  Url rpmurl = make_url(rpm_uri_str);
  Pathname rpmpath(rpmurl.getPathName());
  rpmurl.setPathName(rpmpath.dirname().asString()); // directory
  rpmpath = rpmpath.basename(); // rpm file name

  try
  {
    media::MediaManager mm;
    media::MediaAccessId mid = mm.open(rpmurl);
    mm.attach(mid);

    mm.provideFile(mid, rpmpath.basename());
    Pathname localrpmpath = mm.localPath(mid, rpmpath.basename());
    filesystem::assert_dir(cache_dir);
    bool error =
      filesystem::hardlinkCopy(localrpmpath, cache_dir / localrpmpath.basename());

    mm.release(mid);
    mm.close(mid);

    if ( error )
    {
      Zypper::instance().out().error(
        _("Problem copying the specified RPM file to the cache directory."),
        _("Perhaps you are running out of disk space."));
      return Pathname();
    }
    return cache_dir / localrpmpath.basename();
  }
  catch (const Exception & e)
  {
    Zypper::instance().out().error(e,
        _("Problem retrieving the specified RPM file") + std::string(":"),
        _("Please check whether the file is accessible."));
  }

  return Pathname();
}

std::string & indent( std::string & text, int columns )
{
  std::string indent( columns, ' ');
  indent.insert( 0, 1, '\n' );
  DBG << "to: '" << indent << "'" << endl;
  text = str::gsub( text, "\n", indent );
  text.insert( 0, std::string( columns, ' ' ) );
  return text;
}

/**
 * \todo this is an ugly quick-hack code, let's do something reusable and maintainable in libzypp later
 */
// ----------------------------------------------------------------------------
std::string asXML(const Product & p, bool is_installed , const std::vector<std::string> &fwdTags )
{
  std::ostringstream str;
  {
    // Legacy: Encoded almost everything as attribute
    // Think about using subnodes for new stuff.
    xmlout::Node parent( str, "product", xmlout::Node::optionalContent, {
      { "name", 	p.name() },
      { "version",	p.edition().version() },
      { "release",	p.edition().release() },
      { "epoch",	p.edition().epoch() },
      { "arch",		p.arch() },
      { "vendor",	p.vendor() },
      { "summary",	p.summary() },
      { "repo",		p.repoInfo().alias() },
      // ^^^ common --- specific vvv
      { "productline",	p.productLine() },
      { "registerrelease",p.registerRelease() },
      { "shortname",	p.shortName() },
      { "flavor",	p.flavor() },
      { "isbase",	p.isTargetDistribution() },
      { "installed",	is_installed },
    } );

    if ( p.hasEndOfLife() )
      dumpAsXmlOn( *parent, p.endOfLife(), "endoflife" );
    dumpAsXmlOn( *parent, p.registerFlavor(), "registerflavor" );
    {
      const std::string & text( p.description() );
      if ( ! text.empty() )
	*xmlout::Node( *parent, "description" ) << xml::escape( text );
    }

    if ( is_installed && fwdTags.size() )
    {
      // literally forward tags found in the .prod file...
      xmlout::Node fwd( *parent, "xmlfwd", xmlout::Node::optionalContent );

      std::vector<std::string> tags;
      for ( const auto & tag : fwdTags )
      { tags.push_back( Pathname::assertprefix( "/product", tag ).asString() ); }

      const Pathname & proddir( Pathname::assertprefix( Zypper::instance().globalOpts().root_dir, "/etc/products.d" ) );
      try
      {
	XmlFilter::fwd( InputStream( proddir+p.referenceFilename() ), *fwd, std::move(tags)  );
      }
      catch ( const Exception & exp )
      {
	ZYPP_CAUGHT( exp );	// parse error
      }
    }
  }
  return str.str();
}

std::string asXML( const Pattern & p, bool is_installed )
{
  std::ostringstream str;
  {
    // Legacy: Encoded almost everything as attribute
    // Think about using subnodes for new stuff.
    xmlout::Node parent( str, "pattern", xmlout::Node::optionalContent, {
      { "name",		p.name() },
      { "version",	p.edition().version() },
      { "release",	p.edition().release() },
      { "epoch",	p.edition().epoch() },
      { "arch",		p.arch() },
      { "vendor",	p.vendor() },
      { "summary",	p.summary() },
      { "repo",		p.repoInfo().alias() },
      // ^^^ common --- specific vvv
      { "installed",	is_installed },
      { "uservisible",	p.userVisible() },
    } );
    {
      const std::string & text( p.description() );
      if ( ! text.empty() )
	*xmlout::Node( *parent, "description" ) << xml::escape( text );
    }
  }
  return str.str();
}

// ----------------------------------------------------------------------------

bool packagekit_running()
{
  bool result = false;
  const char* argv[] =
  {
    "dbus-send",
    "--system",
    "--dest=org.freedesktop.DBus",
    "--type=method_call",
    "--print-reply",
    "--reply-timeout=200",
    "/",
    "org.freedesktop.DBus.NameHasOwner",
    "string:org.freedesktop.PackageKit",
    NULL
  };

  ExternalProgram pkcheck(argv);

  std::string line;
  for ( line = pkcheck.receiveLine(); !line.empty(); line = pkcheck.receiveLine() )
  {
    DBG << "dbus-send: " << line << endl;
    if ( line.find("boolean") != std::string::npos && line.find("true") != std::string::npos )
      result = true;
  }

  pkcheck.close();
  DBG << result << endl;
  return result;
}

// ----------------------------------------------------------------------------

void packagekit_suggest_quit()
{
  const char* argv[] =
  {
    "dbus-send",
    "--system",
    "--dest=org.freedesktop.PackageKit",
    "--type=method_call",
    "/org/freedesktop/PackageKit",
    "org.freedesktop.PackageKit.SuggestDaemonQuit",
    NULL
  };

  ExternalProgram pkcall( argv );
  pkcall.close();
}
