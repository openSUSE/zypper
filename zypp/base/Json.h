/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Json.h
 *
*/
#ifndef ZYPP_BASE_JSON_H
#define ZYPP_BASE_JSON_H

#include <iosfwd>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>

#include "zypp/base/Easy.h"
#include "zypp/base/String.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace json
  {
    // JSN Keywords
    inline static const std::string & nullJSON()  	{ static const std::string _s( "null" );  return _s; }
    inline static const std::string & trueJSON()  	{ static const std::string _s( "true" );  return _s; }
    inline static const std::string & falseJSON() 	{ static const std::string _s( "false" ); return _s; }

    ///////////////////////////////////////////////////////////////////
    namespace detail
    {
      inline std::string strEncode( std::string val_r )
      {
	typedef unsigned char uchar;

	std::string::size_type add = 2;	// enclosing "s
	for_( r, val_r.begin(), val_r.end() )
	{
	  if ( uchar(*r) < 32u )
	  {
	    switch ( *r )
	    {
	      case '\b':
	      case '\f':
	      case '\n':
	      case '\r':
	      case '\t':
		add += 1;	// "\c"
		break;
	      default:
		add += 5;	// "\uXXXX"
		break;
	    }
	  }
	  else
	  {
	    switch ( *r )
	    {
	      case '"':
	      case '/':
	      case '\\':
		add += 1;	// \-escape
		break;
	    }
	  }
	}

	val_r.resize( val_r.size() + add, '@' );
	auto w( val_r.rbegin() );
	auto r( w + add );

	*w++ = '"';
	for ( ; r != val_r.rend(); ++r )
	{
	  if ( uchar(*r) < 32u )
	  {
	    static const char * digit = "0123456789abcdef";
	    switch ( *r )
	    {
	      case '\b':	// "\c"
		*w++ = 'b';
		*w++ = '\\';
		break;
	      case '\f':	// "\c"
		*w++ = 'f';
		*w++ = '\\';
		break;
	      case '\n':	// "\c"
		*w++ = 'n';
		*w++ = '\\';
		break;
	      case '\r':	// "\c"
		*w++ = 'r';
		*w++ = '\\';
		break;
	      case '\t':	// "\c"
		*w++ = 't';
		*w++ = '\\';
		break;
	      default:		// "\uXXXX"
		*w++ = digit[uchar(*r) % 15];
		*w++ = digit[uchar(*r) / 16];
		*w++ = '0';
		*w++ = '0';
		*w++ = 'u';
		*w++ = '\\';
		break;
	    }
	  }
	  else
	  {
	    switch ( (*w++ = *r) )
	    {
	      case '"':
	      case '/':
	      case '\\':	// \-escape
		*w++ = '\\';
		break;
	    }
	  }
	}
	*w++ = '"';
	return val_r;
      }
    } // namespace detail
    ///////////////////////////////////////////////////////////////////

    // null
    inline std::string toJSON( void )			{ return nullJSON(); }
    inline std::string toJSON( std::nullptr_t )		{ return nullJSON(); }

    // bool
    inline std::string toJSON( bool val_r  )		{ return val_r ? trueJSON() : falseJSON(); }
    inline std::string toJSON( const void * val_r )	{ return val_r ? trueJSON() : falseJSON(); }

    // numbers
    inline std::string toJSON( short val_r )		{ return str::numstring( val_r ); }
    inline std::string toJSON( unsigned short val_r )	{ return str::numstring( val_r ); }
    inline std::string toJSON( int val_r )		{ return str::numstring( val_r ); }
    inline std::string toJSON( unsigned val_r )		{ return str::numstring( val_r ); }
    inline std::string toJSON( long val_r )		{ return str::numstring( val_r ); }
    inline std::string toJSON( unsigned long val_r )	{ return str::numstring( val_r ); }
    inline std::string toJSON( long long val_r )	{ return str::numstring( val_r ); }
    inline std::string toJSON( unsigned long long val_r ){ return str::numstring( val_r ); }

    // strings
    inline std::string toJSON( const char val_r )	{ return detail::strEncode( std::string( 1, val_r ) ); }
    inline std::string toJSON( const char * val_r )	{ return val_r ? detail::strEncode( val_r ) : nullJSON(); }
    inline std::string toJSON( const std::string & val_r ){ return detail::strEncode( val_r ); }

    // container to Array
    template <class V> std::string toJSON( const std::vector<V> & cont_r );
    template <class V> std::string toJSON( const std::list<V> & cont_r );
    template <class V> std::string toJSON( const std::set<V> & cont_r );

    // map to Object
    template <class K, class V> std::string toJSON( const std::map<K,V> & cont_r );

    /** Type to JSON string representation.
     * This can be implemented as non-static memberfunction \c asJSON,
     * or as non-memberfunction \c toJSON;
     * \code
     *   class Type;
     *   std::string Type::asJSON() const;
     *   std::string toJSON( const Type & );
     * \endcode
     */
    template <class T>
    std::string toJSON( const T & val_r ) { return val_r.asJSON(); }

    ///////////////////////////////////////////////////////////////////
    /// \class Value
    /// \brief JSON representation of datatypes via \ref toJSON
    /// \code
    ///   namespace mynamspace
    ///   {
    ///     struct Mydata
    ///     {...};
    ///
    ///     std::string toJSON( const Mydata & )
    ///     { return json::Array{ "answer", 42 }.asJSON(); }
    ///   }
    ///
    ///   mynamspace::Mydata data;
    ///   json::Object bigone {
    ///     { "mydata",  data },
    ///     { "panic",   false },
    ///     { "nested",  json::Object{ {"one",1}, {"two",2}, {"three",3} } }
    ///   };
    ///
    ///   cout << bigone << endl;
    ///\endcode
    /// \see http://www.json.org/
    ///////////////////////////////////////////////////////////////////
    struct Value
    {
      /** Default ctor (null) */
      Value() : _data( toJSON() ) {}

      /** Copy ctor */
      Value( const Value & rhs ) : _data( rhs._data ) {}

      /** Ctor creating a JSON representation of \a T via \ref toJSON(T) */
      template <class T>
      Value( const T & val_r ) : _data( toJSON( val_r ) ) {}

      /** JSON representation */
      const std::string & asJSON() const
      { return _data; }

      /** String representation */
      const std::string & asString() const
      { return asJSON(); }

      /** Stream output */
      std::ostream & dumpOn( std::ostream & str ) const
      { return str << _data; }

    private:
      std::string _data;
    };

    /** \relates Value Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Value & obj )
    { return obj.dumpOn( str ); }

    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /// \class String
    /// \brief JSON string
    /// Force representation as JSON string, mapping e.g. \c null values
    /// to an empty string. Maninly used in \ref Object as key.
    ///////////////////////////////////////////////////////////////////
    struct String : public Value
    {
      String()				: Value( "" ) {}
      String( std::nullptr_t )		: Value( "" ) {}

      String( const char val_r )	: Value( val_r ) {}
      String( const char * val_r )	: Value( val_r ? val_r : "" ) {}
      String( const std::string & val_r ): Value( val_r ) {}
    };

    ///////////////////////////////////////////////////////////////////
    /// \class Array
    /// \brief JSON array
    ///////////////////////////////////////////////////////////////////
    struct Array
    {
      Array() {}

      /** Construct from container iterator */
      template <class Iterator>
      Array( Iterator begin, Iterator end )
      { for_( it, begin, end ) add( *it ); }

      /** Construct from container initializer list { v1, v2,... } */
      Array( const std::initializer_list<Value> & contents_r )
	: Array( contents_r.begin(), contents_r.end() )
      {}

      /** Push JSON Value to Array */
      void add( const Value & val_r )
      { _data.push_back( val_r.asJSON() ); }

      /** \overload from container initializer list { v1, v2,... } */
      void add( const std::initializer_list<Value> & contents_r )
      { for_( it, contents_r.begin(), contents_r.end() ) add( *it ); }

      /** JSON representation */
      std::string asJSON() const
      { return str::Str() << *this; }

      /** String representation */
      std::string asString() const
      { return asJSON(); }

      /** Stream output */
      std::ostream & dumpOn( std::ostream & str ) const
      {
	if ( _data.empty() )
	  return str << "[]";
	str << '[' << *_data.begin();
	for_( val, ++_data.begin(), _data.end() )
	  str << ", " << *val;
	return str << ']';
      }

    private:
      std::list<std::string> _data;
    };

    /** \relates Array Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Array & obj )
    { return obj.dumpOn( str ); }

    template <class V>
    std::string toJSON( const std::vector<V> & cont_r )
    { return json::Array( cont_r.begin(), cont_r.end() ).asJSON(); }

    template <class V>
    std::string toJSON( const std::list<V> & cont_r )
    { return json::Array( cont_r.begin(), cont_r.end() ).asJSON(); }

    template <class V>
    std::string toJSON( const std::set<V> & cont_r )
    { return json::Array( cont_r.begin(), cont_r.end() ).asJSON(); }

    ///////////////////////////////////////////////////////////////////
    /// \class Object
    /// \brief JSON object
    ///////////////////////////////////////////////////////////////////
    struct Object
    {
      Object() {}

      /** Construct from map-iterator */
      template <class Iterator>
      Object( Iterator begin, Iterator end )
      { for_( it, begin, end ) add( it->first, it->second ); }

      /** Construct from map-initializer list { {k1,v1}, {k2,v2},... } */
      Object( const std::initializer_list<std::pair<String, Value>> & contents_r )
	: Object( contents_r.begin(), contents_r.end() )
      {}

      /** Add key/value pair */
      void add( const String & key_r, const Value & val_r )
      { _data[key_r.asJSON()] = val_r.asJSON(); }

      /** \overload from map-initializer list { {k1,v1}, {k2,v2},... } */
      void add( const std::initializer_list<std::pair<String, Value>> & contents_r )
      { for_( it, contents_r.begin(), contents_r.end() ) add( it->first, it->second ); }

      /** JSON representation */
      std::string asJSON() const
      { return str::Str() << *this; }

      /** String representation */
      std::string asString() const
      { return asJSON(); }

      /** Stream output */
      std::ostream & dumpOn( std::ostream & str ) const
      {
	using std::endl;
	if ( _data.empty() )
	  return str << "{}";
	dumpOn( str << '{' << endl, _data.begin() );
	for_ ( val, ++_data.begin(), _data.end() )
	  dumpOn( str << ',' << endl, val );
	return str << endl << '}';
      }

    private:
      std::ostream & dumpOn( std::ostream & str, std::map<std::string,std::string>::const_iterator val_r ) const
      { return str << val_r->first << ": " << val_r->second; }

      std::map<std::string,std::string> _data;
    };

    /** \relates Object Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Object & obj )
    { return obj.dumpOn( str ); }

    template <class K, class V>
    std::string toJSON( const std::map<K,V> & cont_r )
    { return json::Object( cont_r.begin(), cont_r.end() ).asJSON(); }


  } // namespace json
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_JSON_H
