/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <sstream>
#include <iostream>
#include <unistd.h>          // for getcwd()

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Easy.h"
#include "zypp/base/Regex.h"
#include "zypp/media/MediaManager.h"
#include "zypp/parser/xml/XmlEscape.h"
#include "zypp/misc/CheckAccessDeleted.h"
#include "zypp/ExternalProgram.h"

#include "zypp/PoolItem.h"
#include "zypp/Product.h"
#include "zypp/Pattern.h"

#include "main.h"
#include "Zypper.h"
#include "Table.h"             // for process list in suggest_restart_services

#include "utils/misc.h"


using namespace std;
using namespace zypp;

// ----------------------------------------------------------------------------

bool is_changeable_media(const zypp::Url & url)
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

ResKind string_to_kind (const string & skind)
{
  ResObject::Kind empty;
  string lskind = str::toLower (skind);
  if (lskind == "package")
    return ResKind::package;
  if (lskind == "pattern")
    return ResKind::pattern;
  if (lskind == "product")
    return ResKind::product;
  if (lskind == "patch")
    return ResKind::patch;
  if (lskind == "srcpackage")
    return ResKind::srcpackage;
  // not recognized
  return empty;
}

// ----------------------------------------------------------------------------

ResKindSet kindset_from(const std::list<std::string> & kindstrings)
{
  ResKind kind;
  ResKindSet kinds;
  for_(it, kindstrings.begin(), kindstrings.end())
  {
    kind = string_to_kind(*it);
    if (kind == ResKind::nokind)
      continue;
    kinds.insert(kind);
  }
  return kinds;
}

// ----------------------------------------------------------------------------

string kind_to_string_localized(const zypp::ResKind & kind, unsigned long count)
{
  if (kind == ResKind::package)
    return _PL("package", "packages", count);
  if (kind == ResKind::pattern)
    return _PL("pattern", "patterns", count);
  if (kind == ResKind::product)
    return _PL("product", "product", count);
  if (kind == ResKind::patch)
    return _PL("patch", "patches", count);
  if (kind == ResKind::srcpackage)
    return _PL("srcpackage", "srcpackages", count);
  // default
  return _PL("resolvable", "resolvables", count);
}

// ----------------------------------------------------------------------------

string string_patch_status(const PoolItem & pi)
{
  // make sure this will not happen
  if (pi.isUndetermined())
    return _("Unknown");

  if (pi.isRelevant())
  {
    if (pi.isSatisfied())
      return _("Installed"); //! \todo make this "Applied" instead?
    if (pi.isBroken())
      return _("Needed");
    // can this ever happen?
    return "";
  }

  return _("Not Applicable"); //! \todo make this "Not Needed" after 11.0
}

// ----------------------------------------------------------------------------

bool looks_like_url (const string& s)
{
/*
  static bool schemes_shown = false;
  if (!schemes_shown) {
    DBG << "Registered schemes: " << Url::getRegisteredSchemes () << endl;
    schemes_shown = true;
  }
*/
  string::size_type pos = s.find (':');
  if (pos != string::npos)
  {
    string scheme (s, 0, pos);
    if (Url::isRegisteredScheme(scheme) || scheme == "obs")
      return true;
  }
  return false;
}

// ----------------------------------------------------------------------------

Url make_url (const string & url_s)
{
  Url u;
  string urlstr(zypp::url::encode(url_s, URL_SAFE_CHARS));

  if (!url_s.empty() && !looks_like_url(url_s))
  {
    DBG << "'" << url_s << "' does not look like a URL, trying to treat it like a local path" << endl;

    Pathname path;
    // make an url from absolute path
    if (url_s[0] == '/')
      path = url_s;
    // make absolute path url from relative path
    else
    {
      char buf[PATH_MAX];
      if (::getcwd(buf, PATH_MAX) != NULL)
      {
        DBG <<  "current working directory: " << buf << endl;
        path = string(buf) + "/" + url_s;
      }
    }

    if (PathInfo(path).isExist())
    {
      urlstr = "dir:" + url::encode(path.absolutename().asString(),"/");
      MIL <<  "resulting url: " << urlstr << endl;
    }
    else
    {
      Zypper::instance()->out().error(
        _("Specified local path does not exist or is not accessible."));
      ERR << "specified local path does not exist or is not accessible" << endl;
      return u;
    }
  }

  try {
    u = Url(urlstr);
  }
  catch ( const Exception & excpt_r ) {
    ZYPP_CAUGHT( excpt_r );
    ostringstream s;
    s << _("Given URI is invalid") << ": " << urlstr
      << " (" << excpt_r.asUserString() << ")";
    Zypper::instance()->out().error(s.str());
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

Url make_obs_url (
    const string & obsuri,
    const Url & base_url,
    const string & default_platform)
{
  // zypper's 'obs' URI regex
  static str::regex obs_uri_rx("^obs://(" OBS_PROJECT_NAME_RX ")/?(.*)$");
  str::smatch what;
  if (str::regex_match(obsuri, what, obs_uri_rx))
  {
    vector<string> obsrpath;
    str::split(what[1], back_inserter(obsrpath), ":");
    if (obsrpath.empty())
    {
      Zypper::instance()->out().error(_("Empty OBS project name."));
      return Url();
    }

    ostringstream urlstr; urlstr << "/";
    unsigned i = 0;
    for (; i < obsrpath.size() - 1; ++i)
      urlstr << obsrpath[i] << ":/";
    urlstr << obsrpath[i] << "/";         // no colon at the end

    if (what[2].empty())
      urlstr << default_platform;
    else
      urlstr << what[2];
    urlstr << "/";

    Url url = Url(base_url);
    Pathname newpath(url.getPathName());
    newpath = newpath / Pathname(urlstr.str());
    url.setPathName(newpath.asString());

    return url;
  }
  else
  {
    Zypper::instance()->out().error(_("Invalid OBS URI."), _("Correct form is obs://<project>/[platform]"));
    Zypper::instance()->out().info(str::form(_("Example: %s"), "obs://server:http/openSUSE_11.3"));
  }

  return Url();
}

// ----------------------------------------------------------------------------

bool looks_like_rpm_file(const string & s)
{
  // don't even bother to check strings shorter than 4 chars.
  if (s.size() <= 4)
    return false;

  if (s.rfind(".rpm") == s.size() - 4 || // ends with .rpm
      s.find("./") == 0 ||               // starts with ./ or ../ indicating
      s.find("../") == 0)                //  a local path
    return true;

  return false;
}

// ----------------------------------------------------------------------------

Pathname cache_rpm(const string & rpm_uri_str, const string & cache_dir)
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

    if (error)
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
        _("Problem retrieving the specified RPM file") + string(":"),
        _("Please check whether the file is accessible."));
  }

  return Pathname();
}

string xml_encode(const string & text)
{
  return zypp::xml::escape(text);
}

std::string & indent(std::string & text, int columns)
{
  string indent(columns, ' '); indent.insert(0, 1, '\n');
  DBG << "to: '" << indent << "'" << endl;
  text = str::gsub(text, "\n", indent);
  text.insert(0, string(columns, ' '));
  return text;
}

/**
 * \todo this is an ugly quick-hack code, let's do something reusable and maintainable in libzypp later
 */
// ----------------------------------------------------------------------------
string asXML(const Product & p, bool is_installed)
{
  ostringstream str;
  str
    << "<product"
       " name=\"" << xml_encode(p.name()) << "\""
       " version=\"" << p.edition().version() << "\""
       " release=\"" << p.edition().release() << "\""
       " epoch=\"" << p.edition().epoch() << "\""
       " arch=\"" << p.arch() << "\""
       " productline=\"" << p.productLine() << "\""
       " registerrelease=\"" << xml_encode(p.registerRelease()) << "\""
       " vendor=\"" << xml_encode(p.vendor()) << "\""
       " summary=\"" << xml_encode(p.summary()) << "\""
       " shortname=\"" << xml_encode(p.shortName()) << "\""
       " flavor=\"" << xml_encode(p.flavor()) << "\""
       " isbase=\"" << (p.isTargetDistribution() ? 1 : 0) << "\""
       " repo=\"" << xml_encode(p.repoInfo().alias()) << "\""
       " installed=\"" << (is_installed ? 1 : 0) << "\"";
  if (p.description().empty())
    str << "/>";
  else
    str
      << ">" << endl << "<description>" << p.description() << "</description>"
      << endl << "</product>";
  return str.str();
}

string asXML(const Pattern & p, bool is_installed)
{
  ostringstream str;
  str
    << "<pattern"
       " name=\"" << xml_encode(p.name()) << "\""
       " version=\"" << p.edition().version() << "\""
       " release=\"" << p.edition().release() << "\""
       " epoch=\"" << p.edition().epoch() << "\""
       " arch=\"" << p.arch() << "\""
       " vendor=\"" << xml_encode(p.vendor()) << "\""
       " summary=\"" << xml_encode(p.summary()) << "\""
       " repo=\"" << xml_encode(p.repoInfo().alias()) << "\""
       " installed=\"" << (is_installed ? 1 : 0) << "\""
       " uservisible=\"" << (p.userVisible() ? 1 : 0) << "\"";
  if (p.description().empty())
    str << "/>";
  else
    str
      << ">" << endl << "<description>" << p.description() << "</description>"
      << endl << "</pattern>";
  return str.str();
}

// ----------------------------------------------------------------------------

void list_processes_using_deleted_files(Zypper & zypper)
{
  zypper.out().info(
      _("Checking for running processes using deleted libraries..."), Out::HIGH);
  zypp::CheckAccessDeleted checker(false); // wait for explicit call to check()
  try
  {
    checker.check();
  }
  catch(const zypp::Exception & e)
  {
    zypper.out().error(e, _("Check failed:"));
    return;
  }

  Table t;
  t.allowAbbrev(6);
  TableHeader th;
  // process ID
  th << _("PID");
  // parent process ID
  th << _("PPID");
  // process user ID
  th << _("UID");
  // process login name
  th << _("Login");
  // process command name
  th << _("Command");
  // "/etc/init.d/ script that might be used to restart the command (guessed)
  th << _("Service");
  // "list of deleted files or libraries accessed"
  th << _("Files");
  t << th;

  for_( it, checker.begin(), checker.end() )
  {
    TableRow tr;
    vector<string>::const_iterator fit = it->files.begin();
    tr << it->pid << it->ppid << it->puid << it->login << it->command
      << it->service() << (fit != it->files.end() ? *fit : "");
    t << tr;
    for (++fit; fit != it->files.end(); ++fit)
    {
      TableRow tr1;
      tr1 << "" << "" << "" << "" << "" << "" << *fit;
      t << tr1;
    }
  }

  if (t.empty())
  {
    zypper.out().info(_("No processes using deleted files found."));
  }
  else
  {
    zypper.out().info(_("The following running processes use deleted files:"));
    cout << endl;
    cout << t << endl;
    zypper.out().info(_("You may wish to restart these processes."));
    zypper.out().info(str::form(
        _("See '%s' for information about the meaning of values"
          " in the above table."),
        "man zypper"));
  }
}

// ----------------------------------------------------------------------------

DownloadMode get_download_option(Zypper & zypper, bool quiet)
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
  string download;
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
    throw ExitRequestException("Unknown download mode");
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

  string line;
  for (line = pkcheck.receiveLine(); !line.empty(); line = pkcheck.receiveLine())
  {
    DBG << "dbus-send: " << line << endl;
    if (line.find("boolean") != string::npos && line.find("true") != string::npos)
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

  ExternalProgram pkcall(argv);
  pkcall.close();
}
