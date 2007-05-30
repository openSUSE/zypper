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
#include <sstream>
#include <iostream>

#include <boost/regex.hpp>

#include "zypp/base/Logger.h"

#include "zypp/parser/ParseException.h"

#include "zypp/source/susetags/PackagesParser.h"
#include "zypp/parser/tagfile/TagFileParser.h"
#include "zypp/Arch.h"
#include "zypp/Package.h"
#include "zypp/CapFactory.h"
#include "zypp/CapSet.h"

#include "zypp/ZYppFactory.h"

using namespace std;
using namespace zypp::parser;
using namespace zypp::parser::tagfile;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace susetags
{ /////////////////////////////////////////////////////////////////

class PackageDiskUsageParser : public parser::tagfile::TagFileParser
{
public:
  PkgDiskUsage result;
  NVRAD _current_nvrad;
  bool _pkg_pending;
  boost::regex sizeEntryRX;

  PackageDiskUsageParser( parser::ParserProgress::Ptr progress ) : parser::tagfile::TagFileParser(progress)
  {}

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
        ZYPP_THROW( ParseException( "packages.DU - Expected [name version release arch], got [" + stag_r.value +"]") );

      _pkg_pending = true;
      _current_nvrad = NVRAD( words[0], Edition(words[1],words[2]), Arch(words[3]) );
    }
    else if ( stag_r.name == "Ver" )
    {
      if (stag_r.value != "2.0")
        WAR << "packages.DU " << stag_r.name << "=" << stag_r.value << ", should be 2.0" << endl;
    }
    else
    {
      //ZYPP_THROW( ParseException( "Unknown tag" ) );
      ERR << "packages.DU - ERROR! found unexpected tag " << stag_r.name << std::endl;
    }
  }

  /* Consume MulitTag data. */
  virtual void consume( const MultiTag & mtag_r )
  {
    if ( ! _pkg_pending )
      return;

    if ( mtag_r.name == "Dir" )
    {
      DiskUsage usage;
      for (std::list<std::string>::const_iterator it = mtag_r.values.begin(); it != mtag_r.values.end(); ++it )
      {
        boost::smatch what;
        if (boost::regex_match(*it, what, sizeEntryRX, boost::match_extra))
        {
          //zypp::parser::tagfile::dumpRegexpResults(what);
          // build the disk usage info
          usage.add( what[1], str::strtonum<unsigned int>(what[2]) + str::strtonum<unsigned int>(what[3]), str::strtonum<unsigned int>(what[4]) + str::strtonum<unsigned  int>(what[5]));
        }
        else
        {
          ZYPP_THROW( ParseException( std::string("Error parsing package size entry") + "[" + *it + "]" ) );
        }
      }
      result[_current_nvrad] = usage;
      _pkg_pending = false;
    }
  }

  virtual void endParse()
  {
  }
};


struct PackagesParser : public parser::tagfile::TagFileParser
{
  PkgContent _result;

  Source_Ref _source;
  SuseTagsImpl::Ptr _sourceImpl;

  bool _isPendingPkg;
  PkgImplPtr _pkgImpl;
  SrcPkgImplPtr _srcPkgImpl;
  NVRAD _nvrad;
  bool _isShared;

  Arch _system_arch;

  PackagesParser( parser::ParserProgress::Ptr progress, Source_Ref source, SuseTagsImpl::Ptr sourceimpl)
      : parser::tagfile::TagFileParser(progress)
      , _source( source )
      , _sourceImpl( sourceimpl )
      , _isPendingPkg( false )
      , _isShared( false )

  {
    ZYpp::Ptr z = getZYpp();
    _system_arch = z->architecture();
  }

  PkgContent result() const
  {
    return _result;
  }

  void collectPkg()
  {
    if ( _isPendingPkg )
    {
      _pkgImpl->_sourceImpl = _sourceImpl;
      // if the package does not depend on other package for its data
      // then use its own nvrad as an index
      if ( !_isShared )
      {
        _pkgImpl->_data_index =  _pkgImpl->_nvra;
        _sourceImpl->_is_shared[ _pkgImpl->_nvra] = false;
      }
      else
      {
        _sourceImpl->_is_shared[ _pkgImpl->_nvra] = true;
      }

      if (_srcPkgImpl == NULL					// only if its not a src/nosrc
          && _nvrad.arch.compatibleWith( _system_arch ) )
      {
        _result.insert( PkgContent::value_type( _nvrad, _pkgImpl ) );
      }
      _isPendingPkg = false;
    }
  }

  void collectDeps( const std::list<std::string> & depstr_r, CapSet & capset )
  {
    for ( std::list<std::string>::const_iterator it = depstr_r.begin();
          it != depstr_r.end(); ++it )
    {
      if ( (*it).empty() )
      {
        stringstream ss;
        WAR << "[" << _source.alias() << "] at URL:[" << _source.url().asString() << "]. Emtpy capability on " << _file_r << " line " << _line_number << ". Ignoring." << endl;
      }

      try
      {
        capset.insert( CapFactory().parse( ResTraits<Package>::kind, *it ) );
      }
      catch (Exception & excpt_r)
      {
        stringstream ss;
        ss << "Bad source [" << _source.alias() << "] at URL:[" << _source.url().asString() << "]. Can't parse capability: [" << *it << "] on " << _file_r << " line " << _line_number;
        ZYPP_THROW( ParseException( ss.str() ) );
      }
    }
  }

  /* Consume SingleTag data. */
  virtual void consume( const SingleTag & stag_r )
  {
    if ( stag_r.name == "Pkg" )
    {
      // reset
      // this means this is either the first package, or we just finished parsing a package and a new one is starting
      // collect previous pending package if needed
      collectPkg();
      // reset
      _isShared = false;
      std::vector<std::string> words;
      str::split( stag_r.value, std::back_inserter(words) );

      if ( str::split( stag_r.value, std::back_inserter(words) ) != 4 )
        ZYPP_THROW( ParseException("Bad source ["+ _source.alias() +"] at URL:[" + _source.url().asString() + "]. error, we expected NVRA here, got: " + stag_r.value ) );

      std::string arch = words[3];
#warning read comment in file
      // warning: according to autobuild, this is the wrong check
      //  a 'src/norsrc' packages is determined by _not_ having a "=Src:" tag in packages
      // However, the 'arch' string we're checking here must be remembered since
      // it determines the directory below the <DATADIR> where the real package
      // can be found.
      if (arch == "src"
          || arch == "nosrc")
      {
        arch = "noarch";
        _srcPkgImpl = SrcPkgImplPtr( new source::susetags::SuseTagsSrcPackageImpl( _source ) );
      }
      else
        _srcPkgImpl = NULL;

      _pkgImpl = PkgImplPtr( new source::susetags::SuseTagsPackageImpl( _source ) );
      _nvrad = NVRAD( words[0], Edition( words[1], words[2] ), Arch( arch ) );
      _pkgImpl->_nvra = NVRA( words[0], Edition( words[1], words[2] ), Arch( arch ) );

      _isPendingPkg = true;

    }
    else if ( stag_r.name == "Cks" )
    {
      std::vector<std::string> words;
      if ( str::split( stag_r.value, std::back_inserter(words) ) != 2 )
        ZYPP_THROW( ParseException("Bad source ["+ _source.alias() +"] at URL:[" + _source.url().asString() + "]. Key: [" + stag_r.name + "] - Expected [type checksum], got [" + stag_r.value +"]"));

      _pkgImpl->_checksum = CheckSum(words[0], words[1]);
    }
    else if ( stag_r.name == "Shr" )
    {
      // shared description tags
      std::vector<std::string> words;
      str::split( stag_r.value, std::back_inserter(words) );

      if ( str::split( stag_r.value, std::back_inserter(words) ) != 4 )
        ZYPP_THROW( ParseException("Bad source ["+ _source.alias() +"] at URL:[" + _source.url().asString() + "]. Shr tag is wrong, expected NVRA, got: " + stag_r.value ) );

      std::string arch = words[3];
      NVRA shared_desc( words[0], Edition( words[1], words[2] ), Arch(arch));
      XXX << "package " << _nvrad << " shares data with " << shared_desc << std::endl;
      _isShared = true;
      _pkgImpl->_data_index = shared_desc;
      // mark the refering package as a package providing data to others
      // because we cant filter those by architecture to save memory
      // or we run in missing package descriptions for x86_64 packages
      // which depends on a ppc package for its data.
      _sourceImpl->_provides_shared_data[ _pkgImpl->_data_index] = true;
    }
    if ( stag_r.name == "Grp" )
    {
      _pkgImpl->_group = stag_r.value;
    }
    if ( stag_r.name == "Vnd" )
    {
      _pkgImpl->_vendor = stag_r.value;
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
        ZYPP_THROW( ParseException("Bad source ["+ _source.alias() +"] at URL:[" + _source.url().asString() + "]. Siz tag wrong. Got [" + stag_r.value + "]" ) );

      _pkgImpl->_archivesize = str::strtonum<unsigned long>(words[0]);
      _pkgImpl->_size = str::strtonum<unsigned long>(words[1]);
    }
    if ( stag_r.name == "Loc" )
    {
      std::vector<std::string> words;
      unsigned int howmany = str::split( stag_r.value, std::back_inserter(words) );
      _pkgImpl->_media_number = 1;
      if ( howmany >= 2 )
      {
        _pkgImpl->_media_number = str::strtonum<unsigned int>(words[0]);
        // if a 3rd value is given, use it as the directory specifier, else default to the architecture
        _pkgImpl->_location = _sourceImpl->sourceDir( (howmany > 2) ? words[2] : _nvrad.arch.asString() ) + words[1];
      }
      else
      {
        ZYPP_THROW( ParseException("Bad source ["+ _source.alias() +"] at URL:[" + _source.url().asString() + "]. Bad [Loc] tag. Got: [" + stag_r.value + "]"));
      }
      // ignore path
    }
  }

  /* Consume MulitTag data. */
  virtual void consume( const MultiTag & mtag_r )
  {
    if ( ! _isPendingPkg )
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
    else if ( mtag_r.name == "Rec" )
    {
      collectDeps( mtag_r.values, _nvrad[Dep::RECOMMENDS] );
    }
    else if ( mtag_r.name == "Sup" )
    {
      collectDeps( mtag_r.values, _nvrad[Dep::SUPPLEMENTS] );
    }
    else if ( mtag_r.name == "Sug" )
    {
      collectDeps( mtag_r.values, _nvrad[Dep::SUGGESTS] );
    }
    else if ( mtag_r.name == "Fre" )
    {
      collectDeps( mtag_r.values, _nvrad[Dep::FRESHENS] );
    }
    else if ( mtag_r.name == "Enh" )
    {
      collectDeps( mtag_r.values, _nvrad[Dep::ENHANCES] );
    }
    else if ( mtag_r.name == "Kwd" )
    {
      _pkgImpl->_keywords.insert( mtag_r.values.begin(), mtag_r.values.end() );
    }
    else if ( mtag_r.name == "Aut" )
    {
      _pkgImpl->_authors = mtag_r.values;
    }
  }

  virtual void endParse()
  {
    // collect last package if there is one
    collectPkg();
  }
};

////////////////////////////////////////////////////////////////////////////

PkgContent parsePackages( parser::ParserProgress::Ptr progress, Source_Ref source_r, SuseTagsImpl::Ptr sourceImpl_r, const Pathname & file_r )
{
  MIL << "Starting to parse packages " << file_r << std::endl;
  PackagesParser p( progress, source_r, sourceImpl_r );
  try
  {
    p.parse( file_r );
  }
  catch (ParseException &e)
  {
    ZYPP_CAUGHT(e);
    ERR <<  "Source [" << source_r.alias() << "] at URL:[" << source_r.url().asString() << "] has a broken packages file." << std::endl;
    ZYPP_RETHROW(e);
  }
  return p.result();
}

PkgDiskUsage parsePackagesDiskUsage( parser::ParserProgress::Ptr progress, const Pathname & file_r )
{
  MIL << "Starting to parse packages disk usage " << file_r << std::endl;
  PackageDiskUsageParser duParser(progress);
  try
  {
    duParser.parse(file_r);
  }
  catch (ParseException &e)
  {
    ZYPP_CAUGHT(e);
    ERR <<  "Broken disk usage file " << file_r << ". Ignoring." << std::endl;
    ZYPP_RETHROW(e);
  }
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
