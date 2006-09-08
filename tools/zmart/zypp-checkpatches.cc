#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include <boost/program_options.hpp>

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

namespace po = boost::program_options;
#define ZYPP_CHECKPATCHES_LOG "/var/log/zypp-checkpatches.log"

ZYpp::Ptr God;
RuntimeData gData;
Settings gSettings;

//using namespace DbXml;

void render_xml( const zypp::ResPool &pool )
{
  int count = 0;
  int security_count = 0;
  
  cout << "<?xml>" << std::endl;
  cout << "<update-status>" << std::endl;
  cout << " <update-list>" << std::endl;
  for ( ResPool::byKind_iterator it = pool.byKindBegin<Patch>(); it != pool.byKindEnd<Patch>(); ++it )
  {
    Resolvable::constPtr res = it->resolvable();
    Patch::constPtr patch = asKind<Patch>(res);
    MIL << patch->name() << " " << patch->edition() << " " << "[" << patch->category() << "]" << ( it->status().isNeeded() ? " [needed]" : " [unneeded]" )<< std::endl;
    if ( it->status().isNeeded() )
    {
      cout << " <update category=\"" << patch ->category() << "\">" << std::endl;
      cout << "  <name>" << patch->name() << "</name>" <<endl;
      cout << "  <edition>" << patch->edition() << "</edition>" <<endl;
      
      
      count++;
      if (patch->category() == "security")
        security_count++;
    }
  }
  cout << " </update-list>" << std::endl;
  cout << " <update-summary total=\"" << count << "\" security=\"" << security_count << "\"/>" << std::endl;
  cout << "</update-status>" << std::endl;
}

int main(int argc, char **argv)
{
  const char *logfile = getenv("ZYPP_LOGFILE");
  if (logfile != NULL)
    zypp::base::LogControl::instance().logfile( logfile );
  else
    zypp::base::LogControl::instance().logfile( ZYPP_CHECKPATCHES_LOG );
  
  po::positional_options_description pos_options;
  pos_options.add("command", -1);
  
  po::options_description general_options("General options");
  general_options.add_options()
      ("help,h", "produce a help message")
      ("version,v", "output the version number")
      ;

  po::options_description check_options("Check options");
  check_options.add_options()
      ("previous-token,t", po::value< string >(), "The token got from last run.")
      ("previous-result,r", po::value< int >(), "Previous result. If repositories are the same as last run, this will be shown.")
      ;
  
  //po::options_description source_options("Source options");
  //source_options.add_options()
  //    ("disable-system-sources,D", "Don't read the system sources.")
  //    ("sources,S", po::value< vector<string> >(), "Read from additional sources")
  //    ;
  
  // Declare an options description instance which will include
  // all the options
  po::options_description all_options("Allowed options");
  all_options.add(general_options).add(check_options);

  // Declare an options description instance which will be shown
  // to the user
  po::options_description visible_options("Allowed options");
  visible_options.add(general_options).add(check_options);

  po::variables_map vm;
  //po::store(po::parse_command_line(argc, argv, visible_options), vm);
  po::store(po::command_line_parser(argc, argv).options(visible_options).positional(pos_options).run(), vm);
  po::notify(vm);
 
  if (vm.count("help")) {
    cout << visible_options << "\n";
    return 1;
  }
  
  std::string previous_token;
  if (vm.count("previous-token"))
  {
    previous_token = vm["previous-token"].as< string >();
  }
  
  int previous_code = -1;
  if (vm.count("previous-result"))
  {
    previous_code = vm["previous-result"].as< int >();
  }
  
  MIL << argv[0] << " started with arguments " << previous_token << " " << previous_code << std::endl;
   
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
  
  
  render_xml(God->pool());
  
  //MIL << "Patches " << security_count << " " << count << std::endl;
  
  if ( security_count > 0 )
    return 2;
  
  if ( count > 0 )
    return 1;
  
  return 0;
}


