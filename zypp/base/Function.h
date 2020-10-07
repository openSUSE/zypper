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

#include <boost/function.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#include <boost/ref.hpp>

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

  /* http://www.boost.org/libs/bind/bind.html

   boost::bind is a generalization of the standard functions std::bind1st
   and std::bind2nd. It supports arbitrary function objects, functions,
   function pointers, and member function pointers, and is able to bind
   any argument to a specific value or route input arguments into arbitrary
   positions. bind  does not place any requirements on the function object;
   in particular, it does not need the result_type, first_argument_type and
   second_argument_type  standard typedefs.
  */
  using boost::bind;

  /* http://www.boost.org/doc/html/ref.html

   The Ref library is a small library that is useful for passing references
   to function templates (algorithms) that would usually take copies of their
   arguments. It defines the class template boost::reference_wrapper<T>, the
   two functions boost::ref and boost::cref that return instances of
   boost::reference_wrapper<T>, and the two traits classes
   boost::is_reference_wrapper<T>  and boost::unwrap_reference<T>.

   The purpose of boost::reference_wrapper<T> is to contain a reference to an
   object of type T. It is primarily used to "feed" references to function
   templates (algorithms) that take their parameter by value.

   To support this usage, boost::reference_wrapper<T> provides an implicit
   conversion to T&. This usually allows the function templates to work on
   references unmodified.
  */
  using boost::ref;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_FUNCTION_H
