/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/PackagesLangFileReader.cc
 *
*/
#include <iostream>
#include "zypp/base/Easy.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogTools.h"
#include "zypp/parser/susetags/PackagesLangFileReader.h"
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
      //	CLASS NAME : PackagesLangFileReader::Impl
      //
      /** PackagesLangFileReader implementation. */
      class PackagesLangFileReader::Impl : public BaseImpl
      {
	public:
	  Impl( const PackagesLangFileReader & parent_r,
		const Locale & locale_r )
	  : BaseImpl( parent_r ), _locale( locale_r )
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
	  { /*NOP*/ }

	  /** Consume =:Pkg. */
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
	      _data->arch = words[3];
	    }
	    _data->name    = words[0];
	    _data->edition = words[1] + "-" + words[2];
	  }

	  /** Consume =Sum:. */
	  void consumeSum( const SingleTagPtr & tag_r )
	  {
	    _data->summary.setText( tag_r->value, _locale );
	  }

	public: // multi tags
	  /** Consume +Des:. */
	  void consumeDes( const MultiTagPtr & tag_r )
	  {
	    _data->description.setText( tag_r->value, _locale );
	  }

	  /** Consume +Eul:. */
	  void consumeEul( const MultiTagPtr & tag_r )
	  {
	    _data->licenseToConfirm.setText( tag_r->value, _locale );
	  }

	  /** Consume +Ins:. */
	  void consumeIns( const MultiTagPtr & tag_r )
	  {
	    _data->insnotify.setText( tag_r->value, _locale );
	  }

	  /** Consume +Del:. */
	  void consumeDel( const MultiTagPtr & tag_r )
	  {
	    _data->delnotify.setText( tag_r->value, _locale );
	  }

	public:
	  DefaultIntegral<unsigned,0> _c_pkg;
	  DefaultIntegral<unsigned,0> _c_srcpkg;

	private:
	  Locale                  _locale;
	  data::Packagebase_Ptr   _data;
	  data::Package_Ptr       _pkgData;
	  data::SrcPackage_Ptr    _srcpkgData;
      };
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PackagesLangFileReader
      //
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesLangFileReader::PackagesLangFileReader
      //	METHOD TYPE : Ctor
      //
      PackagesLangFileReader::PackagesLangFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesLangFileReader::~PackagesLangFileReader
      //	METHOD TYPE : Dtor
      //
      PackagesLangFileReader::~PackagesLangFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesLangFileReader::beginParse
      //	METHOD TYPE : void
      //
      void PackagesLangFileReader::beginParse()
      {
	_pimpl.reset( new Impl( *this, _locale ) );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesLangFileReader::consume
      //	METHOD TYPE : void
      //
      void PackagesLangFileReader::consume( const SingleTagPtr & tag_r )
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
	else if TAGFWD( Sum );
	else if TAGFWD( Ver );
	else
	{ WAR << errPrefix( tag_r, "Unknown tag" ) << endl; }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesLangFileReader::consume
      //	METHOD TYPE : void
      //
      void PackagesLangFileReader::consume( const MultiTagPtr & tag_r )
      {
	if TAGFWD( Des );
	else if TAGFWD( Eul );
	else if TAGFWD( Ins );
	else if TAGFWD( Del );
	else
	{ WAR << errPrefix( tag_r, "Unknown tag" ) << endl; }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesLangFileReader::lastData
      //	METHOD TYPE : void
      //
      void PackagesLangFileReader::endParse()
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
	MIL << "[PackagesLang]" << "(" << _pimpl->_c_pkg << "|" << _pimpl->_c_srcpkg << ")" << endl;
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
