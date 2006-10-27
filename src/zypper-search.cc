#include "zypper-search.h"

#include "zmart.h"
#include "zmart-sources.h"
#include "zmart-misc.h"

#include <zypp/base/Algorithm.h>

using namespace std;
using namespace boost;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::resfilter;

// TODO get rid of these globals
extern RuntimeData gData;

void ZyppSearchOptions::resolveConflicts() {
  if (matchExact()) {
    // --match-all does not make sense here
    setMatchAny();
    // the same goes for search in descriptions
    setSearchDescriptions(false);
  }

  // ??? should we notify user about conflict resolutions?
}

/**
 * Initializes installation sources, creates search regex, caches installed
 * packages from RPM database, and populates ResPool with items from
 * installation sources.
 */ 
ZyppSearch::ZyppSearch (
    ZYpp::Ptr & zypp,
    const ZyppSearchOptions & options,
    const vector<string> qstrings
    ) :
    _zypp(zypp), _options(options), _qstrings(qstrings) {

  cond_init_target();         // calls ZYpp::initializeTarget("/");
  cond_init_system_sources(); // calls manager->restore("/");

  // no sources warning
  if (gData.sources.empty()) {
    cerr << "No sources. Zypper currently searches within installation"
        "sources only." << endl;
    exit(2); // TODO #define zypper error codes?
  }

  setupRegexp();
  cacheInstalled();
  load_sources(); // populates ResPool with resolvables from inst. sources
}

/**
 * Invokes zypp::invokeOnEach() on a subset of pool items restricted by
 * some search criteria (--type,--match-exact).
 */
template <class _Filter, class _Function>
int ZyppSearch::invokeOnEachSearched(_Filter filter_r, _Function fnc_r) {
  ResPool pool = _zypp->pool();

  // search for specific resolvable type only
  if (_options.kind() != Resolvable::Kind()) {
    cerr_vv << "invokeOnEachSearched(): search by type" << endl;

    return invokeOnEach(
        pool.byKindBegin(_options.kind()), pool.byKindEnd(_options.kind()),
        filter_r, fnc_r);
  }
  // search for exact package using byName_iterator
  // usable only if there is only one query string and if this string
  // doesn't contain wildcards
  else if (_options.matchExact() && _qstrings.size() == 1 &&
      _qstrings[0].find('*') == string::npos &&
      _qstrings[0].find('?') == string::npos) {
    cerr_vv << "invokeOnEachSearched(): exact name match" << endl;

    return invokeOnEach(
        pool.byNameBegin(_qstrings[0]), pool.byNameEnd(_qstrings[0]),
        filter_r, fnc_r);
  }

  // search among all resolvables
  else {
    cerr_vv << "invokeOnEachSearched(): search among all resolvables" << endl;

    return invokeOnEach(pool.begin(), pool.end(), filter_r, fnc_r);
  }
}

/**
 * Cache installed packages matching given search criteria into a hash_map.
 * Assumption made: names of currently installed resolvables + kind
 * (+version???) are unique.
 */
void ZyppSearch::cacheInstalled() {
  // don't include kind string in hash map key if search is to be restricted
  // to particular kind (to improve performance a little bit)
  if (_options.kind() != Resolvable::Kind())
    _icache.setIncludeKindInKey(false);

  cout_v << "Pre-caching installed resolvables matching given search criteria... " << endl;

  ResStore tgt_resolvables(_zypp->target()->resolvables());

  _zypp->addResolvables(tgt_resolvables, true /*installed*/);

  invokeOnEachSearched(Match(_reg,_options.searchDescriptions()),
    functorRef<bool,const zypp::PoolItem &>(_icache));

  _zypp->removeResolvables(tgt_resolvables);

  cout_v << _icache.size() << " out of (" <<  tgt_resolvables.size() << ")"  
    "cached." << endl;
}

/**
 * Invokes functor f on each pool item matching search criteria. 
 */
void ZyppSearch::doSearch(const boost::function<bool(const PoolItem &)> & f) {
  boost::function<bool (const PoolItem &)> filter;

  switch (_options.installedFilter()) {
    case ZyppSearchOptions::INSTALLED_ONLY:
      filter = chain(ByInstalledCache(_icache),Match(_reg,_options.searchDescriptions()));
      break;
    case ZyppSearchOptions::UNINSTALLED_ONLY:
      filter = chain(not_c(ByInstalledCache(_icache)),Match(_reg,_options.searchDescriptions()));
      break;
    case ZyppSearchOptions::ALL:
    default:
      filter = Match(_reg,_options.searchDescriptions());
  }

  invokeOnEachSearched(filter, f);
}

//! macro for word boundary tags for regexes
#define WB (_options.matchWords() ? string("\\b") : string())

// TODO make a regex builder.
/**
 * Creates a regex for searching in resolvable names and descriptions.
 *
 * The regex is created according to given search strings and search options.
 * 
 * <pre>
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
 * </pre>
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

  cout_vv << "using regex: " << regstr << endl;

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

// Local Variables:
// c-basic-offset: 2
// End:
