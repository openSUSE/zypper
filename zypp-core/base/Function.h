/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Function.h
 *
*/
#ifndef ZYPP_BASE_FUNCTION_H
#define ZYPP_BASE_FUNCTION_H

#include <functional>
#include <boost/function.hpp>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /* http://www.boost.org/doc/html/function.html

   The Boost.Function library contains a family of class templates
   that are function object wrappers. The notion is similar to a
   generalized callback. It shares features with function pointers
   in that both define a call interface (e.g., a function taking
   two integer arguments and returning a floating-point value)
   through which some implementation can be called, and the
   implementation that is invoked may change throughout the
   course of the program.

   Generally, any place in which a function pointer would be used
   to defer a call or make a callback, Boost.Function can be used
   instead to allow the user greater flexibility in the implementation
   of the target. Targets can be any 'compatible' function object
   (or function pointer), meaning that the arguments to the interface
   designated by Boost.Function can be converted to the arguments of
   the target function object.
  */
  using boost::function;

  using std::bind;
  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_FUNCTION_H
