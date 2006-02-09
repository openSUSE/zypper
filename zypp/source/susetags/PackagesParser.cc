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

      struct PackagesParser : public parser::tagfile::TagFileParser
      {
        PkgContent result;

	Source_Ref _source;
	SuseTagsImpl::Ptr _sourceImpl;

        PkgImplPtr pkgImpl;
        NVRAD nvrad;

        bool pkgPending() const
        { return pkgImpl; }

        void collectPkg( const shared_ptr<source::susetags::SuseTagsPackageImpl> & nextPkg_r
                         = shared_ptr<source::susetags::SuseTagsPackageImpl>() )
        {
          if ( pkgPending() )
            {
              result.insert(PkgContent::value_type( nvrad, pkgImpl ) );
            }
          pkgImpl = nextPkg_r;
        }

        void newPkg()
        {
          collectPkg( shared_ptr<source::susetags::SuseTagsPackageImpl>(new source::susetags::SuseTagsPackageImpl(_source)) );
        }

        void collectDeps( const std::list<std::string> & depstr_r, CapSet & capset )
        {
          for ( std::list<std::string>::const_iterator it = depstr_r.begin();
                it != depstr_r.end(); ++it )
            {
	      try {
                capset.insert( CapFactory().parse( ResTraits<Package>::kind, *it ) );
	      }
	      catch (Exception & excpt_r) {
		ZYPP_CAUGHT(excpt_r);
	      }
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
          if ( stag_r.name == "Loc" )
          {
            std::vector<std::string> words;
            unsigned int howmany = str::split( stag_r.value, std::back_inserter(words) );
            pkgImpl->_media_number = 1;
            if ( howmany >= 2 )
            {
              pkgImpl->_media_number = str::strtonum<unsigned int>(words[0]);
              pkgImpl->_location = _sourceImpl->sourceDir(nvrad) + words[1];
            }
            else
            {
                ZYPP_THROW( ParseException( "Loc" ) );
            }
            // ignore path
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
               pkgImpl->_keywords = mtag_r.values;
            }
          else if ( mtag_r.name == "Aut" )
            {
              // MultiTag is a Set but author is a list
              pkgImpl->_authors = mtag_r.values;
            }
        }

        virtual void parseEnd()
        { collectPkg(); }
      };

      ////////////////////////////////////////////////////////////////////////////

      PkgContent parsePackages( Source_Ref source_r, SuseTagsImpl::Ptr sourceImpl_r, const Pathname & file_r )
      {
        PackagesParser p;
	p._source = source_r;
	p._sourceImpl = sourceImpl_r;
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
