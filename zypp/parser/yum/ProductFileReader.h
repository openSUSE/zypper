/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_PARSER_YUM_PRODUCTFILEREADER_H_
#define ZYPP_PARSER_YUM_PRODUCTFILEREADER_H_

#include "zypp/base/Function.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/yum/FileReaderBase.h"
#include "zypp/data/ResolvableData.h"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * Reader of products.xml file conforming to RNC definition located
   * in zypp/parser/yum/schema/products.rnc
   * 
   * \see zypp::data::Product
   * \see zypp::parser::xml::Reader
   */
  class ProductFileReader : FileReaderBase
  {
  public:

    /**
     * Consumer callback definition. Function which will process the read
     * data must be of this type.
     */
    typedef function<bool(const data::Product_Ptr &)> ProcessProduct;

    /**
     * CTOR. Creates also \ref xml::Reader and starts reading.
     *
     * \param products_file products.xml file to read.
     * \param callback Function which will process read data.
     */
    ProductFileReader(const Pathname & products_file, ProcessProduct callback);

  private:

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


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*ZYPP_PARSER_YUM_PRODUCTFILEREADER_H_*/
