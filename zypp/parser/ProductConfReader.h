/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/ProductConfReader.h
 *
*/
#ifndef ZYPP_PARSER_PRODUCTCONFREADER_H
#define ZYPP_PARSER_PRODUCTCONFREADER_H

#include <iosfwd>

#include "zypp/base/Function.h"
#include "zypp/base/InputStream.h"

#include "zypp/ZConfig.h"

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
    //	CLASS NAME : ProductConfData
    //
    /** Data returned by \ref ProductConfReader
     * \see \ref ProductConfReader
    */
    class ProductConfData
    {
      public:
        /** Product name */
        IdString name() const
        { return _name; }

        /** Product edition */
        Edition edition() const
        { return _edition; }

        /** Product arch */
        Arch arch() const
        { return _arch.empty() ? ZConfig::instance().systemArchitecture() : _arch; }

        /** Product distName (defaults to \ref name)*/
        IdString  distName() const
        { return _distName.empty() ? _name : _distName; }

        /** Product distEdition (defaults to \ref edition) */
        Edition   distEdition() const
        { return _distEdition.empty() ? _edition : _distEdition; }

      public:
        IdString  _name;
        Edition   _edition;
        Arch      _arch;

        IdString  _distName;
        Edition   _distEdition;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates  ProductConfData Stream output */
    std::ostream & operator<<( std::ostream & str, const ProductConfData & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProductConfReader
    //
    /** ProductConfReader (registration helper)
     * \code
     * - The file may contain zero or more product sections.
     * Required format for the section header is:
     *
     *         [the products name version architecture]
     *
     * Last word is architecture,
     * one but last word is version,
     * the remaining words are the name.
     *
     * Less than 3 words is an error, section is ignored.
     *
     * If architecture is "%arch" it is substituted by the sytems architecture.
     *
     * - In each section you may define
     *
     *         distproduct = some dist name
     *         distversion = 1.2.3-4.5
     * \endcode
    */
    class ProductConfReader
    {
    public:
      /** Callback being invoked for each parsed section.
       * Return \c false to stop parsing.
      */
      typedef function<bool( const ProductConfData & )> Consumer;

    public:
      ProductConfReader()
      {}

      ProductConfReader( const Consumer & consumer_r )
      : _consumer( consumer_r )
      {}

      ProductConfReader( const Consumer & consumer_r, const InputStream & input_r )
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
      /** Parse all <tt>*.prod</tt> files in \c dir_r and call \c consumer_r
       * for each parsed section.
       *
       * Returns \c false if the \c _consumer requested to stop parsing.
       */
      static bool scanDir( const Consumer & consumer_r, const Pathname & dir_r );

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
#endif // ZYPP_PARSER_PRODUCTCONFREADER_H
