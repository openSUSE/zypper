/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/xml/Reader.cc
 *
*/

extern "C"
{
#include <libxml/xmlreader.h>
#include <libxml/xmlerror.h>
}

#include <iostream>

#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Exception.h"

#include "zypp/xml/Node.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    xmlTextReaderPtr const Node::_no_reader = 0;

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
      if ( obj.prefix() )
        str << X(prefix) << '|';
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

