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
    template<typename Tp>
    struct _has_type_const_iterator
    {
    private:
      template<typename C> static std::true_type  test( typename C::const_iterator * );
      template<typename C> static std::false_type test(...);
    public:
      static constexpr bool value = decltype(test<Tp>(nullptr))::value;
    };

    template <typename Tp>
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
      static constexpr bool beg_value = decltype(testBeg<Tp>(nullptr))::value;
      static constexpr bool end_value = decltype(testEnd<Tp>(nullptr))::value;
      static constexpr bool value = beg_value && end_value;
    };
  } // namespace _detail
  ///////////////////////////////////////////////////////////////////

  /** Whether \a Tp defines type \a Tp::const_iterator */
  template<typename Tp>
  struct has_type_const_iterator
  : public std::integral_constant<bool, _detail::has_type_const_iterator<Tp>::value>
  {};

  /** Whether \a Tp defines methods <tt>Tp::const_iterator begin/end() const</tt> */
  template<typename Tp>
  struct has_container_begin_end
  : public std::integral_constant<bool, _detail::_has_container_begin_end<Tp>::value>
  {};

  /** Whether \a Tp is a container (begin/end iterabel, but not plain std::string) */
  template<typename Tp>
  struct is_container
  : public std::integral_constant<bool, !std::is_same<Tp, std::string>::value && has_container_begin_end<Tp>::value>
  {};


} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TYPETRAITS_H
