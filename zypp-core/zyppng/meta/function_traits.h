/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
* //Code based on a blogpost on https://functionalcpp.wordpress.com/2013/08/05/function-traits/
*
*/
#ifndef ZYPPNG_META_FUNCTION_TRAITS_H_INCLUDED
#define ZYPPNG_META_FUNCTION_TRAITS_H_INCLUDED

#include <cstddef>
#include <tuple>
#include <zypp-core/zyppng/meta/TypeTraits>

namespace zyppng {

template<class F, class = void >
struct function_traits;

template<class R, class... Args>
struct function_traits<R(Args...)>
{
    using return_type = R;

    static constexpr std::size_t arity = sizeof...(Args);

    template <std::size_t N>
    struct argument
    {
        static_assert(N >= 0 && N < arity, "error: invalid parameter index.");
        using type = typename std::tuple_element<N,std::tuple<Args...>>::type;
    };
};

// function pointer
template<class R, class... Args>
struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)>
{ };

// function ref
template<class R, class... Args>
struct function_traits<R(&)(Args...)> : public function_traits<R(Args...)>
{ };

// member function pointer
template<class C, class R, class... Args>
struct function_traits<R(C::*)(Args...)> : public function_traits<R(C&,Args...)>
{};

// const member function pointer, this will match lambdas too
template<class C, class R, class... Args>
struct function_traits<R(C::*)(Args...) const>  : public function_traits<R(C&,Args...)>
{};

// member object pointer
template<class C, class R>
struct function_traits<R(C::*)> : public function_traits<R(C&)>
{};

template <typename T>
using has_call_operator = decltype (&T::operator());

//functor with one overload
template<class F>
struct function_traits<F, std::void_t<decltype (&F::operator())>> : public function_traits<decltype (&F::operator())>
{};

}
#endif
