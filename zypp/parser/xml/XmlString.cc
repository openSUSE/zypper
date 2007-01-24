/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml/XmlString.cc
 *
*/

#include <iostream>

#include "zypp/parser/xml/XmlString.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : XmlString::XmlString
    //	METHOD TYPE : Constructor
    //
    XmlString::XmlString( const xmlChar *const xmlstr_r,
                          OnDelete ondelete_r )
    {
      if ( xmlstr_r )
        {
          if ( ondelete_r == FREE )
            _xmlstr.reset( xmlstr_r, Deleter() );
          else
            _xmlstr.reset( xmlstr_r, NullDeleter() );
        }
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const XmlString & obj )
    {
      if ( obj )
        return str << obj.c_str();
      return str << "NULL";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
