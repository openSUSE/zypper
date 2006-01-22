/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/FunctorRef.h
 *
*/
#ifndef ZYPP_BASE_FUNCTORREF_H
#define ZYPP_BASE_FUNCTORREF_H

#include <functional>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace functor
  { /////////////////////////////////////////////////////////////////

    /** An unary functor forwarding to some other <tt>_Functor &</tt>.
     * \ingroup g_Functor
     *
     * Most algorithms take functor arguments by value. That's inconvenient
     * if the functor wants to collect and return data. Creating and
     * passing a \ref FunctorRef to the algorithm, may help you out of this.
     *
     * \code
     *   // Counts invokations of operator().
     *   template<class _Tp>
     *     struct Counter : public std::unary_function<_Tp, void>
     *     {
     *       void operator()( _Tp )
     *       { ++_value; }
     *
     *       Counter() : _value( 0 ) {}
     *
     *       unsigned _value;
     *     };
     *
     *   std::set<SomeType> c;
     *   Counter<SomeType> counter;
     *   // Invokations of FunctorRef are forwarded to counter:
     *   std::for_each( c.begin, c.end(), functorRef(counter) );
     * \endcode
     *
     * \note FunctorRef must be able to deduce the signature of \c _Functor::operator().
     * Per default the tyedefs  \c _Functor::argument_type and \c _Functor::result_type
     * are expected. They are e.g. provided by deriving your \c _Functor from
     * \c std::unary_function. In case these typedefs are not provided, you have to
     * specify them as additional template arguments:
     *
     * \code
     *   // Counts invokations of operator().
     *     struct Counter
     *     {
     *       template<class _Tp>
     *         void operator()( _Tp )
     *         { ++_value; }
     *
     *       Counter() : _value( 0 ) {}
     *
     *       unsigned _value;
     *     };
     *
     *   std::set<SomeType> c;
     *   Counter counter;
     *   // Invokations of FunctorRef are forwarded to counter:
     *   std::for_each( c.begin, c.end(),
     *                  functorRef<Counter, SomeType, void>(counter) );
     * \endcode
    */
    template <class _Functor, class argument_type = typename _Functor::argument_type,
                              class result_type = typename _Functor::result_type>
      class FunctorRef : public std::unary_function<argument_type, result_type>
      {
      public:
        FunctorRef( _Functor & f_r )
        : _f( f_r )
        {}

        typename FunctorRef::result_type operator()( typename FunctorRef::argument_type a1 ) const
        {
          return _f( a1 );
        }

      private:
        _Functor & _f;
      };

    /** Convenience function creating a \ref FunctorRef. */
    template <class _Functor>
      FunctorRef<_Functor> functorRef( _Functor & f_r )
      { return FunctorRef<_Functor>( f_r ); }

    template <class _Functor, class argument_type, class result_type>
      FunctorRef<_Functor,argument_type,result_type> functorRef( _Functor & f_r )
      { return FunctorRef<_Functor,argument_type,result_type>( f_r ); }

  /////////////////////////////////////////////////////////////////
} // namespace functor
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_FUNCTORREF_H
