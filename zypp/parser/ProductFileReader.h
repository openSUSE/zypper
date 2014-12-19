/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/ProductFileReader.h
 *
*/
#ifndef ZYPP_PARSER_PRODUCTSDREADER_H
#define ZYPP_PARSER_PRODUCTSDREADER_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Function.h"
#include "zypp/base/InputStream.h"

#include "zypp/Pathname.h"
#include "zypp/IdString.h"
#include "zypp/Edition.h"
#include "zypp/Arch.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProductFileData
    //
    /** Data returned by \ref ProductFileReader
     * \see \ref ProductFileReader
    */
    class ProductFileData
    {
      public:
        class Impl;
        /** Ctor takes ownership of \c allocated_r. */
        ProductFileData( Impl * allocated_r = 0 );

        /** Whether this is an empty object without valid data. */
        bool empty() const
        { return name().empty(); }

      public:
        IdString    vendor()  const;
        IdString    name()    const;
        Edition     edition() const;
        Arch        arch()    const;

	std::string shortName()	const;
	std::string summary()	const;

      public:
        std::string productline()     const;
        std::string registerTarget()  const;
        std::string registerRelease() const;
        std::string registerFlavor()  const;

      public:
        std::string updaterepokey() const;

      public:
        ///////////////////////////////////////////////////////////////////
        /** \see http://en.opensuse.org/Product_Management/Code11/Upgrade */
        struct Upgrade
        {
          public:
            class Impl;
            /** Ctor takes ownership of \c allocated_r. */
            Upgrade( Impl * allocated_r = 0 );

          public:
            std::string name()    const;
            std::string summary() const;
            std::string repository() const;
            std::string product() const;
            bool        notify()  const;
            std::string status()  const;

          private:
            RWCOW_pointer<Impl> _pimpl;
        };
        ///////////////////////////////////////////////////////////////////

        typedef std::vector<Upgrade> Upgrades;
        const Upgrades & upgrades() const;

      private:
        RWCOW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates  ProductFileData Stream output */
    std::ostream & operator<<( std::ostream & str, const ProductFileData & obj );

    /** \relates  ProductFileData::Upgrade Stream output */
    std::ostream & operator<<( std::ostream & str, const ProductFileData::Upgrade & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProductFileReader
    //
    /** Parser for /etc/products.d enries (just relevant entires).
     *
     * \code
     * #include "zypp/base/Functional.h" // functor::getAll
     *
     * std::vector<ProductFileData> result;
     * ProductFileReader::scanDir( functor::getAll( std::back_inserter( result ) ),
     *                             "/etc/products.d" );
     * \endcode
     */
    class ProductFileReader
    {
    public:
      /** Callback being invoked for each \ref ProductFileData parsed.
       * Return \c false to stop parsing.
       */
      typedef function<bool( const ProductFileData & )> Consumer;

    public:
      ProductFileReader()
      {}

      ProductFileReader( const Consumer & consumer_r )
      : _consumer( consumer_r )
      {}

      ProductFileReader( const Consumer & consumer_r, const InputStream & input_r )
      : _consumer( consumer_r )
      { parse( input_r ); }

    public:
      const Consumer & consumer() const
      { return _consumer; }

      void setConsumer( const Consumer & consumer_r )
      { _consumer = consumer_r; }

    public:
      /** Parse the input stream and call \c _consumer for each
       * parsed section.
       *
       * Returns \c false if the \c _consumer requested to stop parsing.
       */
      bool parse( const InputStream & input_r = InputStream() ) const;

    public:
      /** Parse all files (no symlinks) in \c dir_r and call \c consumer_r
       * for each \ref ProductFileData parsed.
       *
       * Returns \c false if the \c _consumer requested to stop parsing.
       */
      static bool scanDir( const Consumer & consumer_r, const Pathname & dir_r );

      /** Parse one file (or symlink) and return the  \ref ProductFileData parsed.
       */
      static ProductFileData scanFile( const Pathname & file_r );

   private:
      Consumer _consumer;
    };
    ///////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_PRODUCTSDREADER_H
