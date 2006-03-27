/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Counter.h
 *
*/
#ifndef ZYPP_BASE_COUNTER_H
#define ZYPP_BASE_COUNTER_H

#include <iosfwd>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Counter
  //
  /** Integral type with initial value \c 0.
  */
  template<class _IntT>
    class Counter
    {
    public:
      Counter( _IntT value_r = _IntT(0) )
      : _value( _IntT( value_r ) )
      {}

      operator _IntT &()
      { return _value; }

      operator const _IntT &() const
      { return _value; }

    public:
      _IntT _value;
    };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_COUNTER_H
