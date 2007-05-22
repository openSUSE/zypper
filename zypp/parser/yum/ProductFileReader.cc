/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/Logger.h"
#include "zypp/parser/yum/ProductFileReader.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  ProductFileReader::ProductFileReader(const Pathname & product_file, ProcessProduct callback)
      : _callback(callback)
  {
    Reader reader(product_file);
    MIL << "Reading " << product_file << endl;
    reader.foreachNode(bind(&ProductFileReader::consumeNode, this, _1));
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

  bool ProductFileReader::consumeNode(Reader & reader_r)
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
        // TODO what's this?
        // _product->? = reader_r->getAttribute("type").asString();
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

  data::Product_Ptr ProductFileReader::handoutProduct()
  {
    data::Product_Ptr ret;
    ret.swap(_product);
    return ret;
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
