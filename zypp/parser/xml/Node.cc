/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml/Reader.cc
 *
*/
#include <libxml/xmlreader.h>
#include <libxml/xmlerror.h>

#include <iostream>

#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Exception.h"

#include "zypp/parser/xml/Node.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    xmlTextReaderPtr const Node::_no_reader = 0;

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Node::Node
    //	METHOD TYPE : Constructor
    //
    Node::Node()
    : _reader( _no_reader )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Node::Node
    //	METHOD TYPE : Constructor
    //
    Node::Node( xmlTextReaderPtr const & reader_r )
    : _reader( reader_r )
    {}

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Node & obj )
    {
      if ( ! obj )
        return str << "NoNode" << endl;
#define X(m) obj.m()
      str << X(depth) << ":" <<  std::string( X(depth), ' ') << X(nodeType) << " <";
      if ( X(nodeType) == XML_READER_TYPE_NONE )
        {
          return str << '[' << X(readState) << "]>";
        }
      if ( obj.prefix() )
        str << X(prefix) << ':';
      str << X(localName) << "> ";
      if ( X(hasAttributes) )
        str << " [attr " << X(attributeCount);
      else
        str << " [noattr";
      if ( X(isEmptyElement) )
        str << "|empty";
      str << ']';
      if ( X(hasValue) )
        str << " {" << X(value) << '}';
#undef X
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

