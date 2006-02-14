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

#include <boost/regex.hpp>

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


      struct PackageDiskUsageParser : public parser::tagfile::TagFileParser
      {
        PkgDiskUsage result;
        NVRAD _current_nvrad;
        bool _pkg_pending;
        boost::regex sizeEntryRX;

        virtual void beginParse()
        {
          _pkg_pending = false;
          sizeEntryRX = boost::regex("^(.*/)[[:space:]]([[:digit:]]+)[[:space:]]([[:digit:]]+)[[:space:]]([[:digit:]]+)[[:space:]]([[:digit:]]+)$");
        }          

        /* Consume SingleTag data. */
        virtual void consume( const SingleTag & stag_r )
        {
          if ( stag_r.name == "Pkg" )
          {
            std::vector<std::string> words;
            if ( str::split( stag_r.value, std::back_inserter(words) ) != 4 )
              ZYPP_THROW( ParseException( "Pkg" ) );

            _pkg_pending = true;
            _current_nvrad = NVRAD( words[0], Edition(words[1],words[2]), Arch(words[3]) );
          }
          else
          {
            //ZYPP_THROW( ParseException( "Loc" ) );
            ERR << "warning found unexpected tag " << stag_r.name << std::endl;
          }
        }

        /* Consume MulitTag data. */
        virtual void consume( const MultiTag & mtag_r )
        {
          if ( ! _pkg_pending )
            return;
          
          if ( mtag_r.name == "Dir" )
          {
            for (std::list<std::string>::const_iterator it = mtag_r.values.begin(); it != mtag_r.values.end(); ++it )
            {
              boost::smatch what;
              if(boost::regex_match(*it, what, sizeEntryRX, boost::match_extra))
              {
                //zypp::parser::tagfile::dumpRegexpResults(what);
                // build the disk usage info
                DiskUsage usage;
                usage.add( what[1], str::strtonum<unsigned int>(what[2]) + str::strtonum<unsigned int>(what[3]), str::strtonum<unsigned int>(what[4]) + str::strtonum<unsigned  int>(what[5]));
                result[_current_nvrad] = usage;
              }
              else
              {
                ERR << "Error parsing package size entry" << "[" << *it << "]" << std::endl;
                ZYPP_THROW( ParseException( "Dir" ) );
              }
            }
            _pkg_pending = false;
          }
        }

        virtual void endParse()
        {}
      };

     
      struct PackagesParser : public parser::tagfile::TagFileParser
      {
        PkgContent _result;

	Source_Ref _source;
	SuseTagsImpl::Ptr _sourceImpl;

        PkgImplPtr _pkgImpl;
        NVRAD _nvrad;

	PackagesParser(Source_Ref source, SuseTagsImpl::Ptr sourceimpl)
	    : _source( source )
	    , _sourceImpl( sourceimpl )
	{ }

        bool pkgPending() const
        { return _pkgImpl; }

        PkgContent result() const
        {
	  return _result;
        }

        void collectPkg( const shared_ptr<source::susetags::SuseTagsPackageImpl> & nextPkg_r
                         = shared_ptr<source::susetags::SuseTagsPackageImpl>() )
        {
          if ( pkgPending() )
            {
              _result.insert(PkgContent::value_type( _nvrad, _pkgImpl ) );
            }
          _pkgImpl = nextPkg_r;
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
#warning FIXME, do proper filtering from content:ARCH line
	    if (words[3] == "src" || words[3] == "nosrc")
		_pkgImpl.reset();

            newPkg();
            _nvrad = NVRAD( words[0], Edition(words[1],words[2]), Arch(words[3]) );
          }
          if ( stag_r.name == "Grp" )
          {
            _pkgImpl->_group = stag_r.value;
          }
          if ( stag_r.name == "Lic" )
          {
            _pkgImpl->_license = stag_r.value;
          }
          if ( stag_r.name == "Tim" )
          {
            _pkgImpl->_buildtime = Date(str::strtonum<time_t>(stag_r.value));
          }
          if ( stag_r.name == "Siz" )
          {
            std::vector<std::string> words;
            if ( str::split( stag_r.value, std::back_inserter(words) ) != 2 )
              ZYPP_THROW( ParseException( "Siz" ) );

            _pkgImpl->_sourcesize = str::strtonum<unsigned long>(words[0]);
            _pkgImpl->_archivesize = str::strtonum<unsigned long>(words[1]);
          }
          if ( stag_r.name == "Loc" )
          {
            std::vector<std::string> words;
            unsigned int howmany = str::split( stag_r.value, std::back_inserter(words) );
            _pkgImpl->_media_number = 1;
            if ( howmany >= 2 )
            {
              _pkgImpl->_media_number = str::strtonum<unsigned int>(words[0]);
              _pkgImpl->_location = _sourceImpl->sourceDir(_nvrad) + words[1];
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
              collectDeps( mtag_r.values, _nvrad[Dep::PROVIDES] );
            }
          else if ( mtag_r.name == "Prq" )
            {
              collectDeps( mtag_r.values, _nvrad[Dep::PREREQUIRES] );
            }
          else if ( mtag_r.name == "Req" )
            {
              collectDeps( mtag_r.values, _nvrad[Dep::REQUIRES] );
            }
          else if ( mtag_r.name == "Con" )
            {
              collectDeps( mtag_r.values, _nvrad[Dep::CONFLICTS] );
            }
          else if ( mtag_r.name == "Obs" )
            {
              collectDeps( mtag_r.values, _nvrad[Dep::OBSOLETES] );
            }
          else if ( mtag_r.name == "Key" )
            {
               _pkgImpl->_keywords = mtag_r.values;
            }
          else if ( mtag_r.name == "Aut" )
            {
              // MultiTag is a Set but author is a list
              _pkgImpl->_authors = mtag_r.values;
            }
        }

        virtual void endParse()
        { collectPkg(); }
      };

      ////////////////////////////////////////////////////////////////////////////

      PkgContent parsePackages( Source_Ref source_r, SuseTagsImpl::Ptr sourceImpl_r, const Pathname & file_r )
      {
        PackagesParser p( source_r, sourceImpl_r );
        p.parse( file_r );
        return p.result();
      }

      PkgDiskUsage parsePackagesDiskUsage( const Pathname & file_r )
      {
        PackageDiskUsageParser duParser;
        duParser.parse(file_r);
        return duParser.result;
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
