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
  template<class TInt>
    class Counter
    {
    public:
      Counter( TInt value_r = TInt(0) )
      : _value( TInt( value_r ) )
      {}

      operator TInt &()
      { return _value; }

      operator const TInt &() const
      { return _value; }

    public:
      TInt _value;
    };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_COUNTER_H
