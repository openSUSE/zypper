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

#include "zypp/ZYppFactory.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"

#include "zypp/ZYppCallbacks.h"
#include "zypp/Pathname.h"
#include "zypp/KeyRing.h"
#include "zypp/Digest.h"

#include "zypp/RepoManager.h"
#include "zypp/PathInfo.h"

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp-refresh"

#define ZYPP_REFRESH_LOG "/var/log/zypp-refresh.log"

// keyring and digest callbacks: accept everything, but don't import any keys

///////////////////////////////////////////////////////////////////
namespace zypp {
///////////////////////////////////////////////////////////////////

  static bool readCallbackAnswer()
  { return true; }

  ///////////////////////////////////////////////////////////////////
  // KeyRingReceive
  ///////////////////////////////////////////////////////////////////
  struct KeyRingReceive : public zypp::callback::ReceiveReport<zypp::KeyRingReport>
  {
    virtual bool askUserToAcceptUnsignedFile( const std::string &file )
    { return readCallbackAnswer(); }
    virtual bool askUserToAcceptUnknownKey( const std::string &/*file*/, const std::string &/*id*/ )
    { return readCallbackAnswer(); }
    virtual bool askUserToTrustKey( const PublicKey &key )
    { return readCallbackAnswer(); }
    virtual bool askUserToAcceptVerificationFailed( const std::string &file, const PublicKey &key )
    { return readCallbackAnswer(); }
  };

  struct DigestReceive : public zypp::callback::ReceiveReport<zypp::DigestReport>
  {
    virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
    { return readCallbackAnswer(); }
    virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
    { return readCallbackAnswer(); }
    virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
    { return readCallbackAnswer(); }
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

using namespace std;
using namespace zypp;

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

  list<RepoInfo> repos = manager.knownRepositories();
  MIL << "Found " << repos.size() << " repos." << endl;

  unsigned repocount = 0, errcount = 0;
  for(list<RepoInfo>::iterator it = repos.begin(); it != repos.end(); ++it, ++repocount)
  {
    Url url = *(it->baseUrlsBegin());
    string scheme(url.getScheme());

    if (scheme == "cd" || scheme == "dvd")
    {
      MIL << "Skipping CD/DVD repository: "
        "alias:[" << it->alias() << "] "
        "url:[" << url << "] ";
      continue;
    }

    if (!it->enabled())
    {
      MIL << "Skipping disabled repository: "
        "alias:[" << it->alias() << "] "
        "url:[" << url << "] ";
      continue;
    }

    MIL << "Going to refresh repository: "
      "alias:[" << it->alias() << "] "
      "url:[" << url << "] ";

    try
    {
      cout << "refreshing '" << it->alias() << "' .";
      manager.refreshMetadata(*it);
      cout << ".";
      manager.buildCache(*it);
      cout << ". Done." << endl;
    }
    catch (const Exception &excpt_r )
    {
      cerr << ". Error." << endl;
      // translators the first %s is the repository name and the second is an error message
      cerr
        << str::form(
          "Could not refresh repository '%s':\n%s",
          it->name().c_str(), excpt_r.msg().c_str())
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
