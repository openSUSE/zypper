/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/PatternFileReader.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/parser/susetags/PatternFileReader.h"
#include "zypp/parser/susetags/FileReaderBaseImpl.h"

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
      //	CLASS NAME : PatternFileReader::Impl
      //
      /** PatternFileReader implementation. */
      class PatternFileReader::Impl : public BaseImpl
      {
	public:
	  Impl( const PatternFileReader & parent_r )
	  : BaseImpl( parent_r )
	  {}

	  virtual ~Impl()
	  {}

	  bool hasData() const
	  { return _data; }

	  data::Pattern_Ptr handoutData()
	  {
	    data::Pattern_Ptr ret;
	    ret.swap( _data );
	    return ret;
	  }

	public: // single tags
	  /** Consume =Ver:. */
	  void consumeVer( const SingleTagPtr & tag_r )
	  { /* NOP */; }

	  /** Consume =Pat:. */
	  void consumePat( const SingleTagPtr & tag_r )
	  {
	    std::vector<std::string> words;
	    if ( str::split( tag_r->value, std::back_inserter(words) ) != 4 )
	    {
	      ZYPP_THROW( error( tag_r, "Expected [name version release arch]") );
	    }

	    _data = new data::Pattern;
	    _data->name    = words[0];
	    _data->edition = Edition( words[1],words[2] );
	    _data->arch    = Arch( words[3] );
	  }

	  /** Consume =Sum:. */
	  void consumeSum( const SingleTagPtr & tag_r )
	  {
	    _data->summary.setText( tag_r->value, Locale(tag_r->modifier) );
	  }

	  /** Consume =Vis:. */
	  void consumeVis( const SingleTagPtr & tag_r )
	  {
	    _data->userVisible = ( tag_r->value == "true" );
	  }

	  /** Consume =Cat:. */
	  void consumeCat( const SingleTagPtr & tag_r )
	  {
	    _data->category.setText( tag_r->value, Locale(tag_r->modifier) );
	  }

	  /** Consume =Ico:. */
	  void consumeIco( const SingleTagPtr & tag_r )
	  {
	    _data->icon = tag_r->value;
	  }

	  /** Consume =Ord:. */
	  void consumeOrd( const SingleTagPtr & tag_r )
	  {
	    _data->order = tag_r->value;
	  }

	public: // multi tags
	  /** Consume +Req:. */
	  void consumeReq( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->deps[Dep::REQUIRES] );
	  }

	  /** Consume +Prv:. */
	  void consumePrv( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->deps[Dep::PROVIDES] );
	  }

	  /** Consume +Con:. */
	  void consumeCon( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->deps[Dep::CONFLICTS] );
	  }

	  /** Consume +Obs:. */
	  void consumeObs( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->deps[Dep::OBSOLETES] );
	  }

	  /** Consume +Rec:. */
	  void consumeRec( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->deps[Dep::RECOMMENDS] );
	  }

	  /** Consume +Fre:. */
	  void consumeFre( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->deps[Dep::FRESHENS] );
	  }

	  /** Consume +Enh:. */
	  void consumeEnh( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->deps[Dep::ENHANCES] );
	  }

	  /** Consume +Sug:. */
	  void consumeSug( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->deps[Dep::SUGGESTS] );
	  }

	  /** Consume +Sup:. */
	  void consumeSup( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->deps[Dep::SUPPLEMENTS] );
	  }

	  // package related dependencies:

	  /** Consume +Prq:. */
	  void consumePrq( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::REQUIRES] );
	  }

	  /** Consume +Pcn:. */
	  void consumePcn( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::CONFLICTS] );
	  }

	  /** Consume +Pob:. */
	  void consumePob( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::OBSOLETES] );
	  }

	  /** Consume +Prc:. */
	  void consumePrc( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::RECOMMENDS] );
	  }

	  /** Consume +Pfr:. */
	  void consumePfr( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::FRESHENS] );
	  }

	  /** Consume +Pen:. */
	  void consumePen( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::ENHANCES] );
	  }

	  /** Consume +Psg:. */
	  void consumePsg( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::SUGGESTS] );
	  }

	  /** Consume +Psp:. */
	  void consumePsp( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::SUPPLEMENTS] );
	  }

	  // non dependency tags

	  /** Consume +Des:. */
	  void consumeDes( const MultiTagPtr & tag_r )
	  {
	    _data->description.setText( tag_r->value, Locale(tag_r->modifier) );
	  }

	  /** Consume +Inc:. */
	  void consumeInc( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->includes );
	  }

	  /** Consume +Ext:. */
	  void consumeExt( const MultiTagPtr & tag_r )
	  {
	    depParse<Pattern>( tag_r, _data->extends );
	  }

	private:
	  data::Pattern_Ptr _data;
      };
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PatternFileReader
      //
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PatternFileReader::PatternFileReader
      //	METHOD TYPE : Ctor
      //
      PatternFileReader::PatternFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PatternFileReader::~PatternFileReader
      //	METHOD TYPE : Dtor
      //
      PatternFileReader::~PatternFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PatternFileReader::beginParse
      //	METHOD TYPE : void
      //
      void PatternFileReader::beginParse()
      {
	_pimpl.reset( new Impl(*this) );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PatternFileReader::consume
      //	METHOD TYPE : void
      //
      void PatternFileReader::consume( const SingleTagPtr & tag_r )
      {
#define TAGN(V)   tag_r->name == #V
#define TAGFWD(V) ( TAGN(V) ) _pimpl->consume##V( tag_r )

	if ( TAGN( Pat ) )
	{
	  // consume old data
	  if ( _pimpl->hasData() )
	  {
	    if ( _consumer )
	      _consumer( _pimpl->handoutData() );
	  }
	  // start new data
	  _pimpl->consumePat( tag_r );
	}
        else
        {
          if ( _pimpl->hasData() )
          {
            if TAGFWD( Sum );
            else if TAGFWD( Vis );
            else if TAGFWD( Cat );
            else if TAGFWD( Ico );
            else if TAGFWD( Ord );
            else
            { WAR << errPrefix( tag_r, "Unknown tag" ) << endl; }
          }
          else
          {
            if TAGFWD( Ver );
            else
            { WAR << errPrefix( tag_r, "Unknown header tag" ) << endl; }
          }
        }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PatternFileReader::consume
      //	METHOD TYPE : void
      //
      void PatternFileReader::consume( const MultiTagPtr & tag_r )
      {
        if ( _pimpl->hasData() )
        {
          if TAGFWD( Req );
          else if TAGFWD( Prq );
          else if TAGFWD( Prv );
          else if TAGFWD( Con );
          else if TAGFWD( Obs );
          else if TAGFWD( Rec );
          else if TAGFWD( Fre );
          else if TAGFWD( Enh );
          else if TAGFWD( Sug );
          else if TAGFWD( Sup );
	  // package related dependencies
          else if TAGFWD( Prq ); // requires
          else if TAGFWD( Pcn ); // conflicts
          else if TAGFWD( Pob ); // obsoletes
          else if TAGFWD( Prc ); // recommends
          else if TAGFWD( Pfr ); // freshens
          else if TAGFWD( Pen ); // enhances
          else if TAGFWD( Psg ); // suggests
          else if TAGFWD( Psp ); // supplements
          //
          else if TAGFWD( Des );
          else if TAGFWD( Inc ); // UI hint: includes
          else if TAGFWD( Ext ); // UI hint: extends
          else
          { WAR << errPrefix( tag_r, "Unknown tag" ) << endl; }
        }
        else
        { WAR << errPrefix( tag_r, "Unknown header tag" ) << endl; }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PatternFileReader::lastData
      //	METHOD TYPE : void
      //
      void PatternFileReader::endParse()
      {
        // consume data
	if ( _pimpl->hasData() )
	{
	  if ( _consumer )
	    _consumer( _pimpl->handoutData() );
	}
	_pimpl.reset();
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
