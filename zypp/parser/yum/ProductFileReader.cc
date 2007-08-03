/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/Logger.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/ZConfig.h"

#include "zypp/parser/yum/FileReaderBaseImpl.h"
#include "zypp/parser/yum/ProductFileReader.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser::yum"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : ProductFileReader::Impl
  //
  class ProductFileReader::Impl : public BaseImpl
  {
  public:
    Impl(const Pathname & products_file,
         const ProcessProduct & callback);

    /**
     * Callback provided to the XML reader.
     *
     * \param  the xml reader object reading the file
     * \return true to tell the reader to continue, false to tell it to stop
     *
     * \see PrimaryFileReader::consumeNode(xml::Reader)
     */
    bool consumeNode(xml::Reader & reader_r);

    /**
     * Creates a new \ref data::Product_Ptr, swaps its contents with \ref _product
     * and returns it. Used to hand-out the data object to its consumer
     * (a \ref ProcessProduct function) after it has been read.
     */
    data::Product_Ptr handoutProduct();

  private:
    /**
     * Callback for processing product metadata.
     */
    ProcessProduct _callback;

    /**
     * Pointer to the \ref zypp::data::Product object for storing the product
     * metada.
     */
    data::Product_Ptr _product;
  };
  ///////////////////////////////////////////////////////////////////

  ProductFileReader::Impl::Impl(
      const Pathname & products_file,
      const ProcessProduct & callback)
    :
      _callback(callback)
  {
    Reader reader(products_file);
    MIL << "Reading " << products_file << endl;
    reader.foreachNode(bind(&ProductFileReader::Impl::consumeNode, this, _1));
  }

  // --------------------------------------------------------------------------

  /*
   * xpath and multiplicity of processed nodes are included in the code
   * for convenience:
   *
   * // xpath: <xpath> (?|*|+)
   *
   * if multiplicity is ommited, then the node has multiplicity 'one'.
   */

  // --------------------------------------------------------------------------

  bool ProductFileReader::Impl::consumeNode(Reader & reader_r)
  {
    // dependency block nodes
    if (_product && consumeDependency(reader_r, _product->deps))
      return true;

    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: /products/product
      if (reader_r->name() == "product")
      {
        tag(tag_product); // just for the case of reuse somewhere/sometimes
        _product = new data::Product;
        return true;
      }

      // xpath: /products/product/name
      if (reader_r->name() == "name")
      {
        _product->name = reader_r.nodeText().asString();
        // product type (base, add-on)
        _product->type = reader_r->getAttribute("type").asString();
        return true;
      }

      // xpath: /products/product/vendor
      if (reader_r->name() == "vendor")
      {
        _product->vendor = reader_r.nodeText().asString();
      }

      // xpath: /products/product/version
      if (reader_r->name() == "version")
      {
        _product->edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());
      }

      // xpath: /products/product/displayname (+)
      if (reader_r->name() == "displayname")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _product->longName.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: /products/product/shortname (*)
      if (reader_r->name() == "shortname")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _product->shortName.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: /products/product/description (+)
      if (reader_r->name() == "description")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _product->description.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: /products/product/distribution-name (+)
      if (reader_r->name() == "distribution-name")
      {
        _product->distributionName = reader_r.nodeText().asString();
        return true;
      }

      // xpath: /products/product/distribution-edition (+)
      if (reader_r->name() == "distribution-edition")
      {
        _product->distributionEdition = reader_r.nodeText().asString();
        return true;
      }

      // xpath: /products/product/release-notes-url (+)
      if (reader_r->name() == "release-notes-url")
      {
        string value = reader_r.nodeText().asString();

        for( std::string::size_type pos = value.find("%a");
            pos != std::string::npos;
            pos = value.find("%a") )
        {
          value.replace( pos, 2, ZConfig::instance().systemArchitecture().asString() );
        }
        try
        {
          _product->releasenotesUrl = value;
        }
        catch( const Exception & excpt_r )
        {
          WAR << "Malformed url ignored: '" << value << "' " << excpt_r.asString() << endl;
        }
        return true;
      }

      // xpath: /products/product/update-url (*)
      if (reader_r->name() == "update-url")
      {
        string value = reader_r.nodeText().asString();

        try
        {
          _product->updateUrls.push_back(Url(value));
        }
        catch( const Exception & excpt_r )
        {
          WAR << "Malformed url ignored: '" << value << "' " << excpt_r.asString() << endl;
        }
        return true;
      }

      // xpath: /products/product/extra-url (*)
      if (reader_r->name() == "extra-url")
      {
        string value = reader_r.nodeText().asString();

        try
        {
          _product->extraUrls.push_back(Url(value));
        }
        catch( const Exception & excpt_r )
        {
          WAR << "Malformed url ignored: '" << value << "' " << excpt_r.asString() << endl;
        }
        return true;
      }

      // xpath: /products/product/optional-url (*)
      if (reader_r->name() == "optional-url")
      {
        string value = reader_r.nodeText().asString();

        try
        {
          _product->optionalUrls.push_back(Url(value));
        }
        catch( const Exception & excpt_r )
        {
          WAR << "Malformed url ignored: '" << value << "' " << excpt_r.asString() << endl;
        }
        return true;
      }
    }

    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /products/product
      if (reader_r->name() == "product")
      {
      	if (_callback)
      	  _callback(handoutProduct());

        toParentTag(); // just for the case of reuse somewhere/sometimes

      	return true;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  data::Product_Ptr ProductFileReader::Impl::handoutProduct()
  {
    data::Product_Ptr ret;
    ret.swap(_product);
    return ret;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : ProductFileReader
  //
  ///////////////////////////////////////////////////////////////////

  ProductFileReader::ProductFileReader(const Pathname & product_file, ProcessProduct callback)
      : _pimpl(new ProductFileReader::Impl(product_file, callback))
  {}


  ProductFileReader::~ProductFileReader()
  {}


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
