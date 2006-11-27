// test for SUSE Source
//

#include <string.h>

#include <iostream>
#include <fstream>
#include <map>
#include "zypp/ResObject.h"
#include "zypp/Package.h"
#include "zypp/Selection.h"
#include "zypp/Pattern.h"
#include "zypp/Patch.h"
#include "zypp/Product.h"
#include "zypp/Script.h"
#include "zypp/Message.h"
#include "zypp/Atom.h"
#include "zypp/Dependencies.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/media/MediaManager.h"
#include "KeyRingCallbacks.h"

using namespace std;
using namespace zypp;

typedef std::map<std::string, unsigned int> CapabilityRank;
CapabilityRank ranking;

void print_rank()
{
  int counter_entries = 0;
  int counter_caps = 0;
  CapabilityRank::const_iterator it;
  for ( it = ranking.begin(); it != ranking.end(); ++it )
  {
    std::cout << "\"" << (*it).first << "\"" << "," << (*it).second << std::endl;
    counter_caps += (*it).second;
    ++counter_entries;
  }
  
  std::cout << "unique: " << counter_entries << " total: " << counter_caps << std::endl;
}

void rank( const CapSet &obj )
{
  for ( CapSet::const_iterator it = obj.begin(); it != obj.end(); ++it )
  {
    std::string capa((*it).asString());
    CapabilityRank::const_iterator itr;
    if ( ( itr = ranking.find( capa )) != ranking.end() )
    {
      ++ ranking[(*it).asString()];
    }
    else
    {
      ranking[(*it).asString()] = 1;
    }
  }
}

void rank( const Dependencies &obj )
{
    rank(obj[Dep::PROVIDES]);
    rank(obj[Dep::PREREQUIRES]);
    rank(obj[Dep::REQUIRES]);
    rank(obj[Dep::CONFLICTS]);
    rank(obj[Dep::OBSOLETES]);
    rank(obj[Dep::RECOMMENDS]);
    rank(obj[Dep::SUGGESTS]);
    rank(obj[Dep::SUPPLEMENTS]);
    rank(obj[Dep::ENHANCES]);
    rank(obj[Dep::FRESHENS]);
}


/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  if (argc < 2)
  {
    cerr << "Usage: suseparse [--arch x] <susedir> [all|reallyall]" << endl;
    exit (1);
  }
  //zypp::base::LogControl::instance().logfile( "-" );

  INT << "===[START]==========================================" << endl;

  ZYpp::Ptr God;
  try
  {
    God = zypp::getZYpp( );
  }
  catch ( const Exception & excpt_r )
  {
    ZYPP_CAUGHT( excpt_r );
    cerr << "Can't aquire ZYPP lock" << endl;
    return 1;
  }

  KeyRingCallbacks keyring_callbacks;
  DigestCallbacks digest_callbacks;

  int argpos = 1;

  if (strcmp( argv[argpos], "--arch") == 0)
  {
    ++argpos;
    God->setArchitecture( Arch( argv[argpos] ) );
    ++argpos;
  }
  Pathname p;
  Url url;
  try
  {
    url = ( argv[argpos++] );
  }
  catch ( const Exception & excpt_r )
  {
    cerr << "Invalid url " << argv[--argpos] << endl;
    ZYPP_CAUGHT( excpt_r );
    return 1;
  }

  string alias("suseparse");
  Locale lang( "en" );

  Pathname cache_dir("");
  Source_Ref src;
  try
  {
    src = SourceFactory().createFrom(url, p, alias, cache_dir);
  }
  catch ( const Exception & excpt_r )
  {
    cerr << "Can't access repository" << endl;
    ZYPP_CAUGHT( excpt_r );
    return 1;
  }

  ResStore store;
  try
  {
    store = src.resolvables();
  }
  catch ( const Exception & excpt_r )
  {
    cerr << "Can't access store" << endl;
    ZYPP_CAUGHT( excpt_r );
    return 1;
  }


  INT << "Found " << store.size() << " resolvables" << endl;
  int pkgcount = 0;
  int selcount = 0;
  int patcount = 0;
  int pchcount = 0;
  int prdcount = 0;
  int scrcount = 0;
  int msgcount = 0;
  int atmcount = 0;
  for (ResStore::iterator it = store.begin(); it != store.end(); ++it)
  {
    Package::constPtr pkg = asKind<Package>(*it);
    Selection::constPtr sel = asKind<Selection>(*it);
    Pattern::constPtr pat = asKind<Pattern>(*it);
    Patch::constPtr pch = asKind<Patch>(*it);
    Product::constPtr prd = asKind<Product>(*it);
    Script::constPtr scr = asKind<Script>(*it);
    Message::constPtr msg = asKind<Message>(*it);
    Atom::constPtr atm = asKind<Atom>(*it);
    if (pkg != NULL) ++pkgcount;
    if (sel != NULL) ++selcount;
    if (pat != NULL) ++patcount;
    if (pch != NULL) ++pchcount;
    if (prd != NULL) ++prdcount;
    if (scr != NULL) ++scrcount;
    if (msg != NULL) ++msgcount;
    if (atm != NULL) ++atmcount;
  }
  std::cout << "Found " << store.size() << " resolvables" << endl;
  std::cout << "\t" << pkgcount << " packages" << endl;
  std::cout << "\t" << selcount << " selections" << endl;
  std::cout << "\t" << patcount << " patterns" << endl;
  std::cout << "\t" << pchcount << " patches" << endl;
  std::cout << "\t" << scrcount << " scripts" << endl;
  std::cout << "\t" << msgcount << " messages" << endl;
  std::cout << "\t" << atmcount << " atoms" << endl;
  std::cout << "\t" << prdcount << " products" << endl;

  std::cout << "Available locales: ";
  ZYpp::LocaleSet locales = God->getAvailableLocales();
  for (ZYpp::LocaleSet::const_iterator it = locales.begin(); it != locales.end(); ++it)
  {
    if (it != locales.begin()) std::cout << ", ";
    //std::cout << it->code();
  }
  std::cout << endl;

  if (argpos < argc)
  {
    int count = 0;
    for (ResStore::iterator it = store.begin(); it != store.end(); ++it)
    {
      ResObject::constPtr robj = *it;
      //std::cout << ++count << *robj << endl;
      if (strcmp(argv[argpos], "reallyall") == 0)
      {
        const Dependencies & deps = robj->deps();
        
        rank(deps);
        
        //std::cout << deps << endl;
        
      }
    }
    print_rank();
  }
  else
  {
    if (store.size() > 0)
    {
      ResStore::iterator it = store.begin();
      MIL << "First " << (*it)->name() << ":" << (*it)->summary() << endl << (*it)->description() << endl;
    }
  }
  INT << "===[END]============================================" << endl;
  return 0;
}
