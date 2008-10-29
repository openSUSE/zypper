/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/ProductFileReader.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Functional.h"

#include "zypp/PathInfo.h"

#include "zypp/parser/ProductFileReader.h"
#include "zypp/parser/xml/ParseDef.h"
#include "zypp/parser/xml/Reader.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    //
    // class ProductFileData::Upgrade
    //
    /////////////////////////////////////////////////////////////////

    struct ProductFileData::Upgrade::Impl
    {
      std::string                 _name;
      std::string                 _summary;
      std::string                 _product;
      DefaultIntegral<bool,false> _notify;
      std::string                 _status;
    };

    ProductFileData::Upgrade::Upgrade( Impl * allocated_r )
      : _pimpl( allocated_r ? allocated_r : new Impl )
    {}

    std::string ProductFileData::Upgrade::name()    const { return _pimpl->_name; }
    std::string ProductFileData::Upgrade::summary() const { return _pimpl->_summary; }
    std::string ProductFileData::Upgrade::product() const { return _pimpl->_product; }
    bool        ProductFileData::Upgrade::notify()  const { return _pimpl->_notify; }
    std::string ProductFileData::Upgrade::status()  const { return _pimpl->_status; }

    /////////////////////////////////////////////////////////////////
    //
    // class ProductFileData
    //
    /////////////////////////////////////////////////////////////////

    struct ProductFileData::Impl
    {
      IdString    _vendor;
      IdString    _name;
      Edition     _edition;
      Arch        _arch;

      std::string _productline;
      std::string _registerTarget;
      std::string _registerRelease;

      std::string _updaterepokey;

      Upgrades    _upgrades;
    };

    ProductFileData::ProductFileData( Impl * allocated_r )
      : _pimpl( allocated_r ? allocated_r : new Impl )
    {}

    IdString    ProductFileData::vendor()  const { return _pimpl->_vendor; }
    IdString    ProductFileData::name()    const { return _pimpl->_name; }
    Edition     ProductFileData::edition() const { return _pimpl->_edition; }
    Arch        ProductFileData::arch()    const { return _pimpl->_arch; }

    std::string ProductFileData::productline()     const { return _pimpl->_productline; }
    std::string ProductFileData::registerTarget()  const { return _pimpl->_registerTarget; }
    std::string ProductFileData::registerRelease() const { return _pimpl->_registerRelease; }

    std::string ProductFileData::updaterepokey() const { return _pimpl->_updaterepokey; }

    const ProductFileData::Upgrades & ProductFileData::upgrades() const { return _pimpl->_upgrades; }

    std::ostream & operator<<( std::ostream & str, const ProductFileData & obj )
    {
      str << str::form( "|product|%s|%s|%s|%s|%p",
                        obj.name().c_str(),
                        obj.edition().c_str(),
                        obj.arch().c_str(),
                        obj.vendor().c_str(), (const void *)&obj );
      return str;
    }

    /////////////////////////////////////////////////////////////////
    //
    // class ProductFileReader
    //
    /////////////////////////////////////////////////////////////////

    bool consumeNode( xml::Reader & reader_r )
    {
      //DBG << *reader_r << endl;
      return true;
    }

    struct ProductNode : public xml::ParseDef
    {
      ProductNode( Mode mode_r )
      : ParseDef( "product", mode_r )
      {
        (*this)("ident",       OPTIONAL)
               ("onsys",       OPTIONAL)
               ;

        (*this)["ident"]
               ("name",        OPTIONAL)
               ("version",     OPTIONAL)
               ("description", OPTIONAL)
               ("created",     OPTIONAL)
               ;

        (*this)["onsys"]
               ("entry",       MULTIPLE_OPTIONAL)
               ;
      }
    };

    bool ProductFileReader::parse( const InputStream & input_r ) const
    {
      MIL << "+++" << input_r << endl;
      bool ret = true;
      xml::ParseDef::_debug = true;

      xml::Reader reader( input_r );

      ProductNode rootNode( xml::ParseDef::MANDTAORY );
      rootNode.take( reader );
      ret = false;

      MIL << "---" << ret << " - " << input_r << endl;
      return ret;

      try
      {
        xml::Reader reader( input_r );
        ret = reader.foreachNode( consumeNode );
      }
      catch ( const Exception & err )
      {
        // parse error
        ERR << err << endl;
      }

      if ( _consumer )
      {
        static unsigned idx = 0;

        ProductFileData::Impl * i = new ProductFileData::Impl;
        i->_name = IdString( str::numstring( ++idx, 4 ) );
        ret = _consumer( i );
      }

      MIL << "---" << ret << " - " << input_r << endl;
      return ret;
    }

    /////////////////////////////////////////////////////////////////

    bool ProductFileReader::scanDir( const Consumer & consumer_r, const Pathname & dir_r )
    {
      std::list<Pathname> retlist;
      int res = filesystem::readdir( retlist, dir_r, /*dots*/false );
      if ( res != 0 )
      {
        WAR << "scanDir " << dir_r << " failed (" << res << ")" << endl;
        return true;
      }

      ProductFileReader reader( consumer_r );
      for_( it, retlist.begin(), retlist.end() )
      {
        if ( PathInfo( *it, PathInfo::LSTAT ).isFile() && ! reader.parse( *it ) )
        {
          return false; // consumer_r request to stop parsing.
        }
      }
      return true;
    }

    ProductFileData ProductFileReader::scanFile( const Pathname & file_r )
    {
      if ( ! PathInfo( file_r ).isFile() )
      {
        WAR << "scanFile " << PathInfo( file_r ) << " is no t a file." << endl;
        return ProductFileData();
      }

      ProductFileData ret;
      ProductFileReader reader( functor::getFirst( ret ), file_r );
      return ret;
   }

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
