/**
 * installation_sources
 */

#include <getopt.h>
#include <time.h>

#include <iomanip>
#include <list>
#include <string>

#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/SourceFactory.h>
#include <zypp/SourceManager.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>
#include <zypp/base/LogControl.h>
#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>
#include <zypp/base/Algorithm.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "installation_sources"

using namespace std;
using namespace zypp;

bool callbackAnswer = false;

static
bool readCallbackAnswer()
{
  return callbackAnswer;
}

///////////////////////////////////////////////////////////////////
// KeyRingReceive
///////////////////////////////////////////////////////////////////
struct KeyRingReceive : public zypp::callback::ReceiveReport<zypp::KeyRingReport>
{
  bool _enabled;
  KeyRingReceive()
    : _enabled( true )
    {
    }

  void disable( bool value )
    {
      _enabled = !value;
      MIL << "KeyRingReceive is now " << (_enabled ? "en" : "dis") << "abled." << std::endl;
    }
 
  virtual bool askUserToAcceptUnsignedFile( const std::string &file )
    {
      if (!_enabled) return true;
      DBG << "21|" << file << std::endl;
      std::cout << "21|" << file << std::endl;
      return readCallbackAnswer();
    }
  virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &keyid, const std::string &keyname, const std::string &fingerprint )
    {
      if (!_enabled) return true;
      DBG << "22|" << file << "|" << keyid << "|" << keyname << "|" << fingerprint << std::endl;
      std::cout << "22|" << file << "|" << keyid << "|" << keyname << "|" << fingerprint << std::endl;
      return readCallbackAnswer();
    }
  virtual bool askUserToTrustKey( const std::string &keyid, const std::string &keyname, const std::string &fingerprint )
    {
      if (!_enabled) return true;
      DBG << "23|" << keyid << "|" << keyname <<  "|" << fingerprint << std::endl;
      std::cout << "23|" << keyid << "|" << keyname <<  "|" << fingerprint << std::endl;
      return readCallbackAnswer();
    }
  virtual bool askUserToAcceptVerificationFailed( const std::string &file, const std::string &keyid, const std::string &keyname, const std::string &fingerprint )
    {
      if (!_enabled) return true;
      DBG << "24|" << file << "|" << keyid << "|" << keyname << "|" << fingerprint << std::endl;
      std::cout << "24|" << file << "|" << keyid << "|" << keyname << "|" << fingerprint << std::endl;
      return readCallbackAnswer();
    }
};


struct DigestReceive : public zypp::callback::ReceiveReport<zypp::DigestReport>
{
  bool _enabled;
  DigestReceive()
    : _enabled( true )
    {
    }

  void disable( bool value )
    {
      _enabled = !value;
      MIL << "DigestReceive is now " << (_enabled ? "en" : "dis") << "abled." << std::endl;
    }
 
  virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
    {
      if (!_enabled) return true;
      DBG << "25|" << file << std::endl;
      std::cout << "25|" << file << std::endl;
      return readCallbackAnswer();
    }
  virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
    {
      if (!_enabled) return true;
      DBG << "26|" << file << "|" << name << std::endl;
      std::cout << "26|" << file << "|" << name << std::endl;
      return readCallbackAnswer();
    }
  virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
    {
      if (!_enabled) return true;
      DBG << "27|" << file << "|" << requested << "|" << found << std::endl;
      std::cout << "27|" << file << "|" << requested << "|" << found << std::endl;
      return readCallbackAnswer();
    }
};

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

void usage()
{
  // TODO: -r remove
  cout << "Usage:" << endl
       << "  installation_sources [options] -a url   Add source at given URL." << endl
       << "    -n name Name the source." << endl
       << "    -e  Enable source. This is the default." << endl
       << "    -d  Disable source." << endl
       << "    -f  Autorefresh source." << endl
       << "    -Y  Answer Yes to all checksum and signature questions." << endl
       << "  installation_sources -s       Show all available sources." << endl;
  exit( 1 );
}

/******************************************************************
 **
 **
 **	FUNCTION NAME : main
 **	FUNCTION TYPE : int
 **
 **	DESCRIPTION :
 */
int main( int argc, char **argv )
{
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile == NULL)
    logfile = "/var/log/YaST2/y2log";
  zypp::base::LogControl::instance().logfile( logfile );
/*
  else
  zypp::base::LogControl::instance().logfile( ZMD_BACKEND_LOG );
*/

  MIL << "START" << endl;

  const char *urlStr = 0;
  string alias;

  bool showSources  = false;
  bool addSource    = false;
  bool enableSource = true; // -e per default
  bool autoRefresh  = false;

  int c;
  while( 1 ) {
    c = getopt( argc, argv, "desa:fYn:" );
    if ( c < 0 ) break;

    switch ( c ) {
	case 's':
	  showSources = true;
	  break;
	case 'a':
	  addSource = true;
	  urlStr = optarg;
	  break;
	case 'n':
	  alias = optarg;
	  break;
	case 'd':
	  enableSource = false;
	  break;
	case 'e':
	  enableSource = true;
	  break;
	case 'f':
	  autoRefresh = true;
	  break;
	case 'Y':
	  callbackAnswer = true;
	  break;
	default:
	  cerr << "Error parsing command line." << endl;
	case '?':
	case 'h':
	  usage();
    }
  }

  if ( showSources && addSource ) usage();
  if ( !showSources && !addSource ) usage();

  try {


    ZYpp::Ptr Z = NULL;
    try {
      Z = getZYpp();
    }
    catch ( const Exception & excpt_r ) {
      ZYPP_CAUGHT (excpt_r);
      cerr << "A transaction is already in progress." << endl;
      exit(1);
    }

    KeyRingReceive keyring_rcv;
    keyring_rcv.connect ();
    DigestReceive digest_rcv;
    digest_rcv.connect ();

    SourceManager_Ptr manager = SourceManager::sourceManager();
    manager->restore ("/", true /*use_cache*/);

    list<SourceManager::SourceId> sourceIds;

    if ( addSource ) {
      Url url;
      try {
	url = Url( urlStr );
      }
      catch ( const Exception & excpt_r ) {
	ZYPP_CAUGHT( excpt_r );
	cerr << "URL is invalid." << endl;
	cerr << excpt_r.asUserString() << endl;
	exit( 1 );
      }

      Pathname path;
      Pathname cache;
      bool is_base = false;
      if (alias.empty ())
	alias = timestamp ();
      // more products?
      // try
      Source_Ref source = SourceFactory().createFrom( url, path, alias, cache, is_base );
      SourceManager::SourceId sourceId = manager->addSource( source );

      if (enableSource)
	source.enable();
      else
	source.disable();
      source.setAutorefresh (autoRefresh);

      sourceIds.push_back( sourceId );
      cout << "Added Installation Sources:" << endl;
    }

    if ( showSources ) {
      sourceIds = manager->allSources ();
      cout << "Installation Sources:" << endl;
    }

    list<SourceManager::SourceId>::const_iterator it;
    for( it = sourceIds.begin(); it != sourceIds.end(); ++it ) {
      Source_Ref source = manager->findSource(*it);
      cout << ( source.enabled() ? "[x]" : "[ ]" );
      cout << ( source.autorefresh() ? "* " : "  " );
      cout << source.alias() << " (" << source.url() << ")" << endl;
    }
    if ( addSource ) {
      manager->store( "/", true /*metadata_cache*/ );
    }

    digest_rcv.disconnect ();
    keyring_rcv.disconnect ();
  }
  catch ( const Exception & excpt_r ) {
    ZYPP_CAUGHT( excpt_r );
    cerr << excpt_r.asUserString() << endl;
    exit( 1 );
  }
  catch ( const exception & excpt_r ) {
    cerr << excpt_r.what() << endl;
    exit( 1 );
  }

  MIL << "END" << endl;
  return 0;
}
