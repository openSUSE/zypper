/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <fstream>
#include <sstream>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/media/MediaManager.h"
#include "zypp/parser/xml/XmlEscape.h"

#include "zypp/PoolItem.h"
#include "zypp/Product.h"
#include "zypp/Pattern.h"

#include "main.h"
#include "Zypper.h"

#include "utils/misc.h"


using namespace std;
using namespace zypp;

// Read a string. "\004" (^D) on EOF.
string readline_getline()
{
  // A static variable for holding the line.
  static char *line_read = NULL;

  /* If the buffer has already been allocated,
     return the memory to the free pool. */
  if (line_read) {
    free (line_read);
    line_read = NULL;
  }

  //::rl_catch_signals = 0;
  /* Get a line from the user. */
  line_read = ::readline ("zypper> ");

  /* If the line has any text in it,
     save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);

  if (line_read)
    return line_read;
  else
    return "\004";
}

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

bool equalNVRA(const Resolvable & lhs, const Resolvable & rhs)
{
  if (lhs.name() != rhs.name())
    return false;
  if (lhs.kind() != rhs.kind())
    return false;
  if (lhs.edition() != rhs.edition())
    return false;
  if (lhs.arch() != rhs.arch())
    return false;
  return true;
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
  if (pos != string::npos) {
    string scheme (s, 0, pos);
    if (Url::isRegisteredScheme (scheme)) {
      return true;
    }
  }
  return false;
}

// ----------------------------------------------------------------------------

Url make_url (const string & url_s)
{
  Url u;
  string urlstr(url_s);

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
      if (getcwd(buf, PATH_MAX) != NULL)
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
    mm.attachDesiredMedia(mid);

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

static string preparse_cap_str(const string & capstr)
{
  // expect versioned caps as NAME[OP<EDITION>]
  // transform to NAME[ OP <EDITION>] (add spaces)
  string new_capstr = capstr;
  DBG << "capstr: " << capstr << endl;
  string::size_type op_pos = capstr.find_first_of("<>=");
  if (op_pos != string::npos)
  {
    new_capstr.insert(op_pos, " ");
    DBG << "new capstr: " << new_capstr << endl;
    op_pos = new_capstr.find_first_not_of("<>=", op_pos + 1);
    if (op_pos != string::npos && new_capstr.size() > op_pos)
    {
      new_capstr.insert(op_pos, " ");
      DBG << "new capstr: " << new_capstr << endl;
    }
  }

  return new_capstr;
}

Capability safe_parse_cap (Zypper & zypper,
                           const string & capstr,
                           const ResKind & kind)
{
  try
  {
    if (kind == ResKind::nokind)
      return Capability(preparse_cap_str(capstr));
    else
      return Capability(preparse_cap_str(capstr), kind);
  }
  catch (const Exception& e)
  {
    //! \todo check this handling (should we fail or set a special exit code?)
    ZYPP_CAUGHT(e);
    zypper.out().error(str::form(_("Cannot parse capability '%s'."), capstr.c_str()));
  }
  return Capability();
}

string asXML(const Product & p, bool is_installed)
{
  ostringstream str;
  str
    << "<product"
       " name=\"" << xml_encode(p.name()) << "\""
       " version=\"" << p.edition() << "\""
       " arch=\"" << p.arch() << "\""
       " vendor=\"" << xml_encode(p.vendor()) << "\""
       " summary=\"" << xml_encode(p.summary()) << "\""
       " shortname=\"" << xml_encode(p.shortName()) << "\""
       " flavor=\"" << xml_encode(p.flavor()) << "\""
       " type=\"" << xml_encode(p.type()) << "\""
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
  return "";
}
