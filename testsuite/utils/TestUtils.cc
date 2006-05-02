
#include <iostream>
#include "zypp/Dependencies.h"
#include "zypp/Capability.h"
#include "zypp/ResObject.h"
#include "zypp/Patch.h"
#include "zypp/ResStore.h"
#include "testsuite/utils/TestUtils.h"

using namespace std;

namespace zypp
{
  namespace testsuite
  {
    namespace utils
    {
      static void dumpCapSet( const CapSet &caps, const std::string &deptype)
      {
        cout << "   " << "<" << deptype << ">" << std::endl;
        CapSet::iterator it = caps.begin();
        for ( ; it != caps.end(); ++it)
        {
          cout << "    <capability kind=\"" << (*it).kind() << "\" refers=\"" << (*it).refers() << "\">" << (*it).asString() << "</capability>" << std::endl;
        }
        cout << "   " << "</" << deptype << ">" << std::endl;
      }

      static void dumpDeps( const Dependencies &deps )
      {
        dumpCapSet( deps[Dep::PROVIDES], "provides" );
        dumpCapSet( deps[Dep::PREREQUIRES], "prerequires" );
        dumpCapSet( deps[Dep::CONFLICTS], "conflicts" );
        dumpCapSet( deps[Dep::OBSOLETES], "obsoletes" );
        dumpCapSet( deps[Dep::FRESHENS], "freshens" );
        dumpCapSet( deps[Dep::REQUIRES], "requires" );
        dumpCapSet( deps[Dep::RECOMMENDS], "recommends" );
        dumpCapSet( deps[Dep::ENHANCES], "enhances" );
        dumpCapSet( deps[Dep::SUPPLEMENTS], "supplements" );
        dumpCapSet( deps[Dep::SUGGESTS], "suggests" );
      }

      static std::string xml_escape( const std::string &s )
      {
        return s;
      }
      
      static void dumpResObject( const ResObject::Ptr r, bool descr, bool deps )
      {
        std::string resolvable_line = "[" + r->kind().asString() + "]" + r->name() + " " + r->edition().asString() + " " + r->arch().asString();
          cout << " <resolvable kind=\"" << r->kind() << "\">" << std::endl;
          cout <<  "  <name>" << r->name() << "</name>" << std::endl;
          cout <<  "  <edition>" << r->edition() << "</edition>" << std::endl;
          cout <<  "  <arch>" << r->arch() << "</arch>" << std::endl;
          if ( descr )
          {
            cout <<  "  <summary>" << r->summary() << "</summary>" << std::endl;
            cout <<  "  <description>" << r->description() << "</description>" << std::endl;
          }
          
          if ( deps )
          {
            cout <<  "  <dependencies>" << std::endl;
            dumpDeps(r->deps());
            cout <<  "  </dependencies>" << std::endl;
          }
          cout << "  </resolvable>" << std::endl;
    //std::cout << (**it).deps() << endl;
      }
      
      void dump( const std::list<ResObject::Ptr> &list, bool descr, bool deps )
      {
        std::list<std::string> resolvables;
        cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        cout << "<resolvable-list xmlns=\"http://www.novell.com/zypp/testcases/resolvable-list\">" << std::endl;
        for (std::list<ResObject::Ptr>::const_iterator it = list.begin(); it != list.end(); it++)
        {
          dumpResObject(*it, descr, deps);
        }
        cout << "</resolvable-list>" << std::endl;
      }
      
      void dump( const ResStore &store, bool descr, bool deps )
      {
        std::list<std::string> resolvables;
        cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        cout << "<resolvable-list xmlns=\"http://www.novell.com/zypp/testcases/resolvable-list\">" << std::endl;
        for (ResStore::const_iterator it = store.begin(); it != store.end(); it++)
        {
          dumpResObject(*it, descr, deps);
        }
        cout << "</resolvable-list>" << std::endl;
  //std::sort( resolvables.begin(), resolvables.end(), CustomSort );
  //for (std::list<std::string>::const_iterator it = resolvables.begin(); it != resolvables.end(); ++it)
  //{
 //   cout << *it << std::endl;
  //}
      }
  
      
      
    }  
  }
}

