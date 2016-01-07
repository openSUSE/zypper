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
#include "repos.h"

#include "utils/misc.h"

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

bool is_changeable_media(const Url & url)
{
  MIL << "Checking if this is a changeable medium" << endl;
  bool is_cd = false;
  try
  {
    media::MediaManager mm;
    media::MediaAccessId id = mm.open(url);
    is_cd = mm.isChangeable(id);
    mm.close(id);
  }
  catch (const media::MediaException & e)
  {
    ZYPP_CAUGHT(e);
    WAR << "Could not determine if the URL points to a changeable medium" << endl;
  }

  return is_cd;
}

// ----------------------------------------------------------------------------

ResKind string_to_kind( const std::string & skind )
{
  ResObject::Kind empty;
  std::string lskind = str::toLower( skind );
  if ( lskind == "package" )
    return ResKind::package;
  if ( lskind == "pattern" )
    return ResKind::pattern;
  if ( lskind == "product" )
    return ResKind::product;
  if ( lskind == "patch" )
    return ResKind::patch;
  if ( lskind == "srcpackage" )
    return ResKind::srcpackage;
  if ( lskind == "application" )
    return ResKind::application;
  // not recognized
  return empty;
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

std::string string_patch_status( const PoolItem & pi )
{
  // make sure this will not happen
  if ( pi.isUndetermined() )
    return _("Unknown");

  if ( pi.isRelevant() )
  {
    if ( pi.isSatisfied() )
      return _("Installed"); //! \todo make this "Applied" instead?
    if ( pi.isBroken() )
      return _("Needed");
    // can this ever happen?
    return "";
  }

  return _("Not Needed");
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
      Zypper::instance()->out().error(_("Specified local path does not exist or is not accessible."));
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
    Zypper::instance()->out().error( str::Str() << _("Given URI is invalid") << ": " << urlstr << " (" << excpt_r.asUserString() << ")" );
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
    { "build.opensuse.org",	{ Url("http://download.opensuse.org/repositories/"),	std::string() } }
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
      Zypper & zypper( *Zypper::instance() );

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
    Zypper::instance()->out().error(_("Invalid OBS URI."), _("Correct form is obs://<project>/[platform]"));
    Zypper::instance()->out().info(str::form(_("Example: %s"), "obs://server:http/openSUSE_11.3"));
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

Pathname cache_rpm( const std::string & rpm_uri_str, const std::string & cache_dir )
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
    Pathname cachedrpmpath = cache_dir;
    filesystem::assert_dir(cachedrpmpath);
    bool error =
      filesystem::copy(localrpmpath, cachedrpmpath / localrpmpath.basename());

    mm.release(mid);
    mm.close(mid);

    if ( error )
    {
      Zypper::instance()->out().error(
        _("Problem copying the specified RPM file to the cache directory."),
        _("Perhaps you are running out of disk space."));
      return Pathname();
    }
    return cachedrpmpath / localrpmpath.basename();
  }
  catch (const Exception & e)
  {
    Zypper::instance()->out().error(e,
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
std::string asXML( const Product & p, bool is_installed )
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

    dumpAsXmlOn( *parent, p.endOfLife(), "endoflife" );
    dumpAsXmlOn( *parent, p.registerFlavor(), "registerflavor" );
    {
      const std::string & text( p.description() );
      if ( ! text.empty() )
	*xmlout::Node( *parent, "description" ) << xml::escape( text );
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

DownloadMode get_download_option( Zypper & zypper, bool quiet )
{
  DownloadMode mode;
  DownloadMode zconfig = ZConfig::instance().commit_downloadMode();

  // check for --download-* options
  if (zypper.cOpts().count("download-only"))
    mode = DownloadOnly;
  else if (zypper.cOpts().count("download-in-advance"))
    mode = DownloadInAdvance;
  else if (zypper.cOpts().count("download-in-heaps"))
    mode = DownloadInHeaps;
  else if (zypper.cOpts().count("download-as-needed"))
    mode = DownloadAsNeeded;
  else
    mode = zconfig;

  // check --download <mode>
  // this option overrides the above aliases, if used simultaneously
  std::string download;
  parsed_opts::const_iterator it = zypper.cOpts().find("download");
  if (it != zypper.cOpts().end())
    download = it->second.front();
  if (download == "only")
    mode = DownloadOnly;
  else if (download == "in-advance")
    mode = DownloadInAdvance;
  else if (download == "in-heaps")
    mode = DownloadInHeaps;
  else if (download == "as-needed")
    mode = DownloadAsNeeded;
  else if (!download.empty())
  {
    zypper.out().error(str::form(_("Unknown download mode '%s'."), download.c_str()));
    zypper.out().info(str::form(_("Available download modes: %s"),
          "only, in-advance, in-heaps, as-needed"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    ZYPP_THROW( ExitRequestException("Unknown download mode") );
  }

  // warn about the override, both were specified
  if (!quiet && !download.empty() &&
      (zypper.cOpts().count("download-only") ||
       zypper.cOpts().count("download-in-advance") ||
       zypper.cOpts().count("download-in-heaps") ||
       zypper.cOpts().count("download-as-needed")))
  {
    zypper.out().warning(
      str::form(_("Option '%s' overrides '%s'."), "--download", "--download-*"));
  }

  if (quiet)
    return mode;

  MIL << "Download mode: ";
  if      (mode == DownloadInAdvance) MIL << "in-advance";
  else if (mode == DownloadInHeaps)   MIL << "in-heaps";
  else if (mode == DownloadOnly)      MIL << "only";
  else if (mode == DownloadAsNeeded)  MIL << "as-needed";
  else                                MIL << "UNKNOWN";
  MIL << (mode == zconfig ? " (zconfig value)" : "") << endl;

  return mode;
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
