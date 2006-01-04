/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/solver/detail/Ustring.h
 *
*/
#ifndef ZYPP_SOLVER_DETAIL_USTRING_H
#define ZYPP_SOLVER_DETAIL_USTRING_H

#include <iosfwd>
#include <string>
#include <set>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : UstringHash
      /**
       * @short A Hash of unique strings.
       *
       * A Hash of unique strings.
       *
       * @see Ustring
       **/
      class UstringHash
      {

      protected:

        typedef std::set<std::string> UstringHash_type;

        UstringHash_type _UstringHash;

      public:

        const std::string & add( const std::string & nstr_r )
        {
          return *(_UstringHash.insert( nstr_r ).first);
        }

        /**
         * Return the number of unique strings stored in the hash.
         **/
        unsigned size() const { return _UstringHash.size(); }
        unsigned long sum() const {
          UstringHash_type::const_iterator it = _UstringHash.begin();
          UstringHash_type::const_iterator e = _UstringHash.end();
          unsigned long sum = 0;
          while (it != e)
            {
              sum += it->size();
              it++;
            }
          return sum;
        }
      };
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Ustring
      /**
       * @short Unique strings
       *
       * Ustring provides an immutable string and uses a UstringHash
       * to keep the strings representaion class (which contains the
       * actual data) unique.
       *
       * That way Ustrings of the same value and using the same
       * UstringHash can be stored at various locations, while the
       * actual string data are stored only once.
       *
       * The UstringHash to use is passed to the constructor, and no
       * more needed after the string has been stored.
       *
       * Conversion to string is possible and cheap, as the created
       * string will, unless he's modified, share it's data with the
       * one inside the Ustring.
       *
       * Comparison between Ustrings and strings are based on string.
       *
       * A <CODE>-></CODE> operator is provided as an easy way to
       * invoke const string methods, like size() or c_str().
       *
       * <PRE>
       *   ustr->size(); // short for ((const std::string &)u).size();
       * </PRE>
       *
       * Most common usage will be deriving some class from Ustring,
       * that provides a static UstringHash, and some constructor.
       * Everything else is provided by Ustring.
       *
       * <PRE>
       * class PkgName : public Ustring {
       *   private:
       *     static UstringHash _nameHash;
       *   public:
       *     explicit PkgName( const std::string & n = "" ) : Ustring( _nameHash, n ) {}
       * };
       * </PRE>
       *
       * @see UstringHash
       * /todo Review and move it to base
       **/
      class Ustring
      {

      private:

        /**
         * !!! It should actualy be const !!!
         * But that way default copy and assingment can be used.
         **/
        std::string _name;

      public:
        /**
         * Constructor calls @ref UstringHash::add on the given string,
         * and stores the string returned from the hash.
         **/
        Ustring( UstringHash & nameHash_r, const std::string & n  )
        :_name( nameHash_r.add( n ) )
        {}

      public:

        /**
         * Conversion to const std::string &
         **/
        const std::string & asString() const { return _name; }

        /**
         * Conversion to const std::string &
         **/
        operator const std::string & () const { return asString(); }

        /**
         * short for ((const std::string &)ustr).size();
         **/
        std::string::size_type size() const { return asString().size(); }

        /**
         * short for ((const std::string &)ustr).empty();
         **/
        bool empty() const { return asString().empty(); }


        int compare( const std::string & rhs ) const {
          return asString().compare( rhs );
        }

        int compare( const Ustring & rhs ) const {
          if ( *this == rhs )
            return 0;
          return( *this < rhs ? -1 : 1 );
        }

        /**
         * ustr->???(); // short for ((const std::string &)ustr).???();
         **/
        const std::string * operator->() const { return & asString(); }

        // operator ==

        friend bool operator==( const Ustring & lhs, const Ustring & rhs ) {
          // Ustrings share their string representation
          return ( lhs->c_str() == rhs->c_str() );
        }

        friend bool operator==( const Ustring & lhs, const std::string & rhs ) {
          return ( lhs.asString() == rhs );
        }

        friend bool operator==( const std::string & lhs, const Ustring & rhs ) {
          return ( lhs == rhs.asString() );
        }

        // operator !=

        friend bool operator!=( const Ustring & lhs, const Ustring & rhs ) {
          return ( ! operator==( lhs, rhs ) );
        }

        friend bool operator!=( const Ustring & lhs, const std::string & rhs ) {
          return ( ! operator==( lhs, rhs ) );
        }

        friend bool operator!=( const std::string & lhs, const Ustring & rhs ) {
          return ( ! operator==( lhs, rhs ) );
        }

        // operator <

        friend bool operator<( const Ustring & lhs, const Ustring & rhs ) {
          return ( lhs.asString() < rhs.asString() );
        }

        friend bool operator<( const Ustring & lhs, const std::string & rhs ) {
          return ( lhs.asString() < rhs );
        }

        friend bool operator<( const std::string & lhs, const Ustring & rhs ) {
          return ( lhs < rhs.asString() );
        }

        // operator >

        friend bool operator>( const Ustring & lhs, const Ustring & rhs ) {
          return ( lhs.asString() > rhs.asString() );
        }

        friend bool operator>( const Ustring & lhs, const std::string & rhs ) {
          return ( lhs.asString() > rhs );
        }

        friend bool operator>( const std::string & lhs, const Ustring & rhs ) {
          return ( lhs > rhs.asString() );
        }

        // operator >=

        friend bool operator>=( const Ustring & lhs, const Ustring & rhs ) {
          return ( ! operator<( lhs, rhs ) );
        }

        friend bool operator>=( const Ustring & lhs, const std::string & rhs ) {
          return ( ! operator<( lhs, rhs ) );
        }

        friend bool operator>=( const std::string & lhs, const Ustring & rhs ) {
          return ( ! operator<( lhs, rhs ) );
        }

        // operator <=

        friend bool operator<=( const Ustring & lhs, const Ustring & rhs ) {
          return ( ! operator>( lhs, rhs ) );
        }

        friend bool operator<=( const Ustring & lhs, const std::string & rhs ) {
          return ( ! operator>( lhs, rhs ) );
        }

        friend bool operator<=( const std::string & lhs, const Ustring & rhs ) {
          return ( ! operator>( lhs, rhs ) );
        }

        // IO

        friend std::ostream & operator<<( std::ostream & str, const Ustring & obj ) {
          return str << obj.asString();
        }
      };

      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace solver
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOLVER_DETAIL_USTRING_H
