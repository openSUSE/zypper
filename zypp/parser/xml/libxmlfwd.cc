/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml/libxmlfwd.cc
 *
*/

#include <iostream>

#include "zypp/parser/xml/libxmlfwd.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const ReadState & obj )
    {
      switch ( obj )
        {
#define X(T) case XML_TEXTREADER_MODE_##T: return str << #T
          X(INITIAL);
          X(INTERACTIVE);
          X(ERROR);
          X(EOF);
          X(CLOSED);
          X(READING);
#undef X
        }
      return str << "UNKNOWN_READ_STATE";
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const NodeType & obj )
    {
      switch ( obj )
        {
#define X(T) case XML_READER_TYPE_##T: return str << #T
          X(NONE);
          X(ELEMENT);
          X(ATTRIBUTE);
          X(TEXT);
          X(CDATA);
          X(ENTITY_REFERENCE);
          X(ENTITY);
          X(PROCESSING_INSTRUCTION);
          X(COMMENT);
          X(DOCUMENT);
          X(DOCUMENT_TYPE);
          X(DOCUMENT_FRAGMENT);
          X(NOTATION);
          X(WHITESPACE);
          X(SIGNIFICANT_WHITESPACE);
          X(END_ELEMENT);
          X(END_ENTITY);
          X(XML_DECLARATION);
#undef X
        }
      return str << "UNKNOWN_NODE_TYPE";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
