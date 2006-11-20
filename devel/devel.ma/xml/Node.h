/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/xml/Node.h
 *
*/
#ifndef ZYPP_XML_NODE_H
#define ZYPP_XML_NODE_H

#include <iosfwd>

#include "zypp/base/SafeBool.h"

#include "zypp/xml/XmlString.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Node
    //
    /** xmlTextReader based interface to \ref Reader's current node.
     *
     **/
    class Node : private base::SafeBool<Node>
    {
    public:
      /** Default ctor. */
      Node()
      : _reader( _no_reader )
      {}

      /** Ctor referencing a \ref Reader. */
      Node( xmlTextReaderPtr const & reader_r )
      : _reader( reader_r )
      {}

      /** Validate Node in a boolean context. \see \ref SafeBool. */
      using base::SafeBool<Node>::operator bool_type;

    public:
      /** Provides the number of attributes of the current node. */
      int attributeCount() const
      { return xmlTextReaderAttributeCount( _reader ); }

      /** The base URI of the node. */
      XmlString baseUri() const
      { return xmlTextReaderConstBaseUri( _reader ); }

      /** Provide the column number of the current parsing point. */
      int columnNumber() const
      { return xmlTextReaderGetParserColumnNumber( _reader ); }

      /** The depth of the node in the tree. */
      int depth() const
      { return xmlTextReaderDepth( _reader ); }

      /** Determine the encoding of the document being read. */
      XmlString encoding() const
      { return xmlTextReaderConstEncoding( _reader ); }

      /** Provides the value of the attribute with the specified
       * qualified name. */
      XmlString getAttribute( const char * name_r ) const
      { return XmlString( xmlTextReaderGetAttribute( _reader, reinterpret_cast<const xmlChar *>(name_r) ),
                          XmlString::FREE ); }

      /** Provides the value of the attribute with the specified
       * index relative to the containing element. */
      XmlString getAttributeNo( int no_r ) const
      { return XmlString( xmlTextReaderGetAttributeNo( _reader, no_r ), XmlString::FREE ); }

      /** Whether the node has attributes. */
      int hasAttributes() const
      { return xmlTextReaderHasAttributes( _reader ); }

      /** Whether the node can have a text value. */
      int hasValue() const
      { return xmlTextReaderHasValue( _reader ); }

      /** Whether this is an Attribute node. */
      bool isAttribute() const
      { return( nodeType() == XML_READER_TYPE_ATTRIBUTE ); }

      /** Whether an Attribute node was generated from the default value
       *  defined in the DTD or schema. */
      int isDefault() const
      { return xmlTextReaderIsDefault( _reader ); }

      /** Check if the current node is empty. */
      int isEmptyElement() const
      { return xmlTextReaderIsEmptyElement( _reader ); }

      /** Determine whether the current node is a namespace declaration
       *  rather than a regular attribute.*/
      int isNamespaceDecl() const
      { return xmlTextReaderIsNamespaceDecl( _reader ); }

      /** Provide the line number of the current parsing point. */
      int lineNumber() const
      { return xmlTextReaderGetParserLineNumber( _reader ); }

      /** The local name of the node. */
      XmlString localName() const
      { return xmlTextReaderConstLocalName( _reader ); }

      /** The qualified name of the node, equal to Prefix :LocalName. */
      XmlString name() const
      { return xmlTextReaderConstName( _reader ); }

      /** The URI defining the namespace associated with the node. */
      XmlString namespaceUri() const
      { return xmlTextReaderConstNamespaceUri( _reader ); }

      /** Get the node type of the current node. */
      NodeType nodeType() const
      { return (NodeType)xmlTextReaderNodeType( _reader ); }

      /** A shorthand reference to the namespace associated with the node. */
      XmlString prefix() const
      { return xmlTextReaderConstPrefix( _reader ); }

      /** The quotation mark character used to enclose the value of an attribute.
       * \return \c " or \c ' or -1 in case of error. */
      int quoteChar() const
      { return xmlTextReaderQuoteChar( _reader ); }

      /** Gets the read state of the reader. */
      ReadState readState() const
      { return (ReadState)xmlTextReaderReadState( _reader ); }

      /** Provides the text value of the node if present. */
      XmlString value() const
      { return xmlTextReaderConstValue( _reader ); }

      /** The xml:lang scope within which the node resides. */
      XmlString xmlLang() const
      { return xmlTextReaderConstXmlLang( _reader ); }

      /** Determine the XML version of the document being read. */
      XmlString xmlVersion() const
      { return xmlTextReaderConstXmlVersion( _reader ); }

    private:
      /** NULL Reader referenced by default ctor. */
      static xmlTextReaderPtr const _no_reader;
      /** Reference to the \ref Reader. */
      xmlTextReaderPtr const & _reader;

    private:
      friend base::SafeBool<Node>::operator bool_type() const;
      /** \ref SafeBool test. */
      bool boolTest() const { return _reader; }
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Node Stream output. */
    std::ostream & operator<<( std::ostream & str, const Node & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_XML_NODE_H
