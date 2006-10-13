#include "zypper-search.h"

#include "zmart.h"
#include "zmart-sources.h"
#include "zmart-misc.h"

using namespace zypp;
using namespace std;
using namespace boost;

// TODO get rid of these globals
extern RuntimeData gData;
extern Settings gSettings;

ZyppSearch::ZyppSearch (const ZyppSearchOptions & options, const vector<string> & qstrings) :
    _options(options), _qstrings(qstrings) {
  init();
}

// TODO clean this up
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

// TODO comment
Table ZyppSearch::doSearch() {
  ResPool pool = getZYpp()->pool();

  setupRegexp();

  Table otable;
  otable.style(Ascii);

  TableHeader header;
  header << "S" << "Catalog" << "Bundle" << "Name" << "Version" << "Arch";
  otable << header; 

  if (_options.kind() != Resolvable::Kind()) {
    for (ResPool::byKind_iterator it = pool.byKindBegin(_options.kind());
        it != pool.byKindEnd(_options.kind()); ++it) {
      if (match(*it)) otable << createRow(*it);
    }
  }
  else {
    for (ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it) {
      if (match(*it)) otable << createRow(*it);
    }
  }

  return otable;
}

//! macro for word boundary tags for regexes
#define WB (_options.matchWords() ? string("\\b") : string())

/**
 * Creates a regex for searching in resolvable names.
 * 
 * The regex is created according to given search strings and search options.
 * 
 * Examples:
 *   - no search string: .*
 *   - one search string: .*searchstring.*
 *     with --match-words: .*\bsearchstring\b.*
 *   - more search strings:
 *     --match-all (default):
 *       (?=.*searchstring1)(?=.*searchstring2).*
 *       with --match-words: (?=.*\bsearchstring1\b)(?=.*\bsearchstring2\b).*
 *     --match-any:
 *       .*(searchstring1|searchstring2).*
 *       with --match-words: .*\b(searchstring1|searchstring2)\b.*
 */
void ZyppSearch::setupRegexp() {
  string regstr;

  if (_qstrings.size() == 0) regstr = ".*";
  else if (_qstrings.size() == 1) regstr = ".*" + WB + wildcards2regex(_qstrings[0]) + WB + ".*";
  else {
    vector<string>::const_iterator it = _qstrings.begin();

    if (_options.matchAll())
      regstr = "(?=.*" + WB + wildcards2regex(*it) + WB + ")";
    else
      regstr = ".*" + WB + "(" + wildcards2regex(*it);

    ++it;

    for (; it != _qstrings.end(); ++it) {
      if (_options.matchAll())
        regstr += "(?=.*" + WB + wildcards2regex(*it) + WB + ")";
      else
        regstr += "|" + wildcards2regex(*it);
    }

    if (_options.matchAll())
      regstr += ".*";
    else
      regstr += ")" + WB + ".*";
  }

  cerr_vv << "using regex: " << regstr << endl;

  try {
     _reg.assign(regstr, boost::regex::perl|boost::regex_constants::icase);
  }
  catch (regex_error & e)
  {
    cerr << "ZyppSearch::setupRegexp(): " << regstr
      << " is not a valid regular expression: \""
      << e.what() << "\"" << endl;
    cerr << "This is a bug, please file a bug report against zypper." << endl;
    exit(1);
  }
}

/**
 * Converts '*' and '?' wildcards within str into their regex equivalents.
 */
string ZyppSearch::wildcards2regex(const string & str) const {
  string regexed;

  regex all("\\*"); // regex to search for '*'
  regex one("\\?"); // regex to search for '?'
  string r_all(".*"); // regex equivalent of '*'
  string r_one(".");  // regex equivalent of '?'

  // replace all "*" in input with ".*"
  regexed = regex_replace(str, all, r_all);
  cerr_vv << "wildcards2regex: " << str << " -> " << regexed;

  // replace all "?" in input with "."
  regexed = regex_replace(regexed, one, r_one);
  cerr_vv << " -> " << regexed << endl;

  return regexed;
}

bool ZyppSearch::match(const PoolItem & pool_item) {
  // TODO search in descriptions and summaries
  return regex_match(pool_item.resolvable()->name(), _reg); 
}

TableRow ZyppSearch::createRow(const PoolItem & pool_item) {
  TableRow row;
  row << (pool_item.status().isInstalled() ? "i" : "")
      << pool_item.resolvable()->source().alias()
      << "" // TODO what about Bundle?
      << pool_item.resolvable()->name()
      << pool_item.resolvable()->edition().asString()
      << pool_item.resolvable()->arch().asString();
  return row;
}
