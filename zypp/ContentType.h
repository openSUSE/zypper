/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ContentType.h
 */
#ifndef ZYPP_CONTENTTYPE_H
#define ZYPP_CONTENTTYPE_H

#include <iosfwd>
#include <string>
#include <stdexcept>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class ContentType
  /// \brief Mime type like \c 'type/subtype' classification of content
  ///
  /// Used e.g. in \ref callback::UserData to describe the kind of
  /// user data passed as \c void* to a callback. Neither type nor
  /// subtype may contain a '/'.
  ///////////////////////////////////////////////////////////////////
  class ContentType
  {
  public:
    /** Default ctor: empty */
    ContentType()
    {}

    /** Ctor taking <tt>"type[/subtype]"</tt>
     * \throws std::invalid_argument if string is malformed
     */
    explicit ContentType( std::string type_r )
    {
      std::string::size_type pos = type_r.find( "/" );
      if ( pos != std::string::npos )
      {
	testAndSet( _subtype, type_r.substr( pos+1 ) );
	type_r.erase( pos );
      }
      testAndSet( _type, std::move(type_r) );
    }

    /** Ctor taking type and subtype
     * \throws std::invalid_argument if string is malformed
     */
    ContentType( std::string type_r, std::string subtype_r )
    {
      testAndSet( _type,    std::move(type_r) );
      testAndSet( _subtype, std::move(subtype_r) );
    }

  public:
    /** Get type */
    const std::string & type() const
    { return _type; }

    /** Set type
     * \throws std::invalid_argument if string is malformed
     */
    void type( std::string type_r )
    { _type = std::move(type_r); }

    /**  Get subtype */
    const std::string & subtype() const
    { return _subtype; }

    /**  Set subtype
     * \throws std::invalid_argument if string is malformed
     */
    void subtype( std::string subtype_r )
    { _subtype = std::move(subtype_r); }

  public:
    /** Whether type and subtype are empty */
    bool empty() const
    { return emptyType() && emptySubtype(); }
    /** Whether type is empty */
    bool emptyType() const
    { return _type.empty(); }
    /** Whether subtype is empty */
    bool emptySubtype() const
    { return _subtype.empty(); }

    /** Validate object in a boolean context: !empty */
    explicit operator bool () const
    { return !empty(); }

    /** String representation <tt>"type[/subtype]"</tt> */
    std::string asString() const
    { std::string ret( type() ); if ( ! emptySubtype() ) { ret += "/"; ret += subtype(); } return ret; }

  private:
    void testAndSet( std::string & var_r, std::string val_r )
    {
      if ( val_r.find_first_of( "/ \t\r\n" ) != std::string::npos )
	throw std::invalid_argument( "ContentType: illegal char in '" + val_r + "'" );
      var_r = std::move(val_r);
    }
  private:
    std::string	_type;
    std::string	_subtype;
  };

  /** \relates ContentType Stream output */
  inline std::ostream & operator<<( std::ostream & str, const ContentType & obj )
  { return str << obj.asString(); }

  /** \relates ContentType */
  inline bool operator==( const ContentType & lhs, const ContentType & rhs )
  { return lhs.type() == rhs.type() && lhs.subtype() == rhs.subtype(); }

  /** \relates ContentType */
  inline bool operator!=( const ContentType & lhs, const ContentType & rhs )
  { return !( lhs == rhs ); }

  /** \relates ContentType */
  inline bool operator<( const ContentType & lhs, const ContentType & rhs )
  { int cmp = lhs.type().compare( rhs.type() ); return cmp < 0 || ( cmp == 0 && lhs.subtype() < rhs.subtype() ); }

  /** \relates ContentType */
  inline bool operator<=( const ContentType & lhs, const ContentType & rhs )
  { return lhs < rhs || lhs == rhs; }

  /** \relates ContentType */
  inline bool operator>( const ContentType & lhs, const ContentType & rhs )
  { return !( lhs <= rhs ); }

  /** \relates ContentType */
  inline bool operator>=( const ContentType & lhs, const ContentType & rhs )
  { return !( lhs < rhs ); }


} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CONTENTTYPE_H
