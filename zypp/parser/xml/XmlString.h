/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml/XmlString.h
 *
*/
#ifndef ZYPP_PARSER_XML_XMLSTRING_H
#define ZYPP_PARSER_XML_XMLSTRING_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"

#include "zypp/parser/xml/libxmlfwd.h"
#include "zypp/parser/xml/XmlEscape.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XmlString
    //
    /** <tt>xmlChar *</tt> wrapper.
     *
     * Common handling of <tt>xmlChar *</tt> that do or do not need to
     * be freed. If the wraped <tt>xmlChar *</tt> needs to be freed by
     * calling \c xmlFree, pass \c FREE as 2nd argument to the ctor.
     **/
    class XmlString
    {
      /** shared_ptr custom deleter calling \c xmlFree. */
      struct Deleter
      {
        void operator()( const xmlChar * xmlstr_r ) const
        { xmlFree( (void*)(xmlstr_r) ); }
      };

    public:

      /** Dtor policy. */
      enum OnDelete { NOFREE, FREE };

      /** Ctor from xmlChar.
       * Pass \c FREE as 2nd arg if \c xmlFree needs to be called on destruction.
      */
      XmlString( const xmlChar *const xmlstr_r = NULL,
                 OnDelete ondelete_r           = NOFREE );

      /** Access the <tt>xmlChar *</tt>. */
      const xmlChar * get() const
      {
        if ( ! _xmlstr )
          return NULL;
        return &(*_xmlstr);
      }

      /** Implicit conversion to <tt>xmlChar *</tt>. */
      operator const xmlChar * () const
      { return get(); }

      /** Explicit conversion to <tt>const char *</tt>. */
      const char * c_str() const
      { return reinterpret_cast<const char *const>(get()); }

      /** Explicit conversion to <tt>std::string</tt>. */
      std::string asString() const
      {
        if ( ! _xmlstr )
          return std::string();
        return c_str();
      }

      bool operator==( const std::string & rhs ) const
      { return( rhs == c_str() ); }

      bool operator!=( const std::string & rhs ) const
      { return( rhs != c_str() ); }

      bool operator==( const char *const rhs ) const
      { return( asString() == rhs ); }

      bool operator!=( const char *const rhs ) const
      { return( asString() != rhs ); }

      bool operator==( const XmlString & rhs ) const
      { return( asString() == rhs.c_str() ); }

      bool operator!=( const XmlString & rhs ) const
      { return( asString() != rhs.c_str() ); }

    private:
      /** Wraps the <tt>xmlChar *</tt>.
       * The appropriate custom deleter is set by the ctor. */
      shared_ptr<const xmlChar> _xmlstr;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates XmlString Stream output. */
    std::ostream & operator<<( std::ostream & str, const XmlString & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_XML_XMLSTRING_H
