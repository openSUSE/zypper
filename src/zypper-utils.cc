#include "zypp/base/Logger.h"
#include "zypp/media/MediaManager.h"

#include "zypper-main.h"
#include "zypper-utils.h"

using namespace std;
using namespace zypp;

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

string kind_to_string_localized(const KindOf<Resolvable> & kind, unsigned long count)
{
  if (kind == ResTraits<Package>::kind.asString())
    return _PL("package", "packages", count);
  if (kind == ResTraits<Selection>::kind.asString())
    return _PL("selection", "selections", count);
  if (kind == ResTraits<Pattern>::kind.asString())
    return _PL("pattern", "patterns", count);
  if (kind == ResTraits<Product>::kind.asString())
    return _PL("product", "product", count);
  if (kind == ResTraits<Patch>::kind.asString())
    return _PL("patch", "patches", count);
  if (kind == ResTraits<Script>::kind.asString())
    return _PL("script", "scripts", count);
  if (kind == ResTraits<Message>::kind.asString())
    return _PL("message", "messages", count);
  if (kind == ResTraits<Language>::kind.asString())
    return _PL("language", "languages", count);
  if (kind == ResTraits<Atom>::kind.asString())
    return _PL("atom", "atoms", count);
  if (kind == ResTraits<SystemResObject>::kind.asString())
    return _PL("system", "systems", count);
  if (kind == ResTraits<SrcPackage>::kind.asString())
    return _PL("srcpackage", "srcpackages", count);
  // default
  return _PL("resolvable", "resolvables", count);
}

// ----------------------------------------------------------------------------

Url make_url (const string & url_s) {
  Url u;

  try {
    u = Url( (url_s[0] == '/') ? string("dir:") + url_s : url_s );
  }
  catch ( const Exception & excpt_r ) {
    ZYPP_CAUGHT( excpt_r );
    cerr << _("Given URL is invalid.") << endl;
    cerr << excpt_r.asUserString() << endl;
  }
  return u;
}
