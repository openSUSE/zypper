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
      str << str::form( "|upgrade|%s|",
                        obj.name().c_str() );
      return str;
    }
    /////////////////////////////////////////////////////////////////
    //
    // class ProductFileReader
    //
    /////////////////////////////////////////////////////////////////

    /** Assign types constructible from \c char* */
    template <class _Val>
    struct ConsumeVal : public xml::ParseDefConsume
    {
      ConsumeVal( _Val & value_r )
        : _value( &value_r )
      {}

      virtual void text( const xml::Node & node_r )
      {
        *_value = _Val( node_r.value().c_str() );
      }

      private:
      _Val * _value;
    };

    template <class _Val>
    shared_ptr<xml::ParseDefConsume> consumeVal( _Val & value_r )
    { return shared_ptr<xml::ParseDefConsume>( new ConsumeVal<_Val>( value_r ) ); }

    /////////////////////////////////////////////////////////////////

    struct ProductNode : public xml::ParseDef, public xml::ParseDefConsume
    {
      ProductNode( ProductFileData::Impl & pdata_r )
        : ParseDef( "product", MANDTAORY )
        , _pdata( pdata_r )
      {
        (*this)
            ("vendor",        OPTIONAL,  consumeVal( _pdata._vendor ) )
            ("name",          MANDTAORY, consumeVal( _pdata._name ) )
            ("version",       OPTIONAL,  consumeVal( _version ) )
            ("release",       OPTIONAL,  consumeVal( _release ) )
            ("arch",          OPTIONAL,  consumeVal( _pdata._arch ) )
            ("productline",   OPTIONAL,  consumeVal( _pdata._productline ) )
            ("register",      OPTIONAL)
            ("updaterepokey", OPTIONAL,  consumeVal( _pdata._updaterepokey ) )
            ("upgrades",      OPTIONAL)
            ;

        (*this)["register"]
            ("target",        OPTIONAL,  consumeVal( _pdata._registerTarget ) )
            ("release",       OPTIONAL,  consumeVal( _pdata._registerRelease ) )
            ;

        (*this)["upgrades"]
            ("upgrade",       MULTIPLE_OPTIONAL)
            ;

        (*this)["upgrades"]["upgrade"]
            ("name",          OPTIONAL,  consumeVal( _upgrade._name ) )
            ("summary",       OPTIONAL,  consumeVal( _upgrade._summary ) )
            ("repository",    OPTIONAL,  consumeVal( _upgrade._product ) )
            ("notify",        OPTIONAL)  // need consume to bool!
            ("status",        OPTIONAL,  consumeVal( _upgrade._status ) )
            ;

        // not a clean way to collect the END_ELEMENT calls, but
        // works for this case. NEEDS CLEANUP!
        setConsumer( *this );
        (*this)["upgrades"].setConsumer( *this );
       }

       virtual void done ( const xml::Node & _node )
       {
         //SEC << "DONE.... " << _node.localName() << endl;
         if ( _node.localName() == name() )
         {
           // this END node
           _pdata._edition = Edition( _version, _release );
         }
         else if ( _node.localName() == "upgrade" )
         {
           // collect upgrade
           ProductFileData::Upgrade cdata( new ProductFileData::Upgrade::Impl( _upgrade ) );
           _pdata._upgrades.push_back( cdata );
         }
       }

       ProductFileData::Impl & _pdata;

       std::string             _version;
       std::string             _release;

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
        //xml::ParseDef::_debug = true;
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
