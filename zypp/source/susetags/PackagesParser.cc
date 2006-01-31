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
#include "zypp/source/susetags/SuseTagsPackageImpl.h"
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

        shared_ptr<source::susetags::SuseTagsPackageImpl> pkgImpl;
        NVRAD nvrad;

        bool pkgPending() const
        { return pkgImpl; }

        void collectPkg( const shared_ptr<source::susetags::SuseTagsPackageImpl> & nextPkg_r
                         = shared_ptr<source::susetags::SuseTagsPackageImpl>() )
        {
          if ( pkgPending() )
            {
              result.push_back( detail::makeResolvableFromImpl( nvrad, pkgImpl ) );
            }
          pkgImpl = nextPkg_r;
        }

        void newPkg()
        {
          collectPkg( shared_ptr<source::susetags::SuseTagsPackageImpl>(new source::susetags::SuseTagsPackageImpl) );
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
          if ( stag_r.name == "Grp" )
          {
            pkgImpl->_group = stag_r.value;
          }
          if ( stag_r.name == "Lic" )
          {
            pkgImpl->_license = stag_r.value;
          }
          if ( stag_r.name == "Tim" )
          {
            pkgImpl->_buildtime = Date(str::strtonum<time_t>(stag_r.value));
          }
          if ( stag_r.name == "Siz" )
          {
            std::vector<std::string> words;
            if ( str::split( stag_r.value, std::back_inserter(words) ) != 2 )
              ZYPP_THROW( ParseException( "Siz" ) );

            pkgImpl->_sourcesize = str::strtonum<unsigned long>(words[0]);
            pkgImpl->_archivesize = str::strtonum<unsigned long>(words[1]);
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
          else if ( mtag_r.name == "Key" )
            {
               pkgImpl->_keywords = std::list<std::string>(mtag_r.values.begin(), mtag_r.values.end());
            }
          else if ( mtag_r.name == "Aut" )
            {
              // MultiTag is a Set but author is a list
              pkgImpl->_authors = std::list<std::string>(mtag_r.values.begin(), mtag_r.values.end());
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
