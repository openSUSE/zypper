/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml/Reader.h
 *
*/
#ifndef ZYPP_PARSER_XML_READER_H
#define ZYPP_PARSER_XML_READER_H

#include <iosfwd>

#include "zypp/base/NonCopyable.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/Function.h"

#include "zypp/parser/xml/Node.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Validate
    //
    /** xmlTextReader document validation.
     * \todo Implement RelaxNG and W3C XSD
     **/
    struct Validate
    {
      static Validate none()
      { return Validate(); }
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Reader
    //
    /** xmlTextReader based interface to iterate xml streams.
     *
     * \code
     * // Consume a node.
     * bool consumeNode( XML::Reader & reader_r )
     * {
     *   DBG << *reader_r << endl;
     *   return true;
     * }
     *
     * // Consume all nodes (omitting attributes)
     * void example()
     * {
     *   try
     *     {
     *       XML::Reader reader( "/Local/repodata/repomd.xml" );
     *       reader.foreachNode( consumeNode );
     *     }
     *   catch ( const Exception & )
     *     { ; } // parse error
     * }
     * \endcode
     *
     * \code
     * // Consume a node.
     * bool consumeNodeAndAttribute( XML::Reader & reader_r )
     * {
     *   consumeNode( reader_r );
     *   return reader_r.foreachNodeAttribute( consumeNode );
     * }
     *
     * // Consume all nodes and thair attributes.
     * void example()
     * {
     *   Pathname repodata( "/Local/repodata/repomd.xml" );
     *   try
     *     {
     *       XML::Reader reader( "/Local/repodata/repomd.xml" );
     *       reader.foreachNode( consumeNodeAndAttribute );
     *       // or:
     *       // reader.foreachNodeOrAttribute( consumeNode )
     *     }
     *   catch ( const Exception & )
     *     { ; } // parse error
     * }
     * \endcode
     **/
    class Reader : private zypp::base::NonCopyable
    {
    public:
      /** Ctor. Setup xmlTextReader and advance to the 1st Node. */
      Reader( const InputStream & stream_r,
              const Validate & validate_r = Validate::none() );

      /** Dtor. */
      ~Reader();

    public:

      /**
       *  If the curent node is not empty, advances the reader to the next
       *  node, and returns the value
       *
       * \note if the node has a xml subtree you will probably jump to that node
       * and get a empty text value back. Use it only if you are sure the node
       * has no XML subtree.
       */
      XmlString nodeText();

      /** */
      bool nextNode();

      /** */
      bool nextNodeAttribute();

      /** */
      bool nextNodeOrAttribute()
      { return( nextNodeAttribute() || nextNode() ); }

      /** */
      bool atEnd() const
      { return( _node.readState() == XML_TEXTREADER_MODE_CLOSED ); }

      /** */
      const Node & operator*() const
      { return _node; }

      /** */
      const Node * operator->() const
      { return &_node; }

    public:
      /** */
      typedef function<bool( Reader & )> ProcessNode;

      /** */
      bool foreachNode( ProcessNode fnc_r )
      {
        if ( _node.isAttribute() )
          nextNode();
        for ( ; ! atEnd(); nextNode() )
          {
            if ( ! fnc_r( *this ) )
              return false;
          }
        return true;
      }

      /** */
      bool foreachNodeAttribute( ProcessNode fnc_r )
      {
        if ( _node.isAttribute() && ! fnc_r( *this ) )
          return false;
        while( nextNodeAttribute() )
          {
            if ( ! fnc_r( *this ) )
              return false;
          }
        return true;
      }

      /** */
      bool foreachNodeOrAttribute( ProcessNode fnc_r )
      {
        for ( ; ! atEnd(); nextNodeOrAttribute() )
          {
            if ( ! fnc_r( *this ) )
              return false;
          }
        return true;
      }

    public:
      /** */
      bool seekToNode( int depth_r, const std::string & name_r );

      /** */
      bool seekToEndNode( int depth_r, const std::string & name_r );

    private:
      void close();

    private:
      InputStream      _stream;
      xmlTextReaderPtr _reader;
      Node             _node;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_XML_READER_H
