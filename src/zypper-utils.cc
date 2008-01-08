#include <fstream>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "zypp/base/Logger.h"
#include "zypp/media/MediaManager.h"
#include "zypp/Pathname.h"

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

  /* Get a line from the user. */
  line_read = readline ("zypper> ");

  /* If the line has any text in it,
     save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);

  if (line_read)
    return line_read;
  else
    return "\004";
}

/// tell the user to report a bug, and how
// (multiline, with endls)
ostream& report_a_bug (ostream& stm)
{
  return stm << _("Please file a bug report about this.") << endl
    // TranslatorExplanation remember not to translate the URL
    // unless you translate the actual page :)
             << _("See http://en.opensuse.org/Zypper#Troubleshooting for instructions.") << endl;
}

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
  if (kind == ResTraits<Selection>::kind)
    return _PL("selection", "selections", count);
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
  if (kind == ResTraits<Language>::kind)
    return _PL("language", "languages", count);
  if (kind == ResTraits<Atom>::kind)
    return _PL("atom", "atoms", count);
  if (kind == ResTraits<SystemResObject>::kind)
    return _PL("system", "systems", count);
  if (kind == ResTraits<SrcPackage>::kind)
    return _PL("srcpackage", "srcpackages", count);
  // default
  return _PL("resolvable", "resolvables", count);
}

// ----------------------------------------------------------------------------

static
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
      urlstr = "dir:" + path.absolutename().asString();
      DBG <<  "resulting url: " << urlstr << endl;
    }
    else
      DBG << "specified local path does not exist or is not accessible" << endl;
  }

  try {
    u = Url(urlstr);
  }
  catch ( const Exception & excpt_r ) {
    ZYPP_CAUGHT( excpt_r );
    cerr << _("Given URL is invalid.") << endl;
    cerr << excpt_r.asUserString() << endl;
  }
  return u;
}
