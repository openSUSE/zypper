/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/SafeBool.h
 *
*/
#ifndef ZYPP_BASE_SAFEBOOL_H
#define ZYPP_BASE_SAFEBOOL_H

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    namespace safebool_detail
    {
      class SafeBoolBase
      {
      protected:
        typedef void (SafeBoolBase::*bool_type)() const;
        void theTrueBoolType() const {}

        SafeBoolBase() {}
        SafeBoolBase( const SafeBoolBase & ) {}
        ~SafeBoolBase() {}
        SafeBoolBase & operator=( const SafeBoolBase & ) { return *this; }
      };
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SafeBool
    //
    /** Validate objects in a boolean context without harmful side effects.
     * \see http://www.artima.com/cppsource/safebool.html
     *
     * Uses CRTP to avoid a virtual function. \c _Derived must provide
     * <tt>bool boolTest() const</tt> preformong the test.
     *
     * \note Using SafeBool enables ==/!= comparision for \c Foo, based on
     * the bool_type values. Make shure you overload \b both operators, in
     * case an other semantic is desired for ==/!=.
     *
     * \code
     * class Foo : protected base::SafeBool<Foo>
     * {
     * public:
     *   using base::SafeBool<Foo>::operator bool_type;
     *
     * private:
     *   friend SafeBool<TT>::operator bool_type() const;
     *   bool boolTest() const
     *   {
     *     // Perform Boolean logic here
     *   }
     * };
     * \endcode
     * \todo Investigate why Bit refuses private inheritance
     * and exposition of operator bool_type. Seems to be a gcc
     * bug. protected works.
    */
    template<class _Derived>
      struct SafeBool : private safebool_detail::SafeBoolBase
      {
        using safebool_detail::SafeBoolBase::bool_type;
        operator bool_type() const
        {
          return( (static_cast<const _Derived *>(this))->boolTest()
                  ? &safebool_detail::SafeBoolBase::theTrueBoolType
                  : 0 );
        }
      protected:
        ~SafeBool() {}
      };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_SAFEBOOL_H
