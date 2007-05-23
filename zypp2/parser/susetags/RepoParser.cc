/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/parser/susetags/RepoParser.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/String.h"

#include "zypp2/parser/susetags/RepoParser.h"
#include "zypp/parser/susetags/ContentFileReader.h"
#include "zypp/parser/susetags/PackagesFileReader.h"
#include "zypp/parser/susetags/PackagesLangFileReader.h"
#include "zypp/parser/susetags/PatternFileReader.h"
#include "zypp/parser/susetags/RepoIndex.h"
#include "zypp/parser/tagfile/ParseException.h"

#include "zypp/ZConfig.h"

using std::endl;

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
	  Impl( const data::RecordId & catalogId_r,
		data::ResolvableDataConsumer & consumer_r,
		const ProgressData::ReceiverFnc & fnc_r )
	  : _catalogId( catalogId_r )
	  , _consumer( consumer_r )
	  {
	    _ticks.sendTo( fnc_r );
	  }

	  /** Main entry to parser. */
	  void parse( const Pathname & reporoot_r );

	  /** \name FileReader callbacks delivering data. */
	  //@{
	  void consumeIndex( const RepoIndex_Ptr & data_r )
	  {
	    SEC << "[Index]" << data_r << endl;
	    _repoIndex = data_r;
	  }

	  void consumeProd( const data::Product_Ptr & data_r )
	  {
	    SEC << "[Prod]" << data_r << endl;
	    _prodData = data_r;
	    _consumer.consumeProduct( _catalogId, _prodData );
	  }

          void consumePkg( const data::Package_Ptr & data_r )
          {
            SEC << "[Package]" << data_r << endl;
          }

          void consumeSrcPkg( const data::SrcPackage_Ptr & data_r )
          {
            SEC << "[SrcPackage]" << data_r << endl;
          }

          void consumePat( const data::Pattern_Ptr & data_r )
          {
            SEC << "[Pattern]" << data_r << endl;
          }
	  //@}

        public:

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
                return true; // got it
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

	    PackagesLangFileReader reader;
	    reader.setLocale( toParse );
	    reader.setPkgConsumer( bind( &Impl::consumePkg, this, _1 ) );
	    reader.setSrcPkgConsumer( bind( &Impl::consumeSrcPkg, this, _1 ) );
	    reader.parse( _descrdir / ("packages." + toParse.code()) );

	    if ( ! _ticks.incr() )
	      ZYPP_THROW( AbortRequestException() );
	  }

	private:
	  data::RecordId                 _catalogId;
	  data::ResolvableDataConsumer & _consumer;
	  ProgressData                   _ticks;

	private: // these (and _ticks) are actually scoped per parse() run.
	  RepoIndex_Ptr     _repoIndex;
	  data::Product_Ptr _prodData;
	  Pathname          _descrdir; // full path
	  Pathname          _datadir;  // full path

	  /** Translations processed by \ref parseLocaleIf so far.*/
	  std::set<Locale>  _parsedLocales;
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
	_descrdir = _datadir = Pathname();
	_parsedLocales.clear();

        // Content file first to get the repoindex
        {
          ContentFileReader content;
          content.setProductConsumer( bind( &Impl::consumeProd, this, _1 ) );
          content.setRepoIndexConsumer( bind( &Impl::consumeIndex, this, _1 ) );
          content.parse( reporoot_r / "content" );
        }
	if ( ! _repoIndex )
	{
	  ZYPP_THROW( ParseException( reporoot_r.asString() + ": " + "No reository index in content file." ) );
	}
	DBG << _repoIndex << endl;

	// Prepare parsing
	_descrdir = reporoot_r / _repoIndex->descrdir;
	_datadir = reporoot_r / _repoIndex->datadir;

	_ticks.name( "Parsing susetags repo at " + reporoot_r.asString() );
	_ticks.range( _repoIndex->metaFileChecksums.size() );
        if ( ! _ticks.toMin() )
          ZYPP_THROW( AbortRequestException() );

        // Start with packages
        {
          PackagesFileReader reader;
          reader.setPkgConsumer( bind( &Impl::consumePkg, this, _1 ) );
          reader.setSrcPkgConsumer( bind( &Impl::consumeSrcPkg, this, _1 ) );
          reader.parse( _descrdir / "packages" );
        }
        if ( ! _ticks.incr() )
          ZYPP_THROW( AbortRequestException() );

        // Now process packages.lang
	// Always parse 'en'. For each wanted locale at least
	// some fallback, if locale is not present.
	parseLocaleIf( Locale("en") );
	parseLocaleIf( Locale("de_DE") );
	parseLocaleIf( ZConfig().defaultTextLocale() );

        // Now process the rest of RepoIndex
	for ( RepoIndex::FileChecksumMap::const_iterator it = _repoIndex->metaFileChecksums.begin();
	      it != _repoIndex->metaFileChecksums.end(); ++it )
        {
          if ( isPatternFile( it->first ) )
          {
            PatternFileReader reader;
            reader.setConsumer( bind( &Impl::consumePat, this, _1 ) );
            reader.parse( _descrdir / it->first );
          }

          if ( ! _ticks.incr() )
            ZYPP_THROW( AbortRequestException() );
        }

        // Done
	if ( ! _ticks.toMax() )
          ZYPP_THROW( AbortRequestException() );
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
      RepoParser::RepoParser( const data::RecordId & catalogId_r,
			      data::ResolvableDataConsumer & consumer_r,
			      const ProgressData::ReceiverFnc & fnc_r )
      : _pimpl( new Impl( catalogId_r, consumer_r, fnc_r ) )
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
