/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Functional.h
 *
*/
#ifndef ZYPP_BASE_FUNCTIONAL_H
#define ZYPP_BASE_FUNCTIONAL_H

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

    /** Convenience function creating a \ref FunctorRef. */
    template <class _Functor, class argument_type, class result_type>
      FunctorRef<_Functor,argument_type,result_type> functorRef( _Functor & f_r )
      { return FunctorRef<_Functor,argument_type,result_type>( f_r ); }

    /////////////////////////////////////////////////////////////////

    /** \defgroup LOGICALFILTERS Functors for building compex queries.
     * \ingroup g_Functor
     *
     * Some logical functors to build more complex queries:
     *
     * \li \ref True and \ref False. No supprise, they always return
     *     \c true or \c false.
     * \li \ref Not\<_Condition\>. _Condition is a functor, and
     *     it's result is inverted.
     * \li \ref Chain\<_ACondition,_BCondition\>. \c _ACondition and \c _BCondition
     *     are functors, and Chain evaluates <tt>_ACondition && _BCondition</tt>.
     *
     * As it's no fun to get and write the correct template arguments,
     * convenience functions creating the correct functor are provided.
     *
     * \li \c true_c and \c false_c. (provided just to match the schema)
     * \li \c not_c. Takes a functor as argument and returns the appropriate
     *     \ref Not functor.
     * \li \c chain. Takes two functors and returns the appropriate
     *     \ref Cain functor.
     *
     * \code
     *  struct Print; // functor priniting elements
     *  struct Count; // functor counting number of elements
     *
     *  std::for_each( c.begin(), c.end(),
     *                 chain( Print(), Count() ) );
     * \endcode
    */
    //@{

    /** Logical functor always \c true.
    */
    struct True
    {
      template<class _Tp>
        bool operator()( _Tp ) const
        {
          return true;
        }
    };

    /** Convenience function for creating a True. */
    inline True true_c()
    { return True(); }

    /** Logical functor always \c false.
    */
    struct False
    {
      template<class _Tp>
        bool operator()( _Tp ) const
        {
          return false;
        }
    };

    /** Convenience function for creating a False. */
    inline False false_c()
    { return False(); }

    /** Logical functor inverting \a _Condition.
    */
    template<class _Condition>
      struct Not
      {
        Not( _Condition cond_r )
        : _cond( cond_r )
        {}

        template<class _Tp>
          bool operator()( _Tp t ) const
          {
            return ! _cond( t );
          }

        _Condition _cond;
      };

    /** Convenience function for creating a Not from \a _Condition. */
    template<class _Condition>
      inline Not<_Condition> not_c( _Condition cond_r )
      {
        return Not<_Condition>( cond_r );
      }

    /** Logical functor chaining \a _ACondition \c AND \a _BCondition.
    */
    template<class _ACondition, class _BCondition>
      struct Chain
      {
        Chain( _ACondition conda_r, _BCondition condb_r )
        : _conda( conda_r )
        , _condb( condb_r )
        {}

        template<class _Tp>
          bool operator()( _Tp t ) const
          {
            return _conda( t ) && _condb( t );
          }

        _ACondition _conda;
        _BCondition _condb;
      };

    /** Convenience function for creating a Chain from two conditions
     *  \a conda_r and \a condb_r.
    */
    template<class _ACondition, class _BCondition>
      inline Chain<_ACondition, _BCondition> chain( _ACondition conda_r, _BCondition condb_r )
      {
        return Chain<_ACondition, _BCondition>( conda_r, condb_r );
      }

    //@}
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace functor
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_FUNCTIONAL_H
