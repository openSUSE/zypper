#include <fstream>
#include <sstream>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "zypp/Pathname.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/media/MediaManager.h"
#include "zypp/parser/xml_escape_parser.hpp"
#include "zypp/Capability.h"

#include "zypper.h"
#include "zypper-main.h"
#include "zypper-utils.h"


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

/// tell the user to report a bug, and how
// (multiline, with endls)
void report_a_bug (Out & out)
{
  ostringstream s;
  s <<_("Please file a bug report about this.") << endl
      // TranslatorExplanation remember not to translate the URL
      // unless you translate the actual page :)
    << _("See http://en.opensuse.org/Zypper#Troubleshooting for instructions.");
  out.error(s.str());
}

// ----------------------------------------------------------------------------

void report_too_many_arguments(const string & specific_help)
{
  report_too_many_arguments(Zypper::instance()->out(), specific_help);
}

void report_too_many_arguments(Out & out, const string & specific_help)
{
  //! \todo make this more explanatory, e.g. "Ingoring arg1 arg2. This command does not take arguments. See %s for more information."
  ostringstream s;
  s << _("Usage") << ':' << endl << specific_help;
  out.error(_("Too many arguments."), s.str());
}

// ----------------------------------------------------------------------------

void report_required_arg_missing(Out & out, const string & cmd_help)
{
  out.error(_("Required argument missing."));
  ostringstream s;
  s << _("Usage") << ':' << endl;
  s << cmd_help;
  out.info(s.str());
}

// ----------------------------------------------------------------------------

void report_dummy_option(Out & out, const string & longoption_str)
{
  out.warning(str::form(
    _("The '--%s' option has currently no effect."), longoption_str.c_str()));
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

string kind_to_string_localized(const Resolvable::Kind & kind, unsigned long count)
{
  if (kind == ResTraits<Package>::kind)
    return _PL("package", "packages", count);
  if (kind == ResTraits<Pattern>::kind)
    return _PL("pattern", "patterns", count);
  if (kind == ResTraits<Product>::kind)
    return _PL("product", "product", count);
  if (kind == ResTraits<Patch>::kind)
    return _PL("patch", "patches", count);
  if (kind == ResTraits<Script>::kind)
    return _PL("script", "scripts", count);
  if (kind == ResTraits<Message>::kind)
    return _PL("message", "messages", count);
  if (kind == ResTraits<Atom>::kind)
    return _PL("atom", "atoms", count);
//   if (kind == ResTraits<SystemResObject>::kind)
//     return _PL("system", "systems", count);
  if (kind == ResTraits<SrcPackage>::kind)
    return _PL("srcpackage", "srcpackages", count);
  // default
  return _PL("resolvable", "resolvables", count);
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
        _("Problem downloading the specified RPM file") + string(":"),
        _("Please check whether the file is accessible."));
  }

  return Pathname();
}

string xml_encode(const string & text)
{
  iobind::parser::xml_escape_parser parser;
  return parser.escape(text);
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
