#include "zmart.h"
#include "zmart-sources.h"
#include "zypper-tabulator.h"
#include "zypper-callbacks.h"

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
    cerr << _("Sorry, you need root privileges to use system sources, disabling them...") << endl;
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
    cerr << _("Restoring system sources...") << endl;
    manager->restore(gSettings.root_dir);
  }
//  catch (const SourcesAlreadyRestoredException& excpt) {
//  }
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
    ERR << "Couldn't restore sources" << endl;
    cerr << _("Failed to restore sources") << endl;
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
    cerr << _("Can't access repository") << endl;
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
  if (gSettings.is_rug_compatible) th << _("Status");
  else th << _("Enabled") << _("Refresh");
  th << _("Type") << _("Name") << "URI";
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
      tr << _("Active");
#else
      tr << (source.enabled() ? _("Active") : _("Disabled"));
#endif
    }
    // zypper status (enabled, autorefresh)
    else
    {
#ifdef LIBZYPP_1xx
      tr << _("Yes");
      tr << (source.autorefresh ? _("Yes") : _("No"));
#else
      tr << (source.enabled() ? _("Yes") : _("No"));
      tr << (source.autorefresh() ? _("Yes") : _("No"));
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
    sources = SourceManager::sourceManager()->knownSourceInfos (gSettings.root_dir);
#else
    zypp::storage::PersistentStorage store;
    store.init( gSettings.root_dir );
    sources = store.storedSources();
#endif
  }
  catch ( const Exception &e )
  {
    cout << _("Error reading system sources: ") << e.msg() << std::endl;
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
	cerr << _("Unknown repository type ") << type << endl;
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
    cerr << _("Name not found") << endl;
  }  
  if (!have_url) {
    cerr << _("baseurl not found") << endl;
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

void add_source_by_url( const zypp::Url &url, const string &alias,
			const string &type, bool enabled, bool refresh  )
{
  cerr_vv << "Constructing SourceManager" << endl;
  SourceManager_Ptr manager = SourceManager::sourceManager();
  cerr_vv << "Restoring SourceManager" << endl;
  manager->restore (gSettings.root_dir, true /*use_cache*/);

  list<SourceManager::SourceId> sourceIds;

  Pathname path;
  Pathname cache;
  bool is_base = false;
  string myalias = alias.empty() ? timestamp() : alias;
   
  // more products?
  // try
  cerr_vv << "Creating source" << endl;
  Source_Ref source;
  if (type.empty()) {
    source = SourceFactory().createFrom( url, path, myalias, cache, is_base );
  }
  else {
    source = SourceFactory().createFrom( type,
					 url, path, myalias, cache, is_base,
					 refresh );
  }
  cerr_vv << "Adding source" << endl;
  SourceManager::SourceId sourceId = manager->addSource( source );

  if (enabled)
    source.enable();
  else
    source.disable();
  
  source.setAutorefresh (refresh);

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
    manager->store( gSettings.root_dir, true /*metadata_cache*/ );
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
  bool success = true;
  std::set<std::string> _broken_sources;
  try {
    manager->restore (gSettings.root_dir, true /*use_cache*/);
  }
  catch (const zypp::FailedSourcesRestoreException& ex)
  {
    ZYPP_CAUGHT (ex);
    _broken_sources = ex.aliases();
    success = false;
  }
  catch (const Exception & ex) {
    // so what if sources cannot be restored
    // we want to delete anyway
    ZYPP_CAUGHT (ex);
    cerr << ex.asUserString () << endl
	 << _("Continuing anyway") << endl;
    success = false;
  }

  SourceManager::SourceId sid = 0;
  safe_lexical_cast (anystring, sid);

  if (!success)
  {
    if (sid > 0 || looks_like_url (anystring))
    {
      cerr << "Broken sources found. Cannot remove sources by ID or URL. Please use alias instead." << endl;
      return;
    }

    zypp::storage::PersistentStorage store;
    store.init( gSettings.root_dir );
    list<source::SourceInfo> known_sources = store.storedSources();

    bool is_known = false;
    for (list<source::SourceInfo>::const_iterator it = known_sources.begin();
          it != known_sources.end(); ++it)
      if(it->alias() == anystring)
      {
        is_known = true;
        break;
      }

    if (is_known)
    {
      try
      {
        cout_v << format(_("Removing source '%s'")) % anystring << endl;
        store.deleteSource( anystring );
        _broken_sources.erase( anystring );
        cout << format(_("Source '%s' removed.")) % anystring << endl;
      }
      catch( const zypp::Exception& excpt )
      {
        cerr << format(_("Failed to remove source '%s':")) % anystring << endl;
      }
    }
    else
    {
      cerr << format (_("Source %s not found.")) % anystring << endl;
    }

    if (_broken_sources.size() == 0)
      return;

    // offer to remove the broken sources
    for (set<string>::const_iterator it = _broken_sources.begin();
         it != _broken_sources.end(); ++it)
    {
      string prompt = boost::str(format(_("'%s' is broken or not accessible. Do you wish to remove it?")) % *it);
      if(read_bool_answer(prompt, false))
      {
        try
        {
          cout_v << format(_("Removing source '%s'")) % *it << endl;
          store.deleteSource( *it );
          cout << format(_("Source '%s' removed.")) % *it << endl;
        }
        catch( const zypp::Exception& excpt )
        {
          cerr << format(_("Failed to remove source '%s':")) % *it << endl;
        }
      }
    }

    return;
  }

  if (sid > 0) {
    cerr_v << _("Removing source ") << sid << endl;
    try {
      manager->findSource (sid);
    }
    catch (const Exception & ex) {
      ZYPP_CAUGHT (ex);
      // boost::format: %s is fine regardless of the actual type :-)
      cerr << format (_("Source %s not found.")) % sid << endl;
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
	  cerr << _("URL is invalid: ") << excpt_r.asUserString() << endl;
	}
	if (url.isValid ()) {
	  try {
	    manager->findSourceByUrl (url);
	  }
	  catch (const Exception & ex) {
	    ZYPP_CAUGHT (ex);
	    cerr << format (_("Source %s not found.")) % url.asString() << endl;
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
	cerr << format (_("Source %s not found.")) % anystring << endl;
      }
      manager->removeSource (anystring); 	// by alias
    }
  }

  cerr_vv << "Storing source data" << endl;
  manager->store( gSettings.root_dir, true /*metadata_cache*/ );
}

//! rename a source, identified in any way: alias, url, id
void rename_source( const std::string& anystring, const std::string& newalias )
{
#ifdef LIBZYPP_1xx
  // renameSource is recent
  cerr << "Sorry, not implemented yet for libzypp-1.x.x" << endl;
#else
  cerr_vv << "Constructing SourceManager" << endl;
  SourceManager_Ptr manager = SourceManager::sourceManager();
  cerr_vv << "Restoring SourceManager" << endl;
  manager->restore (gSettings.root_dir, true /*use_cache*/);

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
      cerr << format (_("Source %s not found.")) % sid << endl;
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
	  cerr << _("URL is invalid: ") << excpt_r.asUserString() << endl;
	}
	if (url.isValid ()) {
	  try {
	    src = manager->findSourceByUrl (url);
	  }
	  catch (const Exception & ex) {
	    ZYPP_CAUGHT (ex);
	    cerr << format (_("Source %s not found.")) % url.asString() << endl;
	  }
	}
    }

    if (!is_url) {
      try {
	src = manager->findSource (anystring);
      }
      catch (const Exception & ex) {
	ZYPP_CAUGHT (ex);
	cerr << format (_("Source %s not found.")) % anystring << endl;
      }
    }
  }

  if (src) {
    // getting Source_Ref is useless if we only can use an id
    manager->renameSource (src.numericId (), newalias);
  }

  cerr_vv << "Storing source data" << endl;
  manager->store( gSettings.root_dir, true /*metadata_cache*/ );
#endif
}

void refresh_sources()
{
#ifdef LIBZYPP_1xx
  cerr << _("Sorry, not implemented yet for libzypp-1.x.x") << endl;
#else
  zypp::storage::PersistentStorage store;
  std::list<SourceInfo> sources;

  try
  {
    store.init( gSettings.root_dir );
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
      cout << _("DONE") << endl << endl;
    }
    catch ( const zypp::Exception & ex )
    {
      cerr << _("Error while refreshing the source: ") << ex.asString();
      // continuing with next source, however
    }
  }

  cout << _("All system sources have been refreshed.") << endl;
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
    cerr << _("Error while fetching ") << filename_or_url << " : "
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

// #217028
void warn_if_zmd () {
  if (system ("pgrep -lx zmd") == 0) { // list name, exact match
    cerr << _("ZENworks Management Daemon is running.") << endl
	 << _("WARNING: this command will not synchronize changes.") << endl
	 << _("Use rug or yast2 for that.") << endl;
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
