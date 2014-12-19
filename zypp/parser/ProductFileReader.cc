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
#include "zypp/parser/xml/ParseDefConsume.h"
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
      std::string                 _repository;
      std::string                 _product;
      DefaultIntegral<bool,false> _notify;
      std::string                 _status;
    };

    ProductFileData::Upgrade::Upgrade( Impl * allocated_r )
      : _pimpl( allocated_r ? allocated_r : new Impl )
    {}

    std::string ProductFileData::Upgrade::name()       const { return _pimpl->_name; }
    std::string ProductFileData::Upgrade::summary()    const { return _pimpl->_summary; }
    std::string ProductFileData::Upgrade::repository() const { return _pimpl->_repository; }
    std::string ProductFileData::Upgrade::product()    const { return _pimpl->_product; }
    bool        ProductFileData::Upgrade::notify()     const { return _pimpl->_notify; }
    std::string ProductFileData::Upgrade::status()     const { return _pimpl->_status; }

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

      std::string _shortName;
      std::string _summary;

      std::string _productline;
      std::string _registerTarget;
      std::string _registerRelease;
      std::string _registerFlavor;

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

    std::string ProductFileData::shortName()	const { return _pimpl->_shortName; }
    std::string ProductFileData::summary()	const { return _pimpl->_summary; }

    std::string ProductFileData::productline()     const { return _pimpl->_productline; }
    std::string ProductFileData::registerTarget()  const { return _pimpl->_registerTarget; }
    std::string ProductFileData::registerRelease() const { return _pimpl->_registerRelease; }
    std::string ProductFileData::registerFlavor()  const { return _pimpl->_registerFlavor; }

    std::string ProductFileData::updaterepokey() const { return _pimpl->_updaterepokey; }

    const ProductFileData::Upgrades & ProductFileData::upgrades() const { return _pimpl->_upgrades; }

    std::ostream & operator<<( std::ostream & str, const ProductFileData & obj )
    {
      str << str::form( "|product|%s|%s|%s|%s|",
                        obj.name().c_str(),
                        obj.edition().c_str(),
                        obj.arch().c_str(),
                        obj.vendor().c_str() );
      if ( ! obj.upgrades().empty() )
      {
        for_( it, obj.upgrades().begin(), obj.upgrades().end() )
          str << endl << "    " << *it;
      }
      return str;
    }

    std::ostream & operator<<( std::ostream & str, const ProductFileData::Upgrade & obj )
    {
      str << str::form( "|upgrade|%s|%s|%s|%s|%s|",
                        obj.name().c_str(),
                        obj.repository().c_str(),
                        obj.product().c_str(),
                        obj.status().c_str(),
                        (obj.notify() ? "notify" : "noNotify") );
      return str;
    }
    /////////////////////////////////////////////////////////////////
    //
    // class ProductFileReader
    //
    /////////////////////////////////////////////////////////////////

    struct ProductNode : public xml::ParseDef
    {
      ProductNode( ProductFileData::Impl & pdata_r )
        : ParseDef( "product", MANDTAORY )
        , _pdata( pdata_r )
      {
        (*this)
            ("vendor",        OPTIONAL,   xml::parseDefAssign( _pdata._vendor ) )
            ("name",          MANDTAORY,  xml::parseDefAssign( _pdata._name ) )
            ("version",       MANDTAORY,  xml::parseDefAssign( _version ) )
            ("release",       MANDTAORY,  xml::parseDefAssign( _release ) )
            ("arch",          MANDTAORY,  xml::parseDefAssign( _pdata._arch ) )
            ("shortsummary",  OPTIONAL,   xml::parseDefAssign( _pdata._shortName ) )
            ("summary",       MULTIPLE_OPTIONAL, xml::parseDefAssign( _ttext )( "lang", _tlocale )
					  >>bind( &ProductNode::doneLocalizedDefault, this, _1, boost::ref(_pdata._summary) ))
            ("productline",   OPTIONAL,   xml::parseDefAssign( _pdata._productline ) )
            ("register",      OPTIONAL)
            ("updaterepokey", OPTIONAL,   xml::parseDefAssign( _pdata._updaterepokey ) )
            ("upgrades",      OPTIONAL)
            ;

        (*this)["register"]
            ("target",        OPTIONAL,   xml::parseDefAssign( _pdata._registerTarget ) )
            ("release",       OPTIONAL,   xml::parseDefAssign( _pdata._registerRelease ) )
            ("flavor",        OPTIONAL,   xml::parseDefAssign( _pdata._registerFlavor ) )
            ;

        (*this)["upgrades"]
            ("upgrade",       MULTIPLE_OPTIONAL, xml::parseDefAssign()
                                                 >> bind( &ProductNode::doneUpgrade, this, _1 ))
            ;

        (*this)["upgrades"]["upgrade"]
            ("name",          OPTIONAL,   xml::parseDefAssign( _upgrade._name ) )
            ("summary",       OPTIONAL,   xml::parseDefAssign( _upgrade._summary ) )
            ("repository",    OPTIONAL,   xml::parseDefAssign( _upgrade._repository ) )
            ("product",       OPTIONAL,   xml::parseDefAssign( _upgrade._product ) )
            ("notify",        OPTIONAL,   xml::parseDefAssign( _upgrade._notify ) )
            ("status",        OPTIONAL,   xml::parseDefAssign( _upgrade._status ) )
            ;

        // </product> callback to build edition.
        setConsumer( xml::parseDefAssign() >> bind( &ProductNode::done, this, _1 ) );
        // xml::ParseDef::_debug = true;
      }

      /** collect _upgrade */
      void doneUpgrade( const xml::Node & _node )
      {
        ProductFileData::Upgrade cdata( new ProductFileData::Upgrade::Impl( _upgrade ) );
        _pdata._upgrades.push_back( cdata );
        _upgrade = ProductFileData::Upgrade::Impl();
      }
      /** collect localized data */
      void doneLocalizedDefault( const xml::Node & _node, std::string & store_r )
      {
	// take 1st or default
	if ( store_r.empty() || _tlocale.empty() )
	  store_r = _ttext;
      }

      /** finaly */
      void done( const xml::Node & _node )
      {
        _pdata._edition = Edition( _version, _release );
      }

      private:
        ProductFileData::Impl & _pdata;

        std::string             _version;
        std::string             _release;

	std::string             _ttext;
	std::string             _tlocale;

        ProductFileData::Upgrade::Impl _upgrade;
    };

    bool ProductFileReader::parse( const InputStream & input_r ) const
    {
      MIL << "+++" << input_r << endl;
      bool ret = true;

      ProductFileData::Impl * pdataImpl = 0;
      ProductFileData pdata( (pdataImpl = new ProductFileData::Impl) );

      try
      {
        xml::Reader reader( input_r );
        ProductNode rootNode( *pdataImpl );
        rootNode.take( reader );

      }
      catch ( const Exception & err )
      {
        // parse error
        ERR << err << endl;
        ERR << "---" << ret << " - " << input_r << endl;
        return ret;
      }

      if ( _consumer )
      {
        ret = _consumer( pdata );
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
        WAR << "scanFile " << PathInfo( file_r ) << " is not a file." << endl;
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
