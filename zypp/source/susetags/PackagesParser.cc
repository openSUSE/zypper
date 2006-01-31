/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/PackagesParser.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/source/susetags/PackagesParser.h"
#include "zypp/parser/tagfile/TagFileParser.h"
#include "zypp/Package.h"
#include "zypp/detail/PackageImpl.h"
#include "zypp/CapFactory.h"
#include "zypp/CapSet.h"


using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      using namespace parser::tagfile;

      struct PackagesParser : public zypp::parser::tagfile::TagFileParser
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

        void collectDeps( const std::set<std::string> & depstr_r, CapSet & capset )
        {
          for ( std::set<std::string>::const_iterator it = depstr_r.begin();
                it != depstr_r.end(); ++it )
            {
              capset.insert( CapFactory().parse( ResTraits<Package>::kind, *it ) );
            }
        }

        /* Consume SingleTag data. */
        virtual void consume( const SingleTag & stag_r )
        {
          if ( stag_r.name == "Pkg" )
            {
              std::vector<std::string> words;
              str::split( stag_r.value, std::back_inserter(words) );

              if ( str::split( stag_r.value, std::back_inserter(words) ) != 4 )
                ZYPP_THROW( ParseException( "Pkg" ) );

              newPkg();
              nvrad = NVRAD( words[0], Edition(words[1],words[2]), Arch(words[3]) );
            }
        }

        /* Consume MulitTag data. */
        virtual void consume( const MultiTag & mtag_r )
        {
          if ( ! pkgPending() )
            return;

          if ( mtag_r.name == "Prv" )
            {
              collectDeps( mtag_r.values, nvrad[Dep::PROVIDES] );
            }
          else if ( mtag_r.name == "Prq" )
            {
              collectDeps( mtag_r.values, nvrad[Dep::PREREQUIRES] );
            }
          else if ( mtag_r.name == "Req" )
            {
              collectDeps( mtag_r.values, nvrad[Dep::REQUIRES] );
            }
          else if ( mtag_r.name == "Con" )
            {
              collectDeps( mtag_r.values, nvrad[Dep::CONFLICTS] );
            }
          else if ( mtag_r.name == "Obs" )
            {
              collectDeps( mtag_r.values, nvrad[Dep::OBSOLETES] );
            }
        }

        virtual void parseEnd()
        { collectPkg(); }
      };

      ////////////////////////////////////////////////////////////////////////////

      std::list<Package::Ptr> parsePackages( const Pathname & file_r )
      {
        PackagesParser p;
        p.parse( file_r );
        return p.result;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
