#include "zypper-search.h"

#include "zypper.h"
#include "zypper-main.h"
#include "zypper-repos.h"
#include "zypper-misc.h"

#include "zypp/base/Algorithm.h"
#include "zypp/RepoManager.h"
#include "zypp/RepoInfo.h"

using namespace std;
using namespace boost;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::resfilter;

extern RuntimeData gData;

ZyppSearchOptions::ZyppSearchOptions()
  : _ifilter(ALL)
  , _match_all(true)
  , _match_words(false)
  , _match_exact(false)
  , _search_descriptions(false)
  , _case_sensitive(false)
{
  // _kinds stays empty

  // check for disabled repos and limit list of repos accordingly.
  //   "-r/--repo" will override this
  try {
    RepoManager manager(Zypper::instance()->globalOpts().rm_options);
    std::list<zypp::RepoInfo> known_repos = manager.knownRepositories();
    std::list<zypp::RepoInfo>::const_iterator it_r;
    for (it_r = known_repos.begin(); it_r != known_repos.end(); ++it_r)
    {
      if (!it_r->enabled()) {
	break;
      }
    }
    if (it_r != known_repos.end())	// loop ended prematurely -> one of the repos is disabled
    {
      clearRepos();
      for (it_r = known_repos.begin(); it_r != known_repos.end(); ++it_r)
      {
	if (it_r->enabled()) {
	  addRepo( it_r->alias() );			// explicitly list enabled repos
        }
      }
    }
  }
  catch ( const Exception & excpt_r )
  {
    ZYPP_CAUGHT( excpt_r );
    cerr << "Checking for disabled repositories failed" << endl;
  }
}

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
 * Initializes repositories, creates search regex, caches installed
 * packages from RPM database, and populates ResPool with items from
 * repositories.
 */
ZyppSearch::ZyppSearch (
    ZYpp::Ptr & zypp,
    const ZyppSearchOptions & options,
    const vector<string> qstrings
    ) :
    _zypp(zypp), _options(options), _qstrings(qstrings),
    _query( Zypper::instance()->globalOpts().rm_options.repoCachePath ) {

#if 0	// we don't search the pool but iterate on the cache directly, hence no repos needed
  // no repos warning
  if (gData.repos.empty()) {
    cerr << _("No repositories configured. Please add at least one"
              " repository using 'zypper addrepo' command before using the search.")
         << endl;
    exit(ZYPPER_EXIT_NO_REPOS); // TODO #define zypper error codes?
  }
#endif

  setupRegexp();
  cacheInstalled();
#if 0	// we iterate directly on the cache, no ResPool populate needed
  load_repo_resolvables(); // populates ResPool with resolvables from repos
#endif
  // cache identification strings of source resolvables (used to check for
  // duplicates of target resolvables in repos - DuplicateFilter)
  invokeOnEachSearched(not_c(ByInstalled()), functorRef<bool,const zypp::PoolItem &>(_idcache), functorRef<bool, const data::RecordId &, data::ResObject_Ptr>(_idcache));
}

/**
 * Invokes zypp::invokeOnEach() on a subset of pool items restricted by
 * some search criteria (--type,--match-exact).
 */
template <class _Filter, class _PoolCallback, class _CacheCallback>
int
ZyppSearch::invokeOnEachSearched(_Filter filter_r, _PoolCallback pool_cb, _CacheCallback cache_cb)
{
  // pool only contains _installed_ resolvables
  ResPool pool = _zypp->pool();

  // search for specific resolvable type only
  if (!_options.kinds().empty())
  {
    cerr_vv << "invokeOnEachSearched(): search by type" << endl;

    if (_options.installedFilter() != ZyppSearchOptions::UNINSTALLED_ONLY)
    {
      // search pool on ALL or INSTALLED
      std::vector<zypp::Resolvable::Kind>::const_iterator it;
      for (it = _options.kinds().begin(); it != _options.kinds().end(); ++it) {
        invokeOnEach( pool.byKindBegin( *it ), pool.byKindEnd( *it ), filter_r, pool_cb );
      }
    }

    if (_options.installedFilter() != ZyppSearchOptions::INSTALLED_ONLY)
    {
      try
      {
        // search cache on ALL or UNINSTALLED by TYPE

        _query.iterateResolvablesByKindsAndStringsAndRepos( _options.kinds(), _qstrings, (_options.matchExact() ? cache::MATCH_EXACT : cache::MATCH_SUBSTRING)|cache::MATCH_NAME, _options.repos(), cache_cb );
      }
      catch ( const Exception & excpt_r )
      {
        ZYPP_CAUGHT( excpt_r );
        cerr << "cache::ResolvableQuery failed: " << excpt_r.asUserString() << endl;
      }
    }
    return 0;
  }

  // search for exact package using byName_iterator
  // usable only if there is only one query string and if this string
  // doesn't contain wildcards
  else if (_options.matchExact() && _qstrings.size() == 1 &&
      _qstrings[0].find('*') == string::npos &&
      _qstrings[0].find('?') == string::npos)
  {
    cerr_vv << "invokeOnEachSearched(): exact name match" << endl;

    std::vector<zypp::Resolvable::Kind> kinds = _options.kinds();

    // default to packages
    if (kinds.empty()) {
      kinds.push_back( ResTraits<Package>::kind );
    }

    if (_options.installedFilter() != ZyppSearchOptions::UNINSTALLED_ONLY)
    {
      // search pool on ALL or INSTALLED
      invokeOnEach(
        pool.byNameBegin(_qstrings[0]), pool.byNameEnd(_qstrings[0]),
        filter_r, pool_cb);
    }

    if (_options.installedFilter() != ZyppSearchOptions::INSTALLED_ONLY)
    {
      try
      {
        // search cache on ALL or UNINSTALLED by Kind AND Name

        _query.iterateResolvablesByKindsAndStringsAndRepos( kinds, _qstrings, cache::MATCH_EXACT|cache::MATCH_NAME, _options.repos(), cache_cb );
      }
      catch ( const Exception & excpt_r )
      {
        ZYPP_CAUGHT( excpt_r );
        cerr << "cache::ResolvableQuery failed: " << excpt_r.asUserString() << endl;
      }
    }
    return 0;
  }

  // search among all resolvables
  else
  {
    cerr_vv << "invokeOnEachSearched(): search among all resolvables" << endl;

    std::vector<zypp::Resolvable::Kind> kinds = _options.kinds();

    // default to packages
    if (kinds.empty()) {
      kinds.push_back( ResTraits<Package>::kind );
    }

    if (_options.installedFilter() != ZyppSearchOptions::UNINSTALLED_ONLY)
    {
      // search pool on ALL or INSTALLED
      invokeOnEach(pool.begin(), pool.end(), filter_r, pool_cb);
    }

    if (_options.installedFilter() != ZyppSearchOptions::INSTALLED_ONLY)
    {
      try
      {
        // search cache on ALL or UNINSTALLED by WILD NAME

        _query.iterateResolvablesByKindsAndStringsAndRepos( kinds, _qstrings, cache::MATCH_SUBSTRING|cache::MATCH_NAME, _options.repos(), cache_cb );
      }
      catch ( const Exception & excpt_r )
      {
        ZYPP_CAUGHT( excpt_r );
        cerr << "cache::ResolvableQuery failed: " << excpt_r.asUserString() << endl;
      }
    }
  }
  return 0;
}

/** PRIVATE
 * Cache installed packages matching given search criteria into a hash_map.
 * Assumption made: names of currently installed resolvables + kind
 * (+version???) are unique.
 */
void ZyppSearch::cacheInstalled() {
  // don't include kind string in hash map key if search is to be restricted
  // to particular kind (to improve performance a little bit)
  if (_options.kinds().size() == 1)
    _icache.setIncludeKindInKey(false);

  cout_v << _("Pre-caching installed resolvables matching given search criteria... ") << endl;

  ResStore tgt_resolvables(_zypp->target()->resolvables());

  _zypp->addResolvables(tgt_resolvables, true /*installed*/);

  invokeOnEachSearched( Match( _reg, _options.searchDescriptions()),
    functorRef<bool,const zypp::PoolItem &>(_icache), functorRef<bool, const data::RecordId &, data::ResObject_Ptr>(_icache)
  );

  cout_v << _icache.size() << _(" out of (") <<  tgt_resolvables.size() << ")"  
    << _("cached.") << endl;
}

/** PUBLIC
 * Invokes functor f on each pool item matching search criteria. 
 */
void
ZyppSearch::doSearch(const boost::function<bool(const PoolItem &)> & f, const zypp::cache::ProcessResolvable & r)
{
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
  
  filter = chain(filter,DuplicateFilter(_idcache));

  invokeOnEachSearched( filter, f, r );
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
  unsigned int flags = zypp::str::regex::normal;
  if (!_options.caseSensitive())
    flags |= zypp::str::regex::icase;

  // create regex object
  try {
     _reg.assign(regstr, flags);
  }
  catch (zypp::str::regex_error & e) {
    cerr << "ZyppSearch::setupRegexp(): " << regstr
      << _(" is not a valid regular expression: \"")
      << regstr << "\"" << endl;
    cerr << _("This is a bug, please file a bug report against zypper.") << endl;
    exit(1);
  }
}

/**
 * Converts '*' and '?' wildcards within str into their regex equivalents.
 */
string ZyppSearch::wildcards2regex(const string & str) const {
  string regexed = str;

  zypp::str::regex all("\\*"); // regex to search for '*'
  zypp::str::regex one("\\?"); // regex to search for '?'
  std::string r_all(".*"); // regex equivalent of '*'
  std::string r_one(".");  // regex equivalent of '?'
  std::string::size_type pos;

  // replace all "*" in input with ".*"
  for (pos = 0; (pos = regexed.find("*", pos)) != std::string::npos; pos+=2)
    regexed = regexed.replace(pos, 1, r_all);
  cerr_vv << "wildcards2regex: " << str << " -> " << regexed << std::endl;

  // replace all "?" in input with "."
  for (pos = 0; (pos = regexed.find('?', pos)) != std::string::npos; ++pos)
    regexed = regexed.replace(pos, 1, r_one);

  cerr_vv << " -> " << regexed << endl;

  return regexed;
}

// Local Variables:
// c-basic-offset: 2
// End:
