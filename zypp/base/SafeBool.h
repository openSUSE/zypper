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
     * \code
     * class Foo : public base::SafeBool<Foo>
     * {
     * public:
     *   bool boolTest() const
     *   {
     *     // Perform Boolean logic here
     *   }
     * };
     * \endcode
     * \todo Investigate why Bit refuses private inheritance
     * and exposition of operator bool_type.
    */
    template<class _Derived>
      struct SafeBool : public safebool_detail::SafeBoolBase
      {
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
