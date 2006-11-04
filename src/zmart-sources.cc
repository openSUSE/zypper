
#include "zmart.h"
#include "zmart-sources.h"
#include "zypper-tabulator.h"

#include <fstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <zypp/target/store/PersistentStorage.h>
#include <zypp/base/IOStream.h>


using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;
extern RuntimeData gData;
extern Settings gSettings;

void cond_init_system_sources ()
{
  static bool done = false;
  if (done)
    return;

  if ( geteuid() != 0 ) {
    cerr << "Sorry, you need root privileges to use system sources, disabling them..." << endl;
    gSettings.disable_system_sources = true;
    MIL << "system sources disabled" << endl;
  }

  if ( ! gSettings.disable_system_sources ) {
    init_system_sources();
  }
  done = true;
} 

void init_system_sources()
{
  SourceManager_Ptr manager;
  manager = SourceManager::sourceManager();
  try
  {
    cerr << "Restoring system sources..." << endl;
    manager->restore("/");
  }
//  catch (const SourcesAlreadyRestoredException& excpt) {
//  }
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
    ERR << "Couldn't restore sources" << endl;
    cerr << "Failed to restore sources" << endl;
    exit(-1);
  }
    
  for ( SourceManager::Source_const_iterator it = manager->Source_begin(); it !=  manager->Source_end(); ++it )
  {
    Source_Ref src = manager->findSource(it->alias());
    gData.sources.push_back(src);
  }
}

void include_source_by_url( const Url &url )
{
  try
  {
    //cout << "Creating source from " << url << endl;
    Source_Ref src;
    src = SourceFactory().createFrom(url, "/", url.asString(), "");
    //cout << "Source created.. " << endl << src << endl;
    gData.sources.push_back(src);
  }
  catch( const Exception & excpt_r )
  {
    cerr << "Can't access repository" << endl;
    ZYPP_CAUGHT( excpt_r );
    exit(-1);
  }
  
}

#ifdef LIBZYPP_1xx
typedef zypp::SourceManager::SourceInfo SourceInfo;
#else
using zypp::source::SourceInfo;
#endif

static void print_source_list(const std::list<SourceInfo> &sources )
{
  Table tbl;
  TableHeader th;
  th << "#";
  if (gSettings.is_rug_compatible) th << "Status";
  else th << "Enabled" << "Refresh";
  th << "Type" << "Name" << "URI";
  tbl << th;

  int i = 1;
  for( std::list<SourceInfo>::const_iterator it = sources.begin() ;
       it != sources.end() ; ++it, ++i )
  {
    SourceInfo source = *it;
    TableRow tr (gSettings.is_rug_compatible ? 5 : 6);
    tr << str::numstring (i);

    // rug's status (active, pending => active, disabled <= enabled, disabled)
    // this is probably the closest possible compatibility arrangement
    if (gSettings.is_rug_compatible)
    {
#ifdef LIBZYPP_1xx
      tr << "Active";
#else
      tr << (source.enabled() ? "Active" : "Disabled");
#endif
    }
    // zypper status (enabled, autorefresh)
    else
    {
#ifdef LIBZYPP_1xx
      tr << "Yes";
      tr << (source.autorefresh ? "Yes" : "No");
#else
      tr << (source.enabled() ? "Yes" : "No");
      tr << (source.autorefresh() ? "Yes" : "No");
#endif
    }

#ifdef LIBZYPP_1xx
    tr << source.type;
    tr << source.alias;
    tr << source.url.asString();
#else
    tr << source.type();
    tr << source.alias();
    tr << source.url().asString();
#endif
    tbl << tr;
  }
  cout << tbl;
}

void list_system_sources()
{
  std::list<SourceInfo> sources;
  
  try
  {
#ifdef LIBZYPP_1xx
    sources = SourceManager::sourceManager()->knownSourceInfos ("/");
#else
    zypp::storage::PersistentStorage store;
    store.init( "/" );
    sources = store.storedSources();
#endif
  }
  catch ( const Exception &e )
  {
    cout << "Error reading system sources: " << e.msg() << std::endl;
    exit(-1); 
  }
  
  print_source_list(sources);
}

bool parse_repo_file (const string& file, string& url, string& alias)
{
  static const boost::regex
    r_alias ("^\\[(.*)\\]$"),
    r_type ("^type=(.*)"),
    r_url ("^baseurl=(.*)");
  boost::smatch match;

  std::ifstream repo(file.c_str());
  bool have_alias = false, have_url = false;
  while (repo.good ()) {
    string line = zypp::iostr::getline (repo);

    if (regex_search (line, match, r_alias)) {
      alias = match[1];
      have_alias = true;
    }
    else if (regex_search (line, match, r_type)) {
      string type = match[1];
      if (type != "rpm-md" && type != "yast2") {
	cerr << "Unknown repository type " << type << endl;
	return false;
      }
    }
    else if (regex_search (line, match, r_url)) {
      url = match[1];
      have_url = true;
    }
  }
  repo.close ();

  if (!have_alias) {
    cerr << "Name not found" << endl;
  }  
  if (!have_url) {
    cerr << "baseurl not found" << endl;
  }  
  cerr_vv << "Name: " << alias << endl;
  cerr_vv << "URL: " << url << endl;

  return have_alias && have_url;
}

static
std::string timestamp ()
{
  time_t t = time(NULL);
  struct tm * tmp = localtime(&t);

  if (tmp == NULL) {
    return "";
  }

  char outstr[50];
  if (strftime(outstr, sizeof(outstr), "%Y%m%d-%H%M%S", tmp) == 0) {
    return "";
  }
  return outstr;
}

void add_source_by_url( const zypp::Url &url, std::string alias  )
{
  cerr_vv << "Constructing SourceManager" << endl;
  SourceManager_Ptr manager = SourceManager::sourceManager();
  cerr_vv << "Restoring SourceManager" << endl;
  manager->restore ("/", true /*use_cache*/);

  list<SourceManager::SourceId> sourceIds;

  Pathname path;
  Pathname cache;
  bool is_base = false;
  if (alias.empty ())
    alias = timestamp();
   
  // more products?
  // try
  cerr_vv << "Creating source" << endl;
  Source_Ref source = SourceFactory().createFrom( url, path, alias, cache, is_base );
  cerr_vv << "Adding source" << endl;
  SourceManager::SourceId sourceId = manager->addSource( source );

  //if (enableSource)
    source.enable();
  //else
  //  source.disable();
  
  //source.setAutorefresh (autoRefresh);

    sourceIds.push_back( sourceId );
      cout << "Added Installation Sources:" << endl;
  
    list<SourceManager::SourceId>::const_iterator it;
    for( it = sourceIds.begin(); it != sourceIds.end(); ++it ) {
      Source_Ref source = manager->findSource(*it);
      cout << ( source.enabled() ? "[x]" : "[ ]" );
      cout << ( source.autorefresh() ? "* " : "  " );
      cout << source.alias() << " (" << source.url() << ")" << endl;
    }

    cerr_vv << "Storing source data" << endl;
    manager->store( "/", true /*metadata_cache*/ );
}

template<typename T>
ostream& operator << (ostream& s, const vector<T>& v) {
  std::copy (v.begin(), v.end(), ostream_iterator<T> (s, ", "));
  return s;
}

template <typename Target, typename Source>
void safe_lexical_cast (Source s, Target &tr) {
  try {
    tr = boost::lexical_cast<Target> (s);
  }
  catch (boost::bad_lexical_cast &) {
  }
}

static
bool looks_like_url (const string& s) {
  static bool schemes_shown = false;
  if (!schemes_shown) {
    cerr_vv << "Registered schemes: " << Url::getRegisteredSchemes () << endl;
    schemes_shown = true;
  }

  string::size_type pos = s.find (':');
  if (pos != string::npos) {
    string scheme (s, 0, pos);
    if (Url::isRegisteredScheme (scheme)) {
      return true;
    }
  }
  return false;
}

//! remove a source, identified in any way: alias, url, id
// may throw:
void remove_source( const std::string& anystring )
{
  cerr_vv << "Constructing SourceManager" << endl;
  SourceManager_Ptr manager = SourceManager::sourceManager();
  cerr_vv << "Restoring SourceManager" << endl;
  try {
    manager->restore ("/", true /*use_cache*/);
    }
  catch (const Exception & ex) {
    // so what if sources cannot be restored
    // we want to delete anyway
    ZYPP_CAUGHT (ex);
    cerr << ex.asUserString () << endl
	 << "Continuing anyway" << endl;
  }

  SourceManager::SourceId sid = 0;
  safe_lexical_cast (anystring, sid);
  if (sid > 0) {
    cerr_v << "removing source " << sid << endl;
    try {
      manager->findSource (sid);
    }
    catch (const Exception & ex) {
      ZYPP_CAUGHT (ex);
      // boost::format: %s is fine regardless of the actual type :-)
      cerr << format ("Source %s not found.") % sid << endl;
    }
    manager->removeSource (sid);
  }
  else {
    bool is_url = false;
    if (looks_like_url (anystring)) {
	is_url = true;
	cerr_vv << "Looks like a URL" << endl;

	Url url;
	try {
	  url = Url (anystring);
	}
	catch ( const Exception & excpt_r ) {
	  ZYPP_CAUGHT( excpt_r );
	  cerr << "URL is invalid: " << excpt_r.asUserString() << endl;
	}
	if (url.isValid ()) {
	  try {
	    manager->findSourceByUrl (url);
	  }
	  catch (const Exception & ex) {
	    ZYPP_CAUGHT (ex);
	    cerr << format ("Source %s not found.") % url.asString() << endl;
	  }
	  manager->removeSourceByUrl (url);
	}
    }

    if (!is_url) {
      try {
	manager->findSource (anystring);
      }
      catch (const Exception & ex) {
	ZYPP_CAUGHT (ex);
	cerr << format ("Source %s not found.") % anystring << endl;
      }
      manager->removeSource (anystring); 	// by alias
    }
  }

  cerr_vv << "Storing source data" << endl;
  manager->store( "/", true /*metadata_cache*/ );
}

//! rename a source, identified in any way: alias, url, id
void rename_source( const std::string& anystring, const std::string& newalias )
{
  cerr_vv << "Constructing SourceManager" << endl;
  SourceManager_Ptr manager = SourceManager::sourceManager();
  cerr_vv << "Restoring SourceManager" << endl;
  manager->restore ("/", true /*use_cache*/);

  Source_Ref src;

  SourceManager::SourceId sid = 0;
  safe_lexical_cast (anystring, sid);
  if (sid > 0) {
    try {
      src = manager->findSource (sid);
    }
    catch (const Exception & ex) {
      ZYPP_CAUGHT (ex);
      // boost::format: %s is fine regardless of the actual type :-)
      cerr << format ("Source %s not found.") % sid << endl;
    }
  }
  else {
    bool is_url = false;
    if (looks_like_url (anystring)) {
	is_url = true;
	cerr_vv << "Looks like a URL" << endl;

	Url url;
	try {
	  url = Url (anystring);
	}
	catch ( const Exception & excpt_r ) {
	  ZYPP_CAUGHT( excpt_r );
	  cerr << "URL is invalid: " << excpt_r.asUserString() << endl;
	}
	if (url.isValid ()) {
	  try {
	    src = manager->findSourceByUrl (url);
	  }
	  catch (const Exception & ex) {
	    ZYPP_CAUGHT (ex);
	    cerr << format ("Source %s not found.") % url.asString() << endl;
	  }
	}
    }

    if (!is_url) {
      try {
	src = manager->findSource (anystring);
      }
      catch (const Exception & ex) {
	ZYPP_CAUGHT (ex);
	cerr << format ("Source %s not found.") % anystring << endl;
      }
    }
  }

  if (src) {
    src.setAlias (newalias);
  }

  cerr_vv << "Storing source data" << endl;
  manager->store( "/", true /*metadata_cache*/ );
}

void refresh_sources()
{
#ifdef LIBZYPP_1xx
  cerr << "Sorry, not implemented yet for libzypp-1.x.x" << endl;
#else
  zypp::storage::PersistentStorage store;
  std::list<SourceInfo> sources;

  try
  {
    store.init( "/" );
    sources = store.storedSources();
  }
  catch ( const Exception &e )
  {
    cerr << _("Error reading system sources: ") << e.msg() << std::endl;
    exit(-1); 
  }

  for(std::list<SourceInfo>::const_iterator it = sources.begin();
       it != sources.end() ; ++it)
  {
    try
    {
      cout << _("Refreshing ") << it->alias() << endl <<
        "URI: " << it->url() << endl; 
      Source_Ref src = SourceFactory().createFrom(
        it->type(), it->url(), it->path(), it->alias(), it->cacheDir(),
        false, // base source
        true); // autorefresh
//      src.refresh();
      cout << "DONE" << endl << endl;
    }
    catch ( const zypp::Exception & ex )
    {
      cerr << _("Error while refreshing the source: ") << ex.asString();
      // continuing with next source, however
    }
  }

  cout << _("All sytem sources have been refreshed.") << endl;
#endif
}

MediaWrapper::MediaWrapper (const string& filename_or_url) {
  try {
    // the interface cannot provide a "complete path" :-(
    Url url (filename_or_url);
    Pathname path (url.getPathName ());
    url.setPathName ("/");

    _id = _mm.open (url);
    _mm.attach (_id);

    _mm.provideFile (_id, path);
    Pathname local = _mm.localPath (_id, path);
    _local_path = local.asString ();
  }
  catch (const Exception & ex) {
    ZYPP_CAUGHT (ex);
    if (looks_like_url (filename_or_url)) {
    cerr << "Error while fetching " << filename_or_url << " : "
	 << ex << endl;
//      ex.dumpOn (cerr);		// this suxxz
    }
    _local_path = filename_or_url;
  }
}

MediaWrapper::~MediaWrapper () {
  if (_mm.isOpen (_id))
    _mm.close (_id);
}

// Local Variables:
// c-basic-offset: 2
// End:
