/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/RepoParser.cc
 *
*/
#include <iostream>
#include "zypp/base/Easy.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include "zypp/parser/susetags/FileReaderBaseImpl.h"
#include "zypp/parser/susetags/RepoParser.h"
#include "zypp/parser/susetags/ContentFileReader.h"
#include "zypp/parser/susetags/PackagesFileReader.h"
#include "zypp/parser/susetags/PackagesLangFileReader.h"
#include "zypp/parser/susetags/PatternFileReader.h"
#include "zypp/parser/susetags/RepoIndex.h"
#include "zypp/parser/ParseException.h"

#include "zypp/ZConfig.h"

using std::endl;
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser::susetags"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : RepoParser::Impl
      //
      /** RepoParser implementation.
       * \todo Clean data on exeption.
      */
      class RepoParser::Impl
      {
	public:
	  Impl( const data::RecordId & repositoryId_r,
		data::ResolvableDataConsumer & consumer_r,
		const ProgressData::ReceiverFnc & fnc_r )
	  : _repositoryId( repositoryId_r )
	  , _consumer( consumer_r )
	  {
	    _ticks.sendTo( fnc_r );
	  }

	  /** Main entry to parser. */
	  void parse( const Pathname & reporoot_r );

	  /** \name FileReader callbacks delivering data. */
	  //@{
	  ///////////////////////////////////////////////////////////////////
	  void consumeIndex( const RepoIndex_Ptr & data_r )
	  {
	    //SEC << "[Index]" << data_r << endl;
	    _repoIndex = data_r;
	  }

	  ///////////////////////////////////////////////////////////////////
	  void consumeProd( const data::Product_Ptr & data_r )
	  {
	    MIL << "[Product] " << data_r << endl;
	    ++_stats.prod;
	    _prodData = data_r;
	    _defaultVendor = data_r->vendor;
	    _consumer.consumeProduct( _repositoryId, data_r );
	  }

	  ///////////////////////////////////////////////////////////////////
          void consumePkg( const data::Package_Ptr & data_r )
          {
	    fixVendor( data_r );
	    fixLocationPath( data_r );
	    resolveSharedDataTag( data_r );

	    ++_stats.pack;
	    data::RecordId newid = _consumer.consumePackage( _repositoryId, data_r );

	    // remember for later reference
	    idMapAdd( makeSharedIdent( ResTraits<Package>::kind,
		                       data_r->name,
				       data_r->edition,
				       data_r->arch ),
		      newid );
          }

	  ///////////////////////////////////////////////////////////////////
          void consumeSrcPkg( const data::SrcPackage_Ptr & data_r )
          {
	    fixVendor( data_r );
	    fixLocationPath( data_r );
	    resolveSharedDataTag( data_r );

	    ++_stats.srcp;
	    data::RecordId newid = _consumer.consumeSourcePackage( _repositoryId, data_r );

	    // remember for later reference
	    idMapAdd( makeSharedIdent( ResTraits<SrcPackage>::kind,
		                       data_r->name,
				       data_r->edition,
				       data_r->arch ),
		      newid );
          }

	  ///////////////////////////////////////////////////////////////////
	  void consumePkgLang( const data::Package_Ptr & data_r )
          {
	    data::RecordId id = idMapGet( makeSharedIdent( ResTraits<Package>::kind,
							   data_r->name,
							   data_r->edition,
							   data_r->arch ) );
	    if ( id != data::noRecordId )
	    {
	      _consumer.updatePackageLang( id, data_r );
	    }
	  }

	  ///////////////////////////////////////////////////////////////////
          void consumeSrcPkgLang( const data::SrcPackage_Ptr & data_r )
          {
	    data::RecordId id = idMapGet( makeSharedIdent( ResTraits<SrcPackage>::kind,
							   data_r->name,
							   data_r->edition,
							   data_r->arch ) );
	    if ( id != data::noRecordId )
	    {
	      _consumer.updatePackageLang( id, data_r );
	    }
	  }

	  ///////////////////////////////////////////////////////////////////
          void consumePat( const data::Pattern_Ptr & data_r )
          {
            //SEC << "[Pattern]" << data_r << endl;
	    fixVendor( data_r );
	    ++_stats.patt;
	    _consumer.consumePattern( _repositoryId, data_r );
          }
	  //@}

	public:
	  /** Use products vendor if vendor was not specified. */
	  void fixVendor( const data::ResObject_Ptr & data_r )
	  {
	    if ( data_r->vendor.empty() && ! _defaultVendor.empty() )
	    {
	      data_r->vendor = _defaultVendor;
	    }
	  }

	  /** Prepend location with 'datadir'. */
	  void fixLocationPath( const data::Packagebase_Ptr & data_r )
	  {
	    Pathname tofix( data_r->repositoryLocation.filename() );
	    data_r->repositoryLocation.changeFilename( _datadir / tofix );
	  }

	  /** Resolve shared data tag. */
	  void resolveSharedDataTag( const data::Packagebase_Ptr & data_r )
	  {
	    if ( ! data_r->sharedDataTag.empty() )
	    {
	      data_r->shareDataWith = idMapGet( data_r->sharedDataTag );
	    }
	  }

	public:
	  /** Throw \ref ParseException if a required file is not
	   * available below \ref _reporoot on disk.
	  */
	  Pathname assertMandatoryFile( const Pathname & file_r ) const
	  {
	    PathInfo inputfile( _reporoot / file_r );
	    if ( ! inputfile.isFile() )
	    {
	      ZYPP_THROW( ParseException( _reporoot.asString() + ": " + _("Required file is missing: ") + file_r.asString() ) );
	    }
	    return inputfile.path();
	  }

	  /** Print a warning if an optional file is not
	   * available below \ref _reporoot on disk.
	  */
 	  Pathname getOptionalFile( const Pathname & file_r ) const
	  {
	    PathInfo inputfile( _reporoot / file_r );
	    if ( ! inputfile.isFile() )
	    {
	      WAR << _reporoot << ": Skip optional file: " <<  file_r.asString() << endl;
	      return Pathname();
	    }
	    return inputfile.path();
	  }

	  bool isPatternFile( const std::string & name_r ) const
          {
            return( name_r.size() > 4 && name_r.substr( name_r.size() - 4 ) == ".pat" );
          }

          /** Test for \c packages.lang in \ref _repoIndex.*/
	  bool haveLocale( const Locale & locale_r ) const
          {
            std::string searchFor( "packages." + locale_r.code() );
	    for ( RepoIndex::FileChecksumMap::const_iterator it = _repoIndex->metaFileChecksums.begin();
	          it != _repoIndex->metaFileChecksums.end(); ++it )
	    {
	      if ( it->first == searchFor )
	      {
		// got it
		PathInfo inputfile( _reporoot / _descrdir / searchFor );
		if ( ! inputfile.isFile() )
		{
		  WAR << "Known and desired file is not on disk: " << inputfile << endl;
		}
		else
		  return true; // got it
	      }
	    }
	    return false; // not found
	  }

	  /** Take care translations for \a locale_r or an appropriate
	   * fallback were parsed.
	  */
	  void parseLocaleIf( const Locale & locale_r )
	  {
	    Locale toParse( locale_r );
	    bool alreadyParsed = false;

	    while ( toParse != Locale::noCode )
	    {
	      alreadyParsed = ( _parsedLocales.find( toParse ) != _parsedLocales.end() );
	      if ( alreadyParsed || haveLocale( toParse ) )
		break; // no return because we want to log in case of a fallback
	      toParse = toParse.fallback();
	    }

	    if ( toParse != locale_r )
	    {
	      WAR << "Using fallback locale " << toParse << " for " << locale_r << endl;
	    }

	    if ( alreadyParsed )
	    {
	      return; // now return...
	    }
	    // ...or parse
	    _parsedLocales.insert( toParse ); // don't try again.

	    Pathname inputfile( getOptionalFile( _descrdir / ("packages." + toParse.code()) ) );
	    if ( ! inputfile.empty() )
	    {
	      PackagesLangFileReader reader;
	      reader.setLocale( toParse );
	      reader.setPkgConsumer( bind( &Impl::consumePkgLang, this, _1 ) );
	      reader.setSrcPkgConsumer( bind( &Impl::consumeSrcPkgLang, this, _1 ) );
	      CombinedProgressData progress( _ticks, PathInfo(inputfile).size()  );
	      reader.parse( inputfile, progress );
	    }

            if ( ! _ticks.incr( PathInfo(inputfile).size() ) )
              ZYPP_THROW( AbortRequestException() );
	  }

	private:
	  void idMapAdd( const std::string & key_r, data::RecordId value_r )
	  {
	    if ( _idMap[key_r] != data::noRecordId )
	    {
	      WAR << "Multiple record ids for " << key_r
		  << " (first " << _idMap[key_r] << ", now " << value_r << ")" << endl;
	    }
	    _idMap[key_r] = value_r;
	  }

	  data::RecordId idMapGet( const std::string & key_r )
	  {
	    data::RecordId ret = _idMap[key_r];
	    if ( ret == data::noRecordId )
	    {
	      WAR << "No record id for " << key_r << endl;
	    }
	    return ret;
	  }

	private:
	  data::RecordId                 _repositoryId;
	  data::ResolvableDataConsumer & _consumer;
	  ProgressData                   _ticks;

	private: // these (and _ticks) are actually scoped per parse() run.
	  RepoIndex_Ptr     _repoIndex;
	  data::Product_Ptr _prodData;
	  std::string       _defaultVendor;
	  Pathname          _reporoot; // full path
	  Pathname          _descrdir; // path below reporoot
	  Pathname          _datadir;  // path below reporoot

	  /** Translations processed by \ref parseLocaleIf so far.*/
	  std::set<Locale>  _parsedLocales;

	  /** Remember the record ids of created packages and soucepackages. */
	  std::map<std::string,data::RecordId> _idMap;

	  struct Stats {
	    DefaultIntegral<unsigned,0> prod;
	    DefaultIntegral<unsigned,0> patt;
	    DefaultIntegral<unsigned,0> pack;
	    DefaultIntegral<unsigned,0> srcp;
	  };
	  Stats _stats;
      };
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : RepoParser::Impl::parse
      //	METHOD TYPE : void
      //
      void RepoParser::Impl::parse( const Pathname & reporoot_r )
      {
	_prodData = 0;
	_repoIndex = 0;
	_defaultVendor.clear();
	_reporoot = reporoot_r;
	_descrdir = _datadir = Pathname();
	_parsedLocales.clear();

        // Content file first to get the repoindex
        {
	  Pathname inputfile( assertMandatoryFile( "content" ) );
          ContentFileReader content;
          content.setProductConsumer( bind( &Impl::consumeProd, this, _1 ) );
          content.setRepoIndexConsumer( bind( &Impl::consumeIndex, this, _1 ) );
          content.parse( inputfile );
        }
	if ( ! _repoIndex )
	{
	  ZYPP_THROW( ParseException( reporoot_r.asString() + ": " + "No reository index in content file." ) );
	}
	DBG << _repoIndex << endl;

	// Prepare parsing
	_descrdir = _repoIndex->descrdir; // path below reporoot
	_datadir  = _repoIndex->datadir;  // path below reporoot

	_ticks.name( "Parsing susetags repo at " + reporoot_r.asString() );

        // calculate progress range based on file sizes to parse
        int jobssize = 0;
        for ( RepoIndex::FileChecksumMap::const_iterator it = _repoIndex->metaFileChecksums.begin();
	      it != _repoIndex->metaFileChecksums.end(); ++it )
        {
          jobssize += PathInfo(getOptionalFile(_descrdir / it->first)).size();

        }
        MIL << "Total job size: " << jobssize << endl;

	_ticks.range(jobssize);

        if ( ! _ticks.toMin() )
          ZYPP_THROW( AbortRequestException() );

        // Start with packages
        {
          Pathname inputfile( assertMandatoryFile( _descrdir / "packages" ) );
          PackagesFileReader reader;
          reader.setPkgConsumer( bind( &Impl::consumePkg, this, _1 ) );
          reader.setSrcPkgConsumer( bind( &Impl::consumeSrcPkg, this, _1 ) );

          CombinedProgressData packageprogress( _ticks, PathInfo(inputfile).size() );
          reader.parse( inputfile, packageprogress );
        }

        // Now process packages.lang. Always parse 'en'.
	// At least packages.en is mandatory, because the file might
	// contain license texts.
	assertMandatoryFile( _descrdir / "packages.en" );
	parseLocaleIf( Locale("en") );
	// For each wanted locale at least
	// some fallback, if locale is not present.
	parseLocaleIf( ZConfig::instance().textLocale() );

        // Now process the rest of RepoIndex
	for ( RepoIndex::FileChecksumMap::const_iterator it = _repoIndex->metaFileChecksums.begin();
	      it != _repoIndex->metaFileChecksums.end(); ++it )
        {
          if ( isPatternFile( it->first ) )
          {
	    Pathname inputfile( getOptionalFile( _descrdir / it->first) );
	    if ( ! inputfile.empty() )
	    {
	      PatternFileReader reader;
	      reader.setConsumer( bind( &Impl::consumePat, this, _1 ) );
              CombinedProgressData patternprogress( _ticks, PathInfo(inputfile).size()  );
	      reader.parse( inputfile, patternprogress );
	    }
          }
        }

        // Done
	if ( ! _ticks.toMax() )
          ZYPP_THROW( AbortRequestException() );

	MIL << "DONE " << reporoot_r << "("
	    << _stats.prod << " products, "
	    << _stats.patt << " patterns, "
	    << _stats.pack << " packages, "
	    << _stats.srcp << " srcpackages)" << endl;
      }
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : RepoParser
      //
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : RepoParser::RepoParser
      //	METHOD TYPE : Ctor
      //
      RepoParser::RepoParser( const data::RecordId & repositoryId_r,
			      data::ResolvableDataConsumer & consumer_r,
			      const ProgressData::ReceiverFnc & fnc_r )
      : _pimpl( new Impl( repositoryId_r, consumer_r, fnc_r ) )
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : RepoParser::~RepoParser
      //	METHOD TYPE : Dtor
      //
      RepoParser::~RepoParser()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : RepoParser::parse
      //	METHOD TYPE : void
      //
      void RepoParser::parse( const Pathname & reporoot_r )
      {
	_pimpl->parse( reporoot_r );
      }

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
