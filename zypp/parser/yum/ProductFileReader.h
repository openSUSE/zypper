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

#include "zypp/parser/yum/FileReaderBase.h"

namespace zypp
{

  namespace data
  {
    class Product;
    DEFINE_PTR_TYPE(Product);
  } // ns data


  namespace parser
  {
    namespace yum
    {


  /**
   * Reader of products.xml file conforming to RNC definition located
   * at zypp/parser/yum/schema/products.rnc.
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
    
    /**
     * DTOR.
     */
    ~ProductFileReader();

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*ZYPP_PARSER_YUM_PRODUCTFILEREADER_H_*/
