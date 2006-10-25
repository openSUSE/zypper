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
#include <fstream>
#include <sstream>

#include "checkpatches-keyring-callbacks.h"
#include "zmart.h"
#include "zmart-updates.h"

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::CheckPatches"

#define XML_FORMAT_VERSION "0.3"

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace boost;

ZYpp::Ptr God;
RuntimeData gData;
Settings gSettings;

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
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
    
    std::ofstream os(RESULT_FILE);
    if ( os.good() )
    {
      render_error( Edition(XML_FORMAT_VERSION), os, "a ZYpp transaction is already in progress.");
      render_error( Edition(XML_FORMAT_VERSION), cout, "a ZYpp transaction is already in progress.");
      os.close();
    }
     // save a random token so we try again next time
     save_token(utils::randomString(48));
     save_version(Edition(XML_FORMAT_VERSION));
    return -1;
  }
  
  SourceManager_Ptr manager;
  manager = SourceManager::sourceManager();
  
  KeyRingCallbacks keyring_callbacks;
  DigestCallbacks digest_callbacks;
  
  try
  {
    manager->restore("/");
  }
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
    
    std::ofstream os(RESULT_FILE);
    if ( os.good() )
    {
      string error = "Couldn't restore sources" + ( excpt_r.msg().empty() ? "\n" : (":\n" + excpt_r.msg()));
      render_error( Edition(XML_FORMAT_VERSION), os, error);
      render_error( Edition(XML_FORMAT_VERSION), cout, error);
      os.close();
    }
    
    // save a random token so we try again next time
     save_token(utils::randomString(48));
     save_version(Edition(XML_FORMAT_VERSION));
     
    return -1;
  }
  
  // dont add rpms
  God->initTarget("/", true);
  
  std::string token;
  stringstream token_stream;
  
  for ( SourceManager::Source_const_iterator it = manager->Source_begin(); it !=  manager->Source_end(); ++it )
  {
    Source_Ref src = manager->findSource(it->alias());
    src.refresh();
    
    token_stream << "[" << src.alias() << "| " << src.url() << src.timestamp() << "]";
    
    MIL << "Source: " << src.alias() << " from " << src.timestamp() << std::endl;
    
    // skip sources without patches sources for now
    if ( src.hasResolvablesOfKind( ResTraits<zypp::Patch>::kind ) )
    {
      MIL << "Including source " << src.url() << std::endl;
      gData.sources.push_back(src);
    }
    else
    {
      MIL << "Excluding source " << src.url() << " ( no patches ) "<< std::endl;
    }
  }
  
  token_stream << "[" << "target" << "| " << God->target()->timestamp() << "]";
  
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
  
  for ( std::list<Source_Ref>::const_iterator it = gData.sources.begin(); it != gData.sources.end(); ++it )
  {
    God->addResolvables(it->resolvables());
  }
  
  God->addResolvables( God->target()->resolvables(), true);
  
  God->resolver()->establishPool();
  
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


