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

void ZyppSearchOptions::resolveConflicts() {
  if (matchExact()) {
    // --match-all does not make sense here
    setMatchAny();
    // the same goes for search in descriptions
    setSearchDescriptions(false);
  }

  // ??? should we notify user about conflict resolutions?
}

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

  if (_options.installedFilter() != ZyppSearchOptions::UNINSTALLED_ONLY) {
    cerr_v << "loading target" << endl;
    load_target();
  }

  if (_options.installedFilter() != ZyppSearchOptions::INSTALLED_ONLY) {
    cerr_v << "loading sources" << endl;
    load_sources();
  }

  return true;
}

void ZyppSearch::doSearch(const boost::function<void(const PoolItem &)> & f) {
  ResPool pool = getZYpp()->pool();

  // search for specific resolvable type only
  if (_options.kind() != Resolvable::Kind()) {
    cerr_vv << "Search by type" << endl;
    setupRegexp();
    for (ResPool::byKind_iterator it = pool.byKindBegin(_options.kind());
        it != pool.byKindEnd(_options.kind()); ++it) {
      if (match(*it)) f(*it);
    }
  }
  // search for exact package using byName_iterator
  // usable only if there is only one query string and if this string
  // doesn't contain wildcards
  else if (_options.matchExact() && _qstrings.size() == 1 &&
      _qstrings[0].find('*') == string::npos &&
      _qstrings[0].find('?') == string::npos) {
    cerr_vv << "Exact name match" << endl;
    for (ResPool::byName_iterator it = pool.byNameBegin(_qstrings[0]);
        it != pool.byNameEnd(_qstrings[0]); ++it) {
      f(*it); //table << createRow(*it);
    }
  }
  // search among all resolvables
  else {
    cerr_vv << "Search among all resolvables" << endl;
    setupRegexp();
    for (ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it) {
      if (match(*it)) f(*it); //table << createRow(*it);
    }
  }
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
  else if (_qstrings.size() == 1) {
    if (_options.matchExact())
      regstr = wildcards2regex(_qstrings[0]);
    else
      regstr = ".*" + WB + wildcards2regex(_qstrings[0]) + WB + ".*";
  }
  else {
    vector<string>::const_iterator it = _qstrings.begin();

    if (_options.matchAll())
      regstr = "(?=.*" + WB + wildcards2regex(*it) + WB + ")";
    else {
      if (!_options.matchExact()) regstr = ".*";
      regstr += WB + "(" + wildcards2regex(*it);
    }

    ++it;

    for (; it != _qstrings.end(); ++it) {
      if (_options.matchAll())
        regstr += "(?=.*" + WB + wildcards2regex(*it) + WB + ")";
      else
        regstr += "|" + wildcards2regex(*it);
    }

    if (_options.matchAll())
      regstr += ".*";
    else {
      regstr += ")" + WB;
      if (!_options.matchExact()) regstr += ".*";
    }
  }

  cerr_vv << "using regex: " << regstr << endl;

  // regex flags
  unsigned int flags = boost::regex::normal;
  if (!_options.caseSensitive())
    flags |= boost::regex_constants::icase;

  // create regex object
  try {
     _reg.assign(regstr, flags);
  }
  catch (regex_error & e) {
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

/**
 * Decides whether the pool_item (resolvable) matches the search criteria
 * encoded in regular expression.
 */
bool ZyppSearch::match(const PoolItem & pool_item) {
  return
    // match resolvable name
    regex_match(pool_item.resolvable()->name(), _reg)
      ||
    // if required, match also summary and description of the resolvable
    (_options.searchDescriptions() ?
      regex_match(pool_item.resolvable()->summary(), _reg) ||
        regex_match(pool_item.resolvable()->description(), _reg)
          :
      false);
}

// Local Variables:
// c-basic-offset: 2
// End:
