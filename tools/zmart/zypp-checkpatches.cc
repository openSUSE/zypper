#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include <zypp/base/LogControl.h>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/Locale.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/zypp_detail/ZYppReadOnlyHack.h>
#include <zypp/SourceManager.h>

#include <zypp/base/Logger.h>

#include <zypp/Digest.h>

#include "checkpatches-keyring-callbacks.h"
#include "zmart.h"

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::CheckPatches"

#define RANDOM_TOKEN "sad987432JJDJD948394DDDxxx22"

using namespace zypp::detail;

using namespace std;
using namespace zypp;

#define ZYPP_CHECKPATCHES_LOG "/var/log/zypp-checkpatches.log"

ZYpp::Ptr God;
RuntimeData gData;
Settings gSettings;

//using namespace DbXml;

int main(int argc, char **argv)
{
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile != NULL)
    zypp::base::LogControl::instance().logfile( logfile );
  else
    zypp::base::LogControl::instance().logfile( ZYPP_CHECKPATCHES_LOG );
  
  std::string previous_token;
  int previous_code = -1;
  
  if (argc != 3)
  {
    cerr << "usage: " << argv[0] << " [<previous token>] [previous result]" << endl;
    exit(-1);
  }
  
  MIL << argv[0] << " started with arguments " << argv[1] << " " << argv[2] << std::endl;
  
  previous_token = std::string(argv[1]);
  previous_code = str::strtonum<int>(argv[2]);
  
  ZYpp::Ptr God = NULL;
  try
  {
    God = zypp::getZYpp();
  }
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
    ERR  << "a ZYpp transaction is already in progress." << endl;
    cerr << "a ZYpp transaction is already in progress." << endl;
    cout << RANDOM_TOKEN;
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
    ERR << "Couldn't restore sources" << endl;
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
  }
  
  token_stream << "[" << "target" << "| " << God->target()->timestamp() << "]";
  
  //static std::string digest(const std::string& name, std::istream& is
  token = Digest::digest("sha1", token_stream);
  cout << token;
  
  MIL << "new token [" << token << "]" << " previous: [" << previous_token << "] previous code: " << previous_code << std::endl;
  if ( token == previous_token )
  {
    return previous_code;
  }
  
  // something changed
  for ( SourceManager::Source_const_iterator it = manager->Source_begin(); it !=  manager->Source_end(); ++it )
  {
    // skip non YUM sources for now
    if ( it->type() == "YUM" )
      God->addResolvables(it->resolvables());
  }
  
  God->addResolvables( God->target()->resolvables(), true);
  
  God->resolver()->establishPool();
  
  int count = 0;
  int security_count = 0;
  MIL << "Pool contains " << God->pool().size() << " items. Checking whether available patches are needed." << std::endl;
  for ( ResPool::byKind_iterator it = God->pool().byKindBegin<Patch>(); it != God->pool().byKindEnd<Patch>(); ++it )
  {
    Resolvable::constPtr res = it->resolvable();
    Patch::constPtr patch = asKind<Patch>(res);
    MIL << patch->name() << " " << patch->edition() << " " << "[" << patch->category() << "]" << ( it->status().isNeeded() ? " [needed]" : " [unneeded]" )<< std::endl;
    if ( it->status().isNeeded() )
    {
      count++;
      if (patch->category() == "security")
        security_count++;
      
      cerr << patch->name() << " " << patch->edition() << " " << "[" << patch->category() << "]" << std::endl;
    }
  }
  
  MIL << "Patches " << security_count << " " << count << std::endl;
  
  if ( security_count > 0 )
    return 2;
  
  if ( count > 0 )
    return 1;
  
  return 0;
}


