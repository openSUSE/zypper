#include <fstream>

#include <zypp/media/MediaManager.h>

#include "zypper.h"

using namespace zypp::detail;

using namespace std;
using namespace zypp;

string read_line_from_file( const Pathname &file )
{
  string buffer;
  string token;
  std::ifstream is(file.asString().c_str());
  if ( is.good() )
  {
    while(is && !is.eof())
    {
      getline(is, buffer);
      token += buffer;
    }
    is.close();
  }
  return token;
 }

// ----------------------------------------------------------------------------

void write_line_to_file( const Pathname &file, const std::string &line )
{
  std::ofstream os(file.asString().c_str());
  if ( os.good() )
  {
    os << line << endl;;
  }
  os.close();
}

// ----------------------------------------------------------------------------

/// tell to report a bug, and how
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
  media::MediaManager mm; media::MediaAccessId id = mm.open(url);
  bool is_cd = mm.isChangeable(id);
  mm.close(id);
  return is_cd;
}
