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
using namespace boost;

#define ZYPP_CHECKPATCHES_LOG "/var/log/zypp-checkpatches.log"

#define TOKEN_FILE "/var/lib/zypp/cache/updates_token"
#define RESULT_FILE "/var/lib/zypp/cache/updates_result.xml"

ZYpp::Ptr God;
RuntimeData gData;
Settings gSettings;

ostream no_stream(NULL);

static string read_old_token()
{
  string buffer;
  string token;
  std::ifstream is(TOKEN_FILE);
  if ( is.good() )
  {
    while(is && !is.eof())
    {
      getline(is, buffer);
      token += buffer;
    }
    is.close();
  }
  return token;
}


static void save_token( const std::string &token )
{
  std::ofstream os(TOKEN_FILE);
  if ( os.good() )
  {
    os << token << endl;;
  }
  os.close();
}

static void render_error(  std::ostream &out, const std::string &reason )
{
  out << "<update-status op=\"error\">" << std::endl;
    out << "<error>" << reason << "</error>" << std::endl;
  out << "</update-status>" << std::endl;
}

static void render_unchanged(  std::ostream &out, const std::string &token )
{
  out << "<update-status op=\"unchanged\">" << std::endl;
  //  out << " <metadata token=\"" << token << "\"/>" << std::endl;
  out << "</update-status>" << std::endl;
}

static void render_result( std::ostream &out, const zypp::ResPool &pool)
{
  int count = 0;
  int security_count = 0;
  
  out << "<?xml>" << std::endl;
  out << "<update-status op=\"success\">" << std::endl;
  //out << " <metadata token=\"" << token << "\"/>" << std::endl;
  out << " <update-sources>" << std::endl;
  for ( std::list<Source_Ref>::const_iterator it = gData.sources.begin(); it != gData.sources.end(); ++it )
  {
    out << "  <source url=\"" << it->url() << "\" alias=\"" << it->alias() << "\">" << std::endl;
  }
  out << " </update-sources>" << std::endl;
  out << " <update-list>" << std::endl;
  for ( ResPool::byKind_iterator it = pool.byKindBegin<Patch>(); it != pool.byKindEnd<Patch>(); ++it )
  {
    Resolvable::constPtr res = it->resolvable();
    Patch::constPtr patch = asKind<Patch>(res);
    MIL << patch->name() << " " << patch->edition() << " " << "[" << patch->category() << "]" << ( it->status().isNeeded() ? " [needed]" : " [unneeded]" )<< std::endl;
    if ( it->status().isNeeded() )
    {
      out << " <update category=\"" << patch ->category() << "\">" << std::endl;
      out << "  <name>" << patch->name() << "</name>" <<endl;
      out << "  <edition>" << patch->edition() << "</edition>" <<endl;
      
      
      count++;
      if (patch->category() == "security")
        security_count++;
    }
  }
  out << " </update-list>" << std::endl;
  out << " <update-summary total=\"" << count << "\" security=\"" << security_count << "\"/>" << std::endl;
  out << "</update-status>" << std::endl;
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
    
    std::ofstream os(TOKEN_FILE);
    if ( os.good() )
    {
      render_error( os, "a ZYpp transaction is already in progress.");
      render_error( cout, "a ZYpp transaction is already in progress.");
      os.close();
    }
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
    
    std::ofstream os(TOKEN_FILE);
    if ( os.good() )
    {
      render_error( os, "Couldn't restore sources");
      render_error( cout, "Couldn't restore sources");
      os.close();
    }
    
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
    previous_token = RANDOM_TOKEN;
  
  //static std::string digest(const std::string& name, std::istream& is
  token = Digest::digest("sha1", token_stream);
  //cout << token;
  
  MIL << "new token [" << token << "]" << " previous: [" << previous_token << "]" << std::endl;
  
  // use the old result
  if ( token == previous_token )
  {
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
    render_result( os, God->pool());
    render_result( cout, God->pool());
    os.close();
  }
   // save token
  save_token(token);
  //MIL << "Patches " << security_count << " " << count << std::endl;
  
  if ( security_count > 0 )
    return 2;
  
  if ( count > 0 )
    return 1;
  
  return 0;
}


