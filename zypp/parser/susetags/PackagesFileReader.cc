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
#include "zypp/data/ResolvableData.h"

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
      struct PackagesFileReader::Impl
      {
	///data::Package
	Impl() { SEC << endl; }
	~Impl() { SEC << endl; }
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
      //	METHOD NAME : PackagesFileReader::
      //	METHOD TYPE : void
      //
      void PackagesFileReader::beginParse()
      {
	_pimpl.reset( new Impl );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesFileReader::
      //	METHOD TYPE : void
      //
      void PackagesFileReader::consume( const SingleTagPtr & tag_r )
      {
#define TAGN(V) tag_r->name == #V

	if ( TAGN( Pkg ) )
	{}
	else if ( TAGN( Cks ) )
	{}
	else if ( TAGN( Grp ) )
	{}
	else if ( TAGN( Vnd ) )
	{}
	else if ( TAGN( Lic ) )
	{}
	else if ( TAGN( Src ) )
	{}
	else if ( TAGN( Tim ) )
	{}
	else if ( TAGN( Loc ) )
	{}
	else if ( TAGN( Siz ) )
	{}
	else if ( TAGN( Shr ) )
	{}
	else if ( TAGN( Ver ) )
	{}
	else
	{ ERR << tag_r << endl; }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesFileReader::
      //	METHOD TYPE : void
      //
      void PackagesFileReader::consume( const MultiTagPtr & tag_r )
      {
  	if ( TAGN( Req ) )
	{}
	else if ( TAGN( Prq ) )
	{}
	else if ( TAGN( Prv ) )
	{}
	else if ( TAGN( Con ) )
	{}
	else if ( TAGN( Obs ) )
	{}
	else if ( TAGN( Rec ) )
	{}
	else if ( TAGN( Fre ) )
	{}
	else if ( TAGN( Enh ) )
	{}
	else if ( TAGN( Sug ) )
	{}
	else if ( TAGN( Sup ) )
	{}
	else if ( TAGN( Kwd ) )
	{}
	else if ( TAGN( Aut ) )
	{}
	else
	{ ERR << tag_r << endl; }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : PackagesFileReader::
      //	METHOD TYPE : void
      //
      void PackagesFileReader::endParse()
      {
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
