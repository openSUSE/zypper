#include "zypper-search.h"

#include "zmart.h"
#include "zmart-sources.h"
#include "zmart-misc.h"

using namespace zypp;
using namespace std;

// TODO get rid of these globals
extern ZYpp::Ptr God;
extern RuntimeData gData;
extern Settings gSettings;

ZyppSearch::ZyppSearch (ZyppSearchOptions const &options) :
    _options(options), _qstrings(empty_vector) {
  init();
}

ZyppSearch::ZyppSearch (ZyppSearchOptions const &options, vector<string> const &qstrings) :
    _options(options), _qstrings(qstrings) {
  init();
}

bool ZyppSearch::init () const {
  cond_init_system_sources();
  cond_init_target();
  
  // load additional sources
  for ( std::list<Url>::const_iterator it = gSettings.additional_sources.begin();
      it != gSettings.additional_sources.end(); ++it ) {
    include_source_by_url( *it );
  }
  
  // TODO no sources warning
  if ( gData.sources.empty() ) {
    cerr << "Warning! No sources. Operating only over the installed resolvables."
      " You will not be able to install stuff" << endl;
  }

  if (!_options.uninstalledOnly()) {
    cerr_v << "loading target" << endl;
    load_target();
  }

  if (!_options.installedOnly()) {
    cerr_v << "loading sources" << endl;
    load_sources();
  }

  return true;
}

Table ZyppSearch::doSearch() {
  ResPool pool = getZYpp()->pool();

  this->setupRegexp();

  Table otable;
  otable.style(Ascii);

  TableHeader header;
  header << "S" << "Catalog" << "Bundle" << "Name" << "Version" << "Arch";
  otable << header; 

  bool match;
  for (zypp::ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it) {
    match = regex_match(it->resolvable()->name(),this->_reg); // TODO search in descriptions and summaries
    if (match) {
      TableRow row;
      row << (it->status().isInstalled() ? "i" : "u")
          << it->resolvable()->source().alias()
          << "" // TODO what about Bundle?
          << it->resolvable()->name()
          << it->resolvable()->edition().asString()
          << it->resolvable()->arch().asString();
      otable << row;
    }
  }

  return otable;
}

void ZyppSearch::setupRegexp() {
  string regstr;

  if (_qstrings.size() == 0) regstr = ".*";
  else if (_qstrings.size() == 1) regstr = ".*" + _qstrings[0] + ".*";
  else {
    vector<string>::const_iterator it = _qstrings.begin();
    
    if (_options.matchAll())
      regstr = "(?=.*" + *it + ")";
    else
      regstr = ".*(" + *it;

    ++it;

    for (; it != _qstrings.end(); ++it) {
      if (_options.matchAll())
        regstr += "(?=.*" + *it + ")";
      else
        regstr += "|" + *it;
    }

    if (_options.matchAll())
      regstr += ".*";
    else
      regstr += ").*";
  }

  // tmp debugging output
  cout << "using regex: " << regstr << endl;

  try {
     _reg.assign(regstr, boost::regex::perl|boost::regex_constants::icase);
  }
  catch (regex_error& e)
  {
    cerr << regstr << " is not a valid regular expression: \""
      << e.what() << "\"" << endl;
    exit(1);
  }
}
