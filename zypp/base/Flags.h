/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Flags.h
 *
*/
#ifndef ZYPP_BASE_FLAGS_H
#define ZYPP_BASE_FLAGS_H

#include "zypp/base/String.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Flags<Enum>
    //
    /** A type-safe way of storing OR-combinations of enum values (like QTs QFlags).
     * \see <a href="http://doc.trolltech.com/4.1/qflags.html">QFlags Class Reference</a>
     * \code
     *  class RpmDb
     *  {
     *    public:
     *      enum DbStateInfoBits {
     *        DbSI_NO_INIT	= 0x0000,
     *        DbSI_HAVE_V4	= 0x0001,
     *        DbSI_MADE_V4	= 0x0002,
     *        DbSI_MODIFIED_V4	= 0x0004,
     *        DbSI_HAVE_V3	= 0x0008,
     *        DbSI_HAVE_V3TOV4	= 0x0010,
     *        DbSI_MADE_V3TOV4	= 0x0020
     *      };
     *
     *      ZYPP_DECLARE_FLAGS(DbStateInfo,DbStateInfoBits);
     *  };
     *  ZYPP_DECLARE_OPERATORS_FOR_FLAGS(RpmDb::DbStateInfo);
     *
     *  ...
     *  enum Other { OTHERVAL = 13 };
     *  {
     *    XRpmDb::DbStateInfo s;
     *    s = XRpmDb::DbSI_MODIFIED_V4|XRpmDb::DbSI_HAVE_V4;
     *    // s |= OTHERVAL; // As desired: it does not compile
     *  }
     * \endcode
     */
    template<typename _Enum>
    class Flags
    {
      public:
        typedef _Enum Enum;	///< The underlying enum type
        typedef typename std::underlying_type<Enum>::type Integral;	///< The underlying integral type

      public:
        constexpr Flags()				: _val( 0 ) {}
        constexpr Flags( Enum flag_r )			: _val( integral(flag_r) ) {}
        constexpr explicit Flags( Integral flag_r )	: _val( flag_r ) {}

        constexpr static Flags none()			{ return Flags( Integral(0) ); }
        constexpr static Flags all()			{ return Flags( ~Integral(0) ); }

        constexpr bool isNone() const			{ return _val == Integral(0); }
        constexpr bool isAll() const			{ return _val == ~Integral(0); }

        Flags & operator&=( Flags rhs )			{ _val &= integral(rhs); return *this; }
        Flags & operator&=( Enum rhs )			{ _val &= integral(rhs); return *this; }

        Flags & operator|=( Flags rhs )			{ _val |= integral(rhs); return *this; }
        Flags & operator|=( Enum rhs )			{ _val |= integral(rhs); return *this; }

        Flags & operator^=( Flags rhs )			{ _val ^= integral(rhs); return *this; }
        Flags & operator^=( Enum rhs )			{ _val ^= integral(rhs); return *this; }

      public:
        constexpr operator Integral() const		{ return _val; }

        constexpr Flags operator&( Flags rhs ) const	{ return Flags( _val & integral(rhs) ); }
        constexpr Flags operator&( Enum rhs ) const	{ return Flags( _val & integral(rhs) ); }

        constexpr Flags operator|( Flags rhs ) const	{ return Flags( _val | integral(rhs) ); }
        constexpr Flags operator|( Enum rhs ) const	{ return Flags( _val | integral(rhs) ); }

        constexpr Flags operator^( Flags rhs ) const	{ return Flags( _val ^ integral(rhs) ); }
        constexpr Flags operator^( Enum rhs ) const	{ return Flags( _val ^ integral(rhs) ); }

        constexpr Flags operator~() const		{ return Flags( ~_val ); }

        constexpr bool operator==( Enum rhs ) const	{  return( _val == integral(rhs) ); }
        constexpr bool operator!=( Enum rhs ) const	{  return( _val != integral(rhs) ); }

      public:
        Flags & setFlag( Flags flag_r, bool newval_r )	{ return( newval_r ? setFlag(flag_r) : unsetFlag(flag_r) ); }
        Flags & setFlag( Enum flag_r, bool newval_r )	{ return( newval_r ? setFlag(flag_r) : unsetFlag(flag_r) ); }

        Flags & setFlag( Flags flag_r )			{ _val |= integral(flag_r); return *this; }
        Flags & setFlag( Enum flag_r )			{ _val |= integral(flag_r); return *this; }

        Flags & unsetFlag( Flags flag_r )		{ _val &= ~integral(flag_r); return *this; }
        Flags & unsetFlag( Enum flag_r )		{ _val &= ~integral(flag_r); return *this; }

        constexpr bool testFlag( Flags flag_r ) const	{ return testFlag( integral(flag_r) ); }
        constexpr bool testFlag( Enum flag_r ) const	{ return testFlag( integral(flag_r) ); }

      private:
	constexpr bool testFlag( Integral flag )	{ return flag ? ( _val & flag ) == flag : !_val; }

	constexpr static Integral integral( Flags obj )	{ return obj._val; }
	constexpr static Integral integral( Enum obj )	{ return static_cast<Integral>(obj); }

        Integral _val;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Flags Stringify
     * Build a string of OR'ed names of each flag value set in \a flag_r.
     * Remaining bits in \a flag_r are added as hexstring.
     * \code
     * 	 enum E { a=1, b=2, c=4 };
     *   ZYPP_DECLARE_FLAGS( E, MyFlags );
     *
     *   MyFlags f = a|b|c;
     *   cout << f << " = " << stringify( f, { {a,"A"}, {b,"B"} } ) << endl;
     *   // prints: 0x0007 = [A|B|0x4]
     * \endcode
     */
    template<typename Enum>
    std::string stringify( const Flags<Enum> & flag_r, const std::initializer_list<std::pair<Flags<Enum>,std::string> > & flaglist_r = {},
			   std::string intro_r = "[", std::string sep_r = "|", std::string extro_r = "]" )
    {
      std::string ret( std::move(intro_r) );
      std::string sep;

      Flags<Enum> mask;
      for ( const auto & pair : flaglist_r )
      {
	if ( flag_r.testFlag( pair.first ) )
	{
	  mask |= pair.first;
	  ret += sep;
	  ret += pair.second;
	  if ( sep.empty() && !sep_r.empty() )
	  { sep = std::move(sep_r); }
	}
      }
      mask = flag_r & ~mask;
      if ( mask )
      {
	ret += sep;
	ret += str::hexstring( mask, 0 );
      }
      ret += std::move(extro_r);
      return ret;
    }

    template<typename _Enum>
    inline std::ostream & operator<<( std::ostream & str, const Flags<_Enum> & obj )
    { return str << str::hexstring(obj); }

    template<typename _Enum>
    inline std::ostream & operator<<( std::ostream & str, const typename Flags<_Enum>::Enum & obj )
    { return str << Flags<_Enum>(obj); }

    /** \relates Flags */
#define ZYPP_DECLARE_FLAGS(Name,Enum) typedef zypp::base::Flags<Enum> Name

    /** \relates Flags */
#define ZYPP_DECLARE_OPERATORS_FOR_FLAGS(Name) \
inline constexpr bool operator==( Name::Enum lhs, Name rhs )		{ return( rhs == lhs ); }	\
inline constexpr bool operator!=(Name:: Enum lhs, Name rhs )		{ return( rhs != lhs ); }	\
inline constexpr Name operator&( Name::Enum lhs, Name::Enum rhs )	{ return Name( lhs ) & rhs; }	\
inline constexpr Name operator&( Name::Enum lhs, Name rhs )		{ return rhs & lhs; }		\
inline constexpr Name operator|( Name::Enum lhs, Name::Enum rhs )	{ return Name( lhs ) | rhs; }	\
inline constexpr Name operator|( Name::Enum lhs, Name rhs )		{ return rhs | lhs; }		\
inline constexpr Name operator^( Name::Enum lhs, Name::Enum rhs )	{ return Name( lhs ) ^ rhs; }	\
inline constexpr Name operator^( Name::Enum lhs, Name rhs )		{ return rhs ^ lhs; }		\
inline constexpr Name operator~( Name::Enum lhs )			{ return ~Name( lhs ); }

    /** \relates Flags */
#define ZYPP_DECLARE_FLAGS_AND_OPERATORS(Name,Enum) \
    ZYPP_DECLARE_FLAGS(Name,Enum); \
    ZYPP_DECLARE_OPERATORS_FOR_FLAGS(Name)

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_FLAGS_H
