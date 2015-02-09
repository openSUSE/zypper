/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/TypeTraits.h
 */
#ifndef ZYPP_TYPETRAITS_H
#define ZYPP_TYPETRAITS_H

#include <type_traits>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace _detail
  {
    template<typename _Tp>
    struct _has_type_const_iterator
    {
    private:
      template<typename C> static std::true_type  test( typename C::const_iterator * );
      template<typename C> static std::false_type test(...);
    public:
      static constexpr bool value = decltype(test<_Tp>(nullptr))::value;
    };

    template <typename _Tp>
    struct _has_container_begin_end
    {
    private:
      template <typename C>
      using Signature = typename C::const_iterator(C::*)() const;

      template<typename C> static std::true_type  testBeg( typename std::enable_if<std::is_same<decltype(static_cast<Signature<C>>(&C::begin)), Signature<C>>::value, void>::type* );
      template<typename C> static std::false_type testBeg(...);

      template<typename C> static std::true_type  testEnd( typename std::enable_if<std::is_same<decltype(static_cast<Signature<C>>(&C::end)), Signature<C>>::value,   void>::type* );
      template<typename C> static std::false_type testEnd(...);

    public:
      static constexpr bool beg_value = decltype(testBeg<_Tp>(nullptr))::value;
      static constexpr bool end_value = decltype(testEnd<_Tp>(nullptr))::value;
      static constexpr bool value = beg_value && end_value;
    };
  } // namespace _detail
  ///////////////////////////////////////////////////////////////////

  /** Whether \a _Tp defines type \a _Tp::const_iterator */
  template<typename _Tp>
  struct has_type_const_iterator
  : public std::integral_constant<bool, _detail::has_type_const_iterator<_Tp>::value>
  {};

  /** Whether \a _Tp defines methods <tt>_Tp::const_iterator begin/end() const</tt> */
  template<typename _Tp>
  struct has_container_begin_end
  : public std::integral_constant<bool, _detail::_has_container_begin_end<_Tp>::value>
  {};

  /** Whether \a _Tp is a container (begin/end iterabel, but not plain std::string) */
  template<typename _Tp>
  struct is_container
  : public std::integral_constant<bool, !std::is_same<_Tp, std::string>::value && has_container_begin_end<_Tp>::value>
  {};


} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TYPETRAITS_H
