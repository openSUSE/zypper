/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/DefaultIntegral.h
 *
*/
#ifndef ZYPP_BASE_DEFAULTINTEGRAL_H
#define ZYPP_BASE_DEFAULTINTEGRAL_H

#include <iosfwd>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_integral.hpp>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////


  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : DefaultIntegral<Tp,TInitial>
  //
  /** Integral type with defined initial value when default constructed.
   *
   * \code
   * typedef DefaultIntegral<unsigned,0> Counter;
   * std::map<KeyType,Counter> stats;
   * for ( all keys  )
   *   ++(stats[key]);
   * \endcode
   *
   * \todo maybe specialize for bool, add logical and bit operators
   * \todo let TInitial default to 0 then remove base/Counter.h
  */
  template<class Tp, Tp TInitial>
    class DefaultIntegral
    {
    public:
      typedef Tp value_type;

    public:
      DefaultIntegral( Tp val_r = TInitial )
      : _val( val_r )
      { BOOST_STATIC_ASSERT(boost::is_integral<Tp>::value); }

      /** Conversion to Tp. */
      //@{
      Tp & get()       { return _val; }
      Tp   get() const { return _val; }

      operator Tp &()       { return get(); }
      operator Tp  () const { return get(); }
      //@}

      /** The initial value. */
      constexpr Tp initial() const { return TInitial; }

      /** Reset to the defined initial value. */
      DefaultIntegral & reset()	{ _val = TInitial; return *this; }

      /** \name Arithmetic operations.
       * \c + \c - \c * \c / are provided via conversion to Tp.
      */
      //@{
      DefaultIntegral & operator=( Tp rhs )  {  _val = rhs; return *this; }
      DefaultIntegral & operator+=( Tp rhs ) { _val += rhs; return *this; }
      DefaultIntegral & operator-=( Tp rhs ) { _val -= rhs; return *this; }
      DefaultIntegral & operator*=( Tp rhs ) { _val *= rhs; return *this; }
      DefaultIntegral & operator/=( Tp rhs ) { _val /= rhs; return *this; }

      DefaultIntegral & operator++(/*prefix*/) { ++_val; return *this; }
      DefaultIntegral & operator--(/*prefix*/) { --_val; return *this; }

      DefaultIntegral operator++(int/*postfix*/) { return _val++; }
      DefaultIntegral operator--(int/*postfix*/) { return _val--; }
      //@}

    private:
      Tp _val;
    };

    /** \relates DefaultIntegral \c true initialized \c bool  */
    typedef DefaultIntegral<bool,true>  TrueBool;

    /** \relates DefaultIntegral \c false initialized \c bool */
    typedef DefaultIntegral<bool,false> FalseBool;

    /** \relates DefaultIntegral \c zero initialized \c integral */
    template<typename TIntegral>
    using ZeroInit = DefaultIntegral<TIntegral,TIntegral(0)>;

    namespace str
    {
      template<class Tp, Tp TInitial>
      std::string asString( const DefaultIntegral<Tp,TInitial> & obj )
      { return asString( obj.get() ); }
    } // namespace str

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_DEFAULTINTEGRAL_H
