/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/parser/xml/Parse.h
 *
*/
#ifndef ZYPP_PARSER_XML_PARSE_H
#define ZYPP_PARSER_XML_PARSE_H

#include <iosfwd>

#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/xml/ParseDef.h"
#include "zypp/parser/xml/ParseDefConsume.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    /** Parse xml \c input_r and store data in \c data_r.
     *
     * \c TData must be defaultconstructible and assignable.
     *
     * \c TData::RootNode must be a \ref xml::ParseDef constructible
     * from \c TData&.
     *
     * \throws ParseDefException on parse errors.
     *
     * To parse a xml file like this:
     * \code
     * <test>
     *   <setup attr="13">value</setup>
     *   <list name="A"/>
     *   <list name="b"/>
     * </test>
     * \endcode
     *
     * You need something like this:
     * \code
     *  struct XmlData
     *  {
     *    // data
     *    unsigned              attr;
     *    std::string           value;
     *    std:list<std::string> names;
     *
     *    public:
     *      // Convenience parsing to *this.
     *      void parse( const Pathname & path_r )
     *      { xml::rnParse( path_r, *this ); }
     *
     *    public:
     *      // Parser description
     *      struct RootNode : public xml::ParseDef
     *      {
     *        RootNode( XmlData & data )
     *          : ParseDef( "test", MANDTAORY )
     *          , _data( data )
     *        {
     *          (*this)
     *              ("setup", MANDTAORY,
     *                        xml::parseDefAssign( data.value )
     *                                           ( "attr", data.attr ) )
     *              // Each individual list entry is collected locally
     *              // and appended to the list after the node is done.
     *              ("list",  MULTIPLE_OPTIONAL,
     *                        xml::parseDefAssign( "name", _cname )
     *                                           >> bind( &RootNode::cdone, this, _1 ) )
     *              ;
     *        }
     *
     *        void cdone( const xml::Node & node_r )
     *        {
     *          _data.push_back( _cname );
     *          _cname.clear(); // prepare for next
     *        }
     *
     *        private:
     *          XmlData &   _data; // stored just because notification callbacks are used.
     *          std::string _cname;
     *      };
     *  };
     *
     *  XmlData xmlData;
     *  xmlData.parse( "/tmp/mytest.xml" );
     * \endcode
     */
    template<class TData>
    inline void rnParse( const InputStream & input_r, TData & data_r )
    {
      typedef typename TData::RootNode RootNode;
      TData pdata;

      xml::Reader reader( input_r );
      RootNode rootNode( pdata );
      rootNode.take( reader );

      data_r = pdata;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_XML_PARSEDEF_H
