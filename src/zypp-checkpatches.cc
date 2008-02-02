/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

/* (c) Novell Inc. */

#include <fcntl.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <libintl.h>

#include "zypp/RepoManager.h"
#include "zypp/PathInfo.h"

#include "checkpatches-keyring-callbacks.h"
#include "zypper-updates.h"

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::CheckPatches"

#define XML_FORMAT_VERSION "0.4"

//using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace boost;

ZYpp::Ptr God;
std::list<zypp::RepoInfo> repos;
std::list<Error> errors;

ostream no_stream(NULL);

namespace utils
{
  /*
    random funcions taken from kdelibs
    Copyright (C) 1998, 1999, 2000 KDE Team
  */
  int random()
  {
    static bool init = false;
    if (!init)
    {
        unsigned int seed;
        init = true;
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0 || ::read(fd, &seed, sizeof(seed)) != sizeof(seed))
        {
              // No /dev/urandom... try something else.
              srand(getpid());
              seed = rand()+time(0);
        }
        if (fd >= 0) close(fd);
        srand(seed);
    }
    return rand();
  }
  
  std::string randomString(int length)
  {
    if (length <=0 ) return std::string();
  
    std::string str; str.resize( length );
    int i = 0;
    while (length--)
    {
        int r=random() % 62;
        r+=48;
        if (r>57) r+=7;
        if (r>90) r+=6;
        str[i++] =  char(r);
        // so what if I work backwards?
    }
    return str;
  }

}


int exit_with_error( const std::string &error_str )
{
  errors.push_back(Error(error_str));
    
  std::ofstream os(RESULT_FILE);
  if ( os.good() )
  {
    render_error( Edition(XML_FORMAT_VERSION), os );
    render_error( Edition(XML_FORMAT_VERSION), cout );
    os.close();
  }
  // save a random token so we try again next time
  save_token(utils::randomString(48));
  save_version(Edition(XML_FORMAT_VERSION));
  return -1;
}

int main(int argc, char **argv)
{
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile != NULL)
    zypp::base::LogControl::instance().logfile( logfile );
  else
    zypp::base::LogControl::instance().logfile( ZYPP_CHECKPATCHES_LOG );
  
   
  ZYpp::Ptr God = NULL;
  try
  {
    God = zypp::getZYpp();
  }
  catch ( const ZYppFactoryException & excpt_r )
  {
    ZYPP_CAUGHT (excpt_r);
    string error_str( _("The updater could not access the package manager engine. This usually happens when you have another application (like YaST) using it at the same time. Please close the other applications and check again for updates." ) );
    return exit_with_error( error_str );
  }
  catch ( const Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
    return exit_with_error(excpt_r.msg());
  }
  
  RepoManager manager;

  KeyRingCallbacks keyring_callbacks;
  DigestCallbacks digest_callbacks;
  
  God->initializeTarget("/");
  
  std::string token;
  stringstream token_stream;
  
  token_stream << "[" << "target" << "| " << God->target()->timestamp() << "]";

  std::list<RepoInfo> new_sources = manager.knownRepositories();
  MIL << "Found " << new_sources.size()  << " repos." << endl;

  for (std::list<RepoInfo>::iterator it = new_sources.begin(); it != new_sources.end(); ++it)
  {
    Url url = *(it->baseUrlsBegin());

    std::string scheme( url.getScheme());

    if ( (scheme == "cd" || scheme == "dvd") )
    {
      MIL << "Skipping CD/DVD repository: url:[" << (it->baseUrlsBegin())->asString() << "] alias:[" << it->alias() << "] auto_refresh:[ " << it->autorefresh() << "]" << endl;
      continue;
    }

    if ( ! it->enabled() )
    {
      MIL << "Skipping disabled repository: url:[" << url.asString() << "] alias:[" << it->alias() << "] auto_refresh:[ " << it->autorefresh() << "]" << endl;
      continue;
    }

    // Note: Url(it->url).asString() to hide password in logs
    MIL << "Creating repository: url:[" << url.asString() << "] alias:[" << it->alias() << "] auto_refresh:[ " << it->autorefresh() << "]" << endl;

    try
    {
      manager.refreshMetadata(*it);
      manager.buildCache(*it);

      token_stream << "[" << it->alias() << "| " << *(it->baseUrlsBegin()) << "]"; // src.timestamp() << "]";

      MIL << "repository: " << it->alias() << std::endl; //" from " << src.timestamp() << std::endl;

      repos.push_back(*it);
    }
    catch (const Exception &excpt_r )
    {
      // TranslatorExplanation %s = detailed low level (unstranslated) error message
      string error = excpt_r.msg();
      errors.push_back(str::form(_("Couldn't restore repository.\nDetail: %s"), error.c_str()));
    }
  }

  string previous_token;
  if ( PathInfo(TOKEN_FILE).isExist() )
    previous_token = read_old_token();
  else
    previous_token = utils::randomString(40);;
  
  //static std::string digest(const std::string& name, std::istream& is
  token = Digest::digest("sha1", token_stream);
  //cout << token;
  
  MIL << "new token [" << token << "]" << " previous: [" << previous_token << "]" << std::endl;
  
  // use the old result
  if ( token == previous_token )
  {
    // check if the xml version is the same, otherwise regenerate
    Edition xml_version = read_old_version();
    
    if ( Edition(XML_FORMAT_VERSION) == xml_version )
    {
      // use the same old data
      std::ifstream is(RESULT_FILE);
    
      string buffer;
      while(is && !is.eof())
      {
        getline(is, buffer);
        cout << buffer << endl;
      }
      //return previous_code;
      return -1;
    }
  }
  else
  {
    MIL << "System has changed, recalculation of updates needed" << endl;
  }
  
  for ( std::list<RepoInfo>::const_iterator it = repos.begin(); it != repos.end(); ++it )
  {
    Repository repository = manager.createFromCache(*it);
    // FIXME fow now this will add repository resolvables
    repository.resolvables();
    //God->addResolvables();
  }

  if ( repos.size() == 0 )
  {
    errors.push_back( str::form( _( "There are no update repositories defined. Please add one or more update repositories in order to be notified of updates.") ) );
  }

  God->target()->load();
  //God->addResolvables( God->target()->resolvables(), true);
  
  // FIXME no need to establish?
  //God->resolver()->establishPool();
  
  int count = 0;
  int security_count = 0;
  MIL << "Pool contains " << God->pool().size() << " items. Checking whether available patches are needed." << std::endl;
  
  std::ofstream os(RESULT_FILE);
  if ( os.good() )
  {
    render_result( Edition(XML_FORMAT_VERSION), os, God->pool());
    render_result( Edition(XML_FORMAT_VERSION), cout, God->pool());
    os.close();
  }
   // save token
  save_token(token);
  save_version(Edition(XML_FORMAT_VERSION));
  //MIL << "Patches " << security_count << " " << count << std::endl;
  
  if ( security_count > 0 )
    return 2;
  
  if ( count > 0 )
    return 1;
  
  return 0;
}


