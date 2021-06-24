#ifndef ZYPPNG_META_TYPE_TRAITS_INCLUDED
#define ZYPPNG_META_TYPE_TRAITS_INCLUDED

#include <type_traits>
#include <memory>

#if !defined ( __cpp_lib_void_t )

namespace std {
  //define void_t since its only available starting with C++17 in some compilers
  template<typename... Ts> struct make_void { typedef void type;};
  template<typename... Ts> using void_t = typename make_void<Ts...>::type;
}

#endif


#if __cplusplus < 201703L

namespace std {

//implementation of the detector idiom, used to help with SFINAE
//from https://en.cppreference.com/w/cpp/experimental/is_detected

namespace detail {
template <class Default, class AlwaysVoid,
  template<class...> class Op, class... Args>
struct detector {
  using value_t = std::false_type;
  using type = Default;
};

template <class Default, template<class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
  using value_t = std::true_type;
  using type = Op<Args...>;
};

struct nonesuch{
  ~nonesuch( )                     = delete;
  nonesuch( nonesuch const& )      = delete;
  void operator = ( nonesuch const& ) = delete;
};

} // namespace detail

template <bool B> using bool_constant = integral_constant<bool, B>;

template <template<class...> class Op, class... Args>
using is_detected = typename detail::detector<detail::nonesuch, void, Op, Args...>::value_t;

template <template<class...> class Op, class... Args>
using detected_t = typename detail::detector<detail::nonesuch, void, Op, Args...>::type;

template <class Default, template<class...> class Op, class... Args>
using detected_or = detail::detector<Default, void, Op, Args...>;

template< template<class...> class Op, class... Args >
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template< class Default, template<class...> class Op, class... Args >
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

template <class Expected, template<class...> class Op, class... Args>
using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

template <class Expected, template<class...> class Op, class... Args>
constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

template <class To, template<class...> class Op, class... Args>
using is_detected_convertible = std::is_convertible<detected_t<Op, Args...>, To>;

template <class To, template<class...> class Op, class... Args>
constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value_t::value;

//https://en.cppreference.com/w/cpp/types/conjunction)
template<class...> struct conjunction : std::true_type { };
template<class B1> struct conjunction<B1> : B1 { };
template<class B1, class... Bn>
struct conjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

//https://en.cppreference.com/w/cpp/types/disjunction
template<class...> struct disjunction : std::false_type { };
template<class B1> struct disjunction<B1> : B1 { };
template<class B1, class... Bn>
struct disjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), B1, disjunction<Bn...>>  { };

//https://en.cppreference.com/w/cpp/types/negation
template<class B>
struct negation : std::bool_constant< !bool(B::value)> { };

}
#endif


namespace zyppng {
  //check wether something is a instance of a template type
  template < template< class ... > class Templ, class Type >
  struct is_instance_of : std::false_type{};

  template < template< typename ... > class Templ, typename... Args >
  struct is_instance_of<Templ, Templ<Args...>> : std::true_type{};

  //Provides the member typedef type which is the type pointed to by T, or, if T is not a pointer, then type is the same as T.
  template< typename T>
  struct remove_smart_ptr{ using type = typename std::remove_cv<T>::type; };

  template< typename T>
  struct remove_smart_ptr<std::shared_ptr<T>>{ using type = typename std::remove_cv<T>::type; };

  template< typename T>
  struct remove_smart_ptr<std::unique_ptr<T>>{ using type = typename std::remove_cv<T>::type; };

  template< typename T>
  using remove_smart_ptr_t = typename remove_smart_ptr<T>::type;

  //helper template to print type parameters
  template <typename T>
  class myerror_t;
}

#endif
