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
        NVRAD nvrad;

        bool pkgPending() const
        { return pkgImpl; }

        void collectPkg( const shared_ptr<detail::PackageImpl> & nextPkg_r
                         = shared_ptr<detail::PackageImpl>() )
        {
          if ( pkgPending() )
            {
              result.push_back( detail::makeResolvableFromImpl( nvrad, pkgImpl ) );
            }
          pkgImpl = nextPkg_r;
        }

        void newPkg()
        {
          collectPkg( shared_ptr<detail::PackageImpl>(new detail::PackageImpl) );
        }

        void collectDeps( const std::list<std::string> & depstr_r, CapSet & capset )
        {
          for ( std::list<std::string>::const_iterator it = depstr_r.begin();
                it != depstr_r.end(); ++it )
            {
              capset.insert( CapFactory().parse( ResTraits<Package>::kind, *it ) );
            }
        }

        /* Consume SingleTag data. */
        virtual void consume( const STag & stag_r )
        {
          if ( stag_r.stag.isPlain( "Pkg" ) )
            {
              std::vector<std::string> words;
              str::split( stag_r.value, std::back_inserter(words) );

              if ( str::split( stag_r.value, std::back_inserter(words) ) != 4 )
                ZYPP_THROW( ParseException( "Pkg" ) );

              newPkg();
              nvrad = NVRAD( words[0], Edition(words[1],words[2]), Arch(words[4]) );
            }
        }

        /* Consume MulitTag data. */
        virtual void consume( const MTag & mtag_r )
        {
          if ( ! pkgPending() )
            return;

          if ( mtag_r.stag.isPlain( "Prv" ) )
            {
              collectDeps( mtag_r.value, nvrad.provides );
            }
          else if ( mtag_r.stag.isPlain( "Prq" ) )
            {
              collectDeps( mtag_r.value, nvrad.prerequires );
            }
          else if ( mtag_r.stag.isPlain( "Req" ) )
            {
              collectDeps( mtag_r.value, nvrad.requires );
            }
          else if ( mtag_r.stag.isPlain( "Con" ) )
            {
              collectDeps( mtag_r.value, nvrad.conflicts );
            }
          else if ( mtag_r.stag.isPlain( "Obs" ) )
            {
              collectDeps( mtag_r.value, nvrad.obsoletes );
            }
        }

        virtual void parseEnd()
        { collectPkg(); }
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
