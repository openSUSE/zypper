/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/PackagesFileReader.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/parser/susetags/PackagesFileReader.h"
#include "zypp/parser/susetags/FileReaderBaseImpl.h"

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
      //	CLASS NAME : PackagesFileReader::Impl
      //
      /** PackagesFileReader implementation. */
      class PackagesFileReader::Impl : public BaseImpl
      {
	public:
	  Impl( const PackagesFileReader & parent_r )
	  : BaseImpl( parent_r )
	  {}

	  virtual ~Impl()
	  {}

	  bool hasPackage() const
	  { return _pkgData; }

	  bool hasSourcepackage() const
	  { return _srcpkgData; }

	  data::Package_Ptr handoutPackage()
	  {
	    data::Package_Ptr ret;
	    ret.swap( _pkgData );
	    _srcpkgData = 0;
	    _data       = 0;
	    return ret;
	  }

	  data::SrcPackage_Ptr handoutSourcepackage()
	  {
	    data::SrcPackage_Ptr ret;
	    ret.swap( _srcpkgData );
	    _pkgData = 0;
	    _data    = 0;
	    return ret;
	  }

	public: // single tags
	  /** Consume =Ver:. */
	  void consumeVer( const SingleTagPtr & tag_r )
	  { /* NOP */; }

	  /** Consume =Pkg:. */
	  void consumePkg( const SingleTagPtr & tag_r )
	  {
	    std::vector<std::string> words;
	    if ( str::split( tag_r->value, std::back_inserter(words) ) != 4 )
	    {
	      ZYPP_THROW( error( tag_r, "Expected [name version release arch]") );
	    }

	    if ( words[3] == "src" || words[3] == "nosrc")
	    {
	      ++_c_srcpkg;
	      _data = _srcpkgData = new data::SrcPackage;
	      _pkgData = 0;
	      // _data->arch is arch_noarch per default
	    }
	    else
	    {
	      ++_c_pkg;
	      _data = _pkgData = new data::Package;
	      _srcpkgData = 0;
	      _data->arch = Arch( words[3] );
	    }
	    _data->name    = words[0];
	    _data->edition = Edition( words[1],words[2] );
	  }

	  /** Consume =Cks:. */
	  void consumeCks( const SingleTagPtr & tag_r )
	  {
	    std::vector<std::string> words;
	    if ( str::split( tag_r->value, std::back_inserter(words) ) != 2 )
	    {
	      ZYPP_THROW( error( tag_r, "Expected [type checksum]") );
	    }
	    _data->checksum = CheckSum( words[0], words[1] );
	  }

	  /** Consume =Grp:. */
	  void consumeGrp( const SingleTagPtr & tag_r )
	  {
	    _data->group = tag_r->value;
	  }

	  /** Consume =Vnd:. */
	  void consumeVnd( const SingleTagPtr & tag_r )
	  {
	    _data->vendor = tag_r->value;
	  }

	  /** Consume =Lic:. */
	  void consumeLic( const SingleTagPtr & tag_r )
	  {
	    _data->license = tag_r->value;
	  }

	  /** Consume =Src:. */
	  void consumeSrc( const SingleTagPtr & tag_r )
	  {
	    if ( ! _pkgData )
	    {
	      ZYPP_THROW( error( tag_r, "Unexpected sourcepackage definition for sourcepackage") );
	    }

	    std::vector<std::string> words;
	    if ( str::split( tag_r->value, std::back_inserter(words) ) != 4 )
	    {
	      ZYPP_THROW( error( tag_r, "Expected sourcepackages [name version release arch]") );
	    }

	    _pkgData->srcPackageIdent.reset( new NVR( words[0], Edition( words[1],words[2] ) ) );
	  }

	  /** Consume =Tim:. */
	  void consumeTim( const SingleTagPtr & tag_r )
	  {
	    _data->build_time = str::strtonum<Date::ValueType>( tag_r->value );
	  }

	  /** Consume =Loc:. */
	  void consumeLoc( const SingleTagPtr & tag_r )
	  {
	    std::vector<std::string> words;
	    switch ( str::split( tag_r->value, std::back_inserter(words) ) )
	    {
	      case 2: // [medianr filename]
		str::strtonum( words[0], _data->source_media_nr );
		_data->location = _data->arch.asString();
		_data->location /= words[1];
		break;

	      case 3: // [medianr filename dir]
		str::strtonum( words[0], _data->source_media_nr );
		_data->location = words[2];
		_data->location /= words[1];
    		break;

	      default:
		ZYPP_THROW( error( tag_r, "Expected [medianr filename dir]") );
		break;
	    }
	  }

	  /** Consume =Siz:. */
	  void consumeSiz( const SingleTagPtr & tag_r )
	  {
	    std::vector<std::string> words;
	    if ( str::split( tag_r->value, std::back_inserter(words) ) != 2 )
	    {
	      ZYPP_THROW( error( tag_r, "Expected [archivesize size]") );
	    }
	    _data->archive_size = str::strtonum<ByteCount::SizeType>( words[0] );
	    _data->size         = str::strtonum<ByteCount::SizeType>( words[1] );
	  }

	  /** Consume =Shr:. */
	  void consumeShr( const SingleTagPtr & tag_r )
	  {
#warning TBD
	  }

	public: // multi tags
	  /** Consume +Req:. */
	  void consumeReq( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::REQUIRES] );
	  }

	  /** Consume +Prq:. */
	  void consumePrq( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::PREREQUIRES] );
	  }

	  /** Consume +Prv:. */
	  void consumePrv( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::PROVIDES] );
	  }

	  /** Consume +Con:. */
	  void consumeCon( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::CONFLICTS] );
	  }

	  /** Consume +Obs:. */
	  void consumeObs( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::OBSOLETES] );
	  }

	  /** Consume +Rec:. */
	  void consumeRec( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::RECOMMENDS] );
	  }

	  /** Consume +Fre:. */
	  void consumeFre( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::FRESHENS] );
	  }

	  /** Consume +Enh:. */
	  void consumeEnh( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::ENHANCES] );
	  }

	  /** Consume +Sug:. */
	  void consumeSug( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::SUGGESTS] );
	  }

	  /** Consume +Sup:. */
	  void consumeSup( const MultiTagPtr & tag_r )
	  {
	    depParse<Package>( tag_r, _data->deps[Dep::SUPPLEMENTS] );
	  }

	  /** Consume +Kwd:. */
	  void consumeKwd( const MultiTagPtr & tag_r )
	  {
	    _data->authors.swap( tag_r->value );
	  }

	  /** Consume +Aut:. */
	  void consumeAut( const MultiTagPtr & tag_r )
	  {
	    _data->authors.swap( tag_r->value );
	  }

	public:
	  DefaultIntegral<unsigned,0> _c_pkg;
	  DefaultIntegral<unsigned,0> _c_srcpkg;

	private:
	  data::Packagebase_Ptr   _data;
	  data::Package_Ptr       _pkgData;
	  data::SrcPackage_Ptr    _srcpkgData;
      };
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PackagesFileReader
      //
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesFileReader::PackagesFileReader
      //	METHOD TYPE : Ctor
      //
      PackagesFileReader::PackagesFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesFileReader::~PackagesFileReader
      //	METHOD TYPE : Dtor
      //
      PackagesFileReader::~PackagesFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesFileReader::beginParse
      //	METHOD TYPE : void
      //
      void PackagesFileReader::beginParse()
      {
	_pimpl.reset( new Impl(*this) );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesFileReader::consume
      //	METHOD TYPE : void
      //
      void PackagesFileReader::consume( const SingleTagPtr & tag_r )
      {
#define TAGN(V)   tag_r->name == #V
#define TAGFWD(V) ( TAGN(V) ) _pimpl->consume##V( tag_r )

	if ( TAGN( Pkg ) )
	{
	  // consume old data
	  if ( _pimpl->hasPackage() )
	  {
	    if ( _pkgConsumer )
	      _pkgConsumer( _pimpl->handoutPackage() );
	  }
	  else if ( _pimpl->hasSourcepackage() )
	  {
	    if ( _srcPkgConsumer )
	      _srcPkgConsumer( _pimpl->handoutSourcepackage() );
	  }
	  // start new data
	  _pimpl->consumePkg( tag_r );
	}
	else if TAGFWD( Cks );
	else if TAGFWD( Grp );
	else if TAGFWD( Vnd );
	else if TAGFWD( Lic );
	else if TAGFWD( Src );
	else if TAGFWD( Tim );
	else if TAGFWD( Loc );
	else if TAGFWD( Siz );
	else if TAGFWD( Shr );
	else if TAGFWD( Ver );
	else
	{ ERR << tag_r << endl; }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesFileReader::consume
      //	METHOD TYPE : void
      //
      void PackagesFileReader::consume( const MultiTagPtr & tag_r )
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
	else if TAGFWD( Kwd );
	else if TAGFWD( Aut );
	else
	{ ERR << tag_r << endl; }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesFileReader::lastData
      //	METHOD TYPE : void
      //
      void PackagesFileReader::endParse()
      {
        // consume oldData
	if ( _pimpl->hasPackage() )
	{
	  if ( _pkgConsumer )
	    _pkgConsumer( _pimpl->handoutPackage() );
	}
	else if ( _pimpl->hasSourcepackage() )
	{
	  if ( _srcPkgConsumer )
	    _srcPkgConsumer( _pimpl->handoutSourcepackage() );
	}
	MIL << "[Packages]" << "(" << _pimpl->_c_pkg << "|" << _pimpl->_c_srcpkg << ")" << endl;
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
