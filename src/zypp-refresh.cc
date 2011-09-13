/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

/* (c) Novell Inc. */

#include <iostream>

#include <zypp/ZYppFactory.h>
#include <zypp/base/LogControl.h>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>

#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>

#include <zypp/RepoManager.h>
#include <zypp/PathInfo.h>

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp-refresh"

#define ZYPP_REFRESH_LOG "/var/log/zypp-refresh.log"

using namespace std;
using namespace zypp;

// keyring and digest callbacks: accept everything, but don't import any keys

///////////////////////////////////////////////////////////////////
namespace zypp {
///////////////////////////////////////////////////////////////////

  static bool readCallbackAnswer()
  { return false; }

  ///////////////////////////////////////////////////////////////////
  // KeyRingReceive
  ///////////////////////////////////////////////////////////////////
  struct KeyRingReceive : public zypp::callback::ReceiveReport<zypp::KeyRingReport>
  {
    virtual bool askUserToAcceptUnsignedFile( const std::string &file, const KeyContext & context )
    { cerr << ". Error:" << endl << "refusing unsigned file " << file << endl;  return readCallbackAnswer(); }
    virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &id, const KeyContext & context )
    { cerr << ". Error:" << endl << "refusing unknown key, id: '" << id << "' from file '" << file << "'" << endl; return readCallbackAnswer(); }
    virtual KeyRingReport::KeyTrust askUserToAcceptKey( const PublicKey &key, const KeyContext & context )
    { cerr << ". Error:" << endl << "not trusting key '" << key << "'" << endl; return KeyRingReport::KEY_DONT_TRUST; }
    virtual bool askUserToAcceptVerificationFailed( const std::string &file, const PublicKey &key, const KeyContext & context )
    { cerr << ". Error:" << endl << "verification of '" << file << "' with key '" << key << "' failed" << endl; return readCallbackAnswer(); }
  };

  struct DigestReceive : public zypp::callback::ReceiveReport<zypp::DigestReport>
  {
    virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
    { cerr << ". Error:" << endl << "refusing file '" << file << "': no digest" << endl; return readCallbackAnswer(); }
    virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
    { cerr << ". Error:" << endl << "refusing file '" << file << "': unknown digest" << endl; return readCallbackAnswer(); }
    virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
    { cerr << ". Error:" << endl << "refusing file '" << file << "': wrong digest" << endl; return readCallbackAnswer(); }
  };
    ///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

class KeyRingCallbacks
{
  private:
    zypp::KeyRingReceive _keyRingReport;
  public:
    KeyRingCallbacks() { _keyRingReport.connect(); }
    ~KeyRingCallbacks() { _keyRingReport.disconnect(); }
};

class DigestCallbacks
{
  private:
    zypp::DigestReceive _digestReport;
  public:
    DigestCallbacks() { _digestReport.connect(); }
    ~DigestCallbacks() { _digestReport.disconnect(); }
};

int main(int argc, char **argv)
{
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile != NULL)
    zypp::base::LogControl::instance().logfile( logfile );
  else
    zypp::base::LogControl::instance().logfile( ZYPP_REFRESH_LOG );

  ZYpp::Ptr God;
  try
  {
    God = zypp::getZYpp();
  }
  catch ( const ZYppFactoryException & excpt_r )
  {
    ZYPP_CAUGHT (excpt_r);
    cerr <<
      "Could not access the package manager engine."
      " This usually happens when you have another application (like YaST)"
      " using it at the same time. Close the other applications and try again.";
    return 1; // the whole operation failed
  }
  catch ( const Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
    cerr << excpt_r.msg() << endl;
    return 1; // the whole operation failed
  }

  God->initializeTarget("/");
  RepoManager manager;

  KeyRingCallbacks keyring_callbacks;
  DigestCallbacks digest_callbacks;

  list<RepoInfo> repos;
  repos.insert(repos.end(), manager.repoBegin(), manager.repoEnd());
  MIL << "Found " << repos.size() << " repos." << endl;

  unsigned repocount = 0, errcount = 0;
  for(list<RepoInfo>::iterator it = repos.begin(); it != repos.end(); ++it, ++repocount)
  {
    Url url = it->url();
    string scheme(url.getScheme());

    if (scheme == "cd" || scheme == "dvd")
    {
      MIL << "Skipping CD/DVD repository: "
        "alias:[" << it->alias() << "] "
        "url:[" << url << "] " << endl;
      continue;
    }

    // refresh only enabled repos with enabled autorefresh (bnc #410791)
    if (!(it->enabled() && it->autorefresh()))
    {
      MIL << "Skipping disabled/no-autorefresh repository: "
        "alias:[" << it->alias() << "] "
        "url:[" << url << "] " << endl;
      continue;
    }

    MIL << "Going to refresh repository: "
      "alias:[" << it->alias() << "] "
      "url:[" << url << "] " << endl;

    try
    {
      cout << "refreshing '" << it->alias() << "' ." << flush;
      manager.refreshMetadata(*it);
      cout << "." << flush;
      manager.buildCache(*it);
      cout << ". Done." << endl;
    }
    catch (const Exception &excpt_r )
    {
      cerr
        << " Error:" << endl
        << str::form(
          "Could not refresh repository '%s':\n%s\n%s",
          it->name().c_str(), excpt_r.asUserString().c_str(), excpt_r.historyAsString().c_str())
        << endl;
      ++errcount;
    }
  }

  if (errcount)
  {
    if (repocount == errcount)
      return 1; // the whole operation failed (all of the repos)

    if (repocount > errcount)
      return 2; // some of the repos failed
  }

  // all right
  return 0;
}
