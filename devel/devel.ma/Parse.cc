//http://www.boost.org/libs/libraries.htm
#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <iterator>

#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>
#include <zypp/base/String.h>
#include <zypp/base/PtrTypes.h>

#include <zypp/parser/tagfile/Parser.h>
#include <zypp/Package.h>
#include <zypp/CapSet.h>
#include <zypp/CapFactory.h>
#include <zypp/detail/PackageImpl.h>

#include <zypp/NVRA.h>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace tagfile
    { /////////////////////////////////////////////////////////////////


      struct PackagesParser : public Parser
      {
        std::list<Package::Ptr> result;

        shared_ptr<detail::PackageImpl> pkgImpl;

        Package::Ptr pkg;
        Dependencies deps;

        void collectPkg()
        {
          if ( pkg )
            {
              pkg->setDeps( deps );
              result.push_back( pkg );
              // reset
              deps = Dependencies();
            }
        }

        void collectDeps( const std::list<std::string> & depstr_r,
                          std::insert_iterator<CapSet> result_r )
        {
          Capability c( CapFactory().parse( ResTraits<Package>::kind, "" ) );
          INT << c << endl;
          *result_r++ = c;
        }

        CapSet collectDeps( const std::list<std::string> & depstr_r )
        {
          return CapSet();
        }

        /* Overload to consume SingleTag data. */
        virtual void consume( const STag & stag_r )
        {
          if ( stag_r.stag.isPlain( "Pkg" ) )
            {
              collectPkg();

              std::vector<std::string> words;
              str::split( stag_r.value, std::back_inserter(words) );

              if ( str::split( stag_r.value, std::back_inserter(words) ) != 4 )
                ZYPP_THROW( ParseException( "Pkg" ) );

              pkg = detail::makeResolvableAndImpl( words[0],
                                                   Edition(words[1],words[2]),
                                                   Arch(words[4]),
                                                   pkgImpl );
            }
          //MIL << stag_r << endl;
        }
        /* Overload to consume MulitTag data. */
        virtual void consume( const MTag & mtag_r )
        {
          if ( mtag_r.stag.isPlain( "Prv" ) )
            {
              CapSet cs;
              collectDeps( mtag_r.value, std::inserter(cs,cs.end()) );
              INT << cs.size() << endl;
              deps.setProvides( cs );
            }
          else if ( mtag_r.stag.isPlain( "Prq" ) )
            {
              deps.setPrerequires( collectDeps( mtag_r.value ) );
            }
          else if ( mtag_r.stag.isPlain( "Req" ) )
            {
              deps.setRequires( collectDeps( mtag_r.value ) );
            }
          else if ( mtag_r.stag.isPlain( "Con" ) )
            {
              deps.setConflicts( collectDeps( mtag_r.value ) );
            }
          else if ( mtag_r.stag.isPlain( "Obs" ) )
            {
              deps.setObsoletes( collectDeps( mtag_r.value ) );
            }
        }
      };

      /////////////////////////////////////////////////////////////////
    } // namespace tagfile
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
//  Types
//
////////////////////////////////////////////////////////////////////////////

using std::endl;
using namespace zypp;
using namespace zypp::parser::tagfile;

////////////////////////////////////////////////////////////////////////////
//
//  Just for the stats
//
////////////////////////////////////////////////////////////////////////////
struct Measure
{
  time_t _begin;
  Measure()
  : _begin( time(NULL) )
  {
    USR << "START MEASURE..." << endl;
  }
  ~Measure()
  {
    USR << "DURATION: " << (time(NULL)-_begin) << " sec." << endl;
  }
};
////////////////////////////////////////////////////////////////////////////


using namespace std;
////////////////////////////////////////////////////////////////////////////
//
//  Main
//
////////////////////////////////////////////////////////////////////////////
int main( int argc, char* argv[] )
{
  INT << "===[START]==========================================" << endl;
  string infile( "p" );
  if (argc >= 2 )
    infile = argv[1];

  PackagesParser p;
  p.parse( infile );

  SEC << p.result.size() << endl;
  MIL << *p.result.front() << endl;
  MIL << p.result.front()->deps() << endl;

  INT << "===[END]============================================" << endl;
  return 0;
}
