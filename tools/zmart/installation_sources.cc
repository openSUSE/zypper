/**
 * installation_sources
 */

#include <getopt.h>
#include <time.h>

#include <iomanip>
#include <list>
#include <string>

#include <boost/format.hpp>

#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/SourceFactory.h>
#include <zypp/SourceManager.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>
#include <zypp/base/LogControl.h>
#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "installation_sources"

using namespace std;
using namespace zypp;
using boost::format;

bool callbackAnswer = false;

static
bool readCallbackAnswer()
{
  cerr << "> " << (callbackAnswer? "yes": "no") << endl;
  return callbackAnswer;
}

static
bool askIt (const boost::format & msg)
{
  MIL << msg << endl;
  cerr << msg << endl;
  return readCallbackAnswer ();
}

///////////////////////////////////////////////////////////////////
// KeyRingReceive
///////////////////////////////////////////////////////////////////
struct KeyRingReceive : public zypp::callback::ReceiveReport<zypp::KeyRingReport>
{
  virtual bool askUserToAcceptUnsignedFile( const std::string &file )
    {
      return askIt (
	format(_("File %s is not signed.\n"
		 "Use it anyway?"))
	% file
	);
    }

  virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &keyid, const std::string &keyname, const std::string &fingerprint )
    {
      return askIt (
	format(_("File %s is signed with an unknown key:\n"
		 "%s|%s|%s\n"
		 "Use the file anyway?"))
	% file % keyid % keyname % fingerprint
	);
    }

  virtual bool askUserToTrustKey( const std::string &keyid, const std::string &keyname, const std::string &fingerprint )
    {
      return askIt (
	format(_("Untrusted key found:\n"
		 "%s|%s|%s\n"
		 "Trust key?"))
	% keyid % keyname % fingerprint
	);
    }

  virtual bool askUserToAcceptVerificationFailed( const std::string &file, const std::string &keyid, const std::string &keyname, const std::string &fingerprint )
    {
      return askIt (
	format(_("File %s failed integrity check with the folowing key:\n"
		 "%s|%s|%s\n"
		 "Use the file anyway?"))
	% file % keyid % keyname % fingerprint
	);
    }
};


struct DigestReceive : public zypp::callback::ReceiveReport<zypp::DigestReport>
{
  virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
    {
      return askIt (
	format(_("File %s does not have a checksum.\n"
		 "Use the file anyway?"))
	% file
	);
    }

  virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
    {
      return askIt (
	format(_("File %s has an unknown checksum %s.\n"
		 "Use the file anyway?"))
	% file % name
	);
    }

  virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
    {
      return askIt (
	format(_("File %s has an invalid checksum.\n"
		 "Expected %s, found %s\n"
		 "Use the file anyway?"))
	% file % requested % found
	);
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
