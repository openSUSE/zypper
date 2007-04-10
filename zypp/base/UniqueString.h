/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/UniqueString.h
 *
*/
#ifndef ZYPP_BASE_UNIQUESTRING_H
#define ZYPP_BASE_UNIQUESTRING_H

#include <iosfwd>
#include <string>

#include <zypp/base/Hash.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : UniqueString
    //
    /** Immutable strings with unique representation in memory.
     *
     * Uses CRTP to provide the unifying hash and define operators.
     * All operatoins are string based.
    */
    template<class _Derived>
	class UniqueString
    {
    protected:
      /** Default ctor */
      UniqueString()
      {}

      /** Ctor taking a name to store unified. */
      UniqueString( const std::string & name_r )
      {
	if ( !name_r.empty() )
	{
	  _name = *(this->hash().insert( name_r ).first);
	}
      }

      /** Dtor */
      virtual ~UniqueString()
      {}

    public:
      /** Explicit conversion to string. */
      const std::string & asString() const
      { return _name; }

      /** Explicit conversion to string (convenience). */
      const std::string & str() const
      { return asString(); }

      /** Implicit conversion to string. */
      operator const std::string & () const
      { return asString(); }

      /** Short for <tt>str().size()</tt>. */
      std::string::size_type size() const
      { return asString().size(); }

      /** Short for <tt>str().empty()</tt>. */
      bool empty() const
      { return asString().empty(); }

      /** Short for <tt>str().compare( s )</tt>.*/
      int compare( const std::string & rhs ) const
      {	return asString().compare( rhs ); }

    public:
      typedef hash_set<std::string>::size_type size_type;
      typedef hash_set<std::string>::const_iterator const_iterator;

      /** Whether there are known UniqueStrings. */
      static bool allEmpty()
      { return hash().empty(); }

      /** Number of known UniqueStrings. */
      static size_type allSize()
      { return hash().size(); }

      /** Iterator to the 1st UniqueString. */
      static const_iterator allBegin()
      { return hash().begin(); }

      /** Iterator behind the last UniqueString. */
      static const_iterator allEnd()
      { return hash().end(); }

    private:
      /** Provides the static hash to unify the strings. */
      static hash_set<std::string> & hash()
      {
	static hash_set<std::string> _value;
	return _value;
      }

    private:
      /** Immutable string. */
      std::string _name;
    };
    ///////////////////////////////////////////////////////////////////

    // operator ==
    template<class _Derived>
	inline bool operator==( const UniqueString<_Derived> & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs.str() == rhs.str() ); }

    template<class _Derived>
	inline bool operator==( const UniqueString<_Derived> & lhs, const std::string & rhs )
    { return ( lhs.str() == rhs ); }

    template<class _Derived>
	inline bool operator==( const std::string & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs == rhs.str() ); }


     // operator !=
    template<class _Derived>
	inline bool operator!=( const UniqueString<_Derived> & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs.str() != rhs.str() ); }

    template<class _Derived>
	inline bool operator!=( const UniqueString<_Derived> & lhs, const std::string & rhs )
    { return ( lhs.str() != rhs ); }

    template<class _Derived>
	inline bool operator!=( const std::string & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs != rhs.str() ); }


     // operator <
    template<class _Derived>
	inline bool operator<( const UniqueString<_Derived> & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs.str() < rhs.str() ); }

    template<class _Derived>
	inline bool operator<( const UniqueString<_Derived> & lhs, const std::string & rhs )
    { return ( lhs.str() < rhs ); }

    template<class _Derived>
	inline bool operator<( const std::string & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs < rhs.str() ); }


     // operator >
    template<class _Derived>
	inline bool operator>( const UniqueString<_Derived> & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs.str() > rhs.str() ); }

    template<class _Derived>
	inline bool operator>( const UniqueString<_Derived> & lhs, const std::string & rhs )
    { return ( lhs.str() > rhs ); }

    template<class _Derived>
	inline bool operator>( const std::string & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs > rhs.str() ); }


     // operator <=
    template<class _Derived>
	inline bool operator<=( const UniqueString<_Derived> & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs.str() <= rhs.str() ); }

    template<class _Derived>
	inline bool operator<=( const UniqueString<_Derived> & lhs, const std::string & rhs )
    { return ( lhs.str() <= rhs ); }

    template<class _Derived>
	inline bool operator<=( const std::string & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs <= rhs.str() ); }


     // operator !=
    template<class _Derived>
	inline bool operator>=( const UniqueString<_Derived> & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs.str() >= rhs.str() ); }

    template<class _Derived>
	inline bool operator>=( const UniqueString<_Derived> & lhs, const std::string & rhs )
    { return ( lhs.str() >= rhs ); }

    template<class _Derived>
	inline bool operator>=( const std::string & lhs, const UniqueString<_Derived> & rhs )
    { return ( lhs >= rhs.str() ); }

   /** \relates UniqueString Stream output */
    template<class _Derived>
	inline std::ostream & operator<<( std::ostream & str, const UniqueString<_Derived> & obj )
    { return str << obj.str(); }

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_UNIQUESTRING_H
