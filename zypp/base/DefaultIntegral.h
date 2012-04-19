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
  //  CLASS NAME : DefaultIntegral<_Tp,_Initial>
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
   * \todo let _Initial default to 0 then remove base/Counter.h
  */
  template<class _Tp, _Tp _Initial>
    class DefaultIntegral
    {
    public:
      typedef _Tp value_type;

    public:
      DefaultIntegral( _Tp val_r = _Initial )
      : _val( val_r )
      { BOOST_STATIC_ASSERT(boost::is_integral<_Tp>::value); }

      /** Conversion to _Tp. */
      //@{
      _Tp & get()       { return _val; }
      _Tp   get() const { return _val; }

      operator _Tp &()       { return get(); }
      operator _Tp  () const { return get(); }
      //@}

      /** Reset to the defined initial value. */
      DefaultIntegral & reset()	{ _val = _Initial; return *this; }

      /** \name Arithmetic operations.
       * \c + \c - \c * \c / are provided via conversion to _Tp.
      */
      //@{
      DefaultIntegral & operator=( _Tp rhs )  {  _val = rhs; return *this; }
      DefaultIntegral & operator+=( _Tp rhs ) { _val += rhs; return *this; }
      DefaultIntegral & operator-=( _Tp rhs ) { _val -= rhs; return *this; }
      DefaultIntegral & operator*=( _Tp rhs ) { _val *= rhs; return *this; }
      DefaultIntegral & operator/=( _Tp rhs ) { _val /= rhs; return *this; }

      DefaultIntegral & operator++(/*prefix*/) { ++_val; return *this; }
      DefaultIntegral & operator--(/*prefix*/) { --_val; return *this; }

      DefaultIntegral operator++(int/*postfix*/) { return _val++; }
      DefaultIntegral operator--(int/*postfix*/) { return _val--; }
      //@}

    private:
      _Tp _val;
    };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_DEFAULTINTEGRAL_H
