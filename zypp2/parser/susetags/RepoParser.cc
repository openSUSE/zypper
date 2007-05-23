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
      /** RepoParser implementation. */
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

	  void parse( const Pathname & reporoot_r );

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

        public:

          bool isPatternFile( const std::string & name_r ) const
          {
            return( name_r.size() > 4 && name_r.substr( name_r.size() - 4 ) == ".pat" );
          }

	private:
	  data::RecordId                 _catalogId;
	  data::ResolvableDataConsumer & _consumer;
	  ProgressData                   _ticks;

	  RepoIndex_Ptr     _repoIndex;
	  data::Product_Ptr _prodData;
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
        Pathname descrdir( reporoot_r / _repoIndex->descrdir );

	_ticks.name( "Parsing susetags repo at " + reporoot_r.asString() );
	_ticks.range( _repoIndex->metaFileChecksums.size() );
        if ( ! _ticks.toMin() )
          ZYPP_THROW( AbortRequestException() );

        // Start with packages
        {
          PackagesFileReader reader;
          reader.setPkgConsumer( bind( &Impl::consumePkg, this, _1 ) );
          reader.setSrcPkgConsumer( bind( &Impl::consumeSrcPkg, this, _1 ) );
          reader.parse( descrdir / "packages" );
        }
        if ( ! _ticks.incr() )
          ZYPP_THROW( AbortRequestException() );

#warning FIX selection of language files to parse
        // Now process packages.lang
	//for ( RepoIndex::FileChecksumMap::const_iterator it = _repoIndex->metaFileChecksums.begin();
	//      it != _repoIndex->metaFileChecksums.end(); ++it )
        {
          PackagesLangFileReader reader;
          reader.setLocale( Locale("en") );
          reader.setPkgConsumer( bind( &Impl::consumePkg, this, _1 ) );
          reader.setSrcPkgConsumer( bind( &Impl::consumeSrcPkg, this, _1 ) );
          reader.parse( descrdir / "packages.en" );
        }
        if ( ! _ticks.incr() )
          ZYPP_THROW( AbortRequestException() );


        // Now process the RepoIndex
	for ( RepoIndex::FileChecksumMap::const_iterator it = _repoIndex->metaFileChecksums.begin();
	      it != _repoIndex->metaFileChecksums.end(); ++it )
        {
          if ( isPatternFile( it->first ) )
          {
            PatternFileReader reader;
            reader.setConsumer( bind( &Impl::consumePat, this, _1 ) );
            reader.parse( descrdir / it->first );
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
