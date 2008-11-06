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

#include <boost/functional.hpp>

#include "zypp/base/Function.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /* http://www.boost.org/libs/functional/mem_fun.html

   The header functional.hpp includes improved versions of
   the full range of member function adapters from the
   C++ Standard Library.
  */
  using boost::mem_fun;
  using boost::mem_fun_ref;

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
     *   std::for_each( c.begin, c.end(),
     *                  // currently you must specify the
     *                  // operator() signature:
     *                  functorRef<void,SomeType>(counter)
     *                );
     * \endcode
     *
     * \note FunctorRef must be able to deduce the signature of
     * \c _Functor::operator(). This is currently not automated,
     * so you must specify the operator() signature as template
     * arguments.
     *
     * \note The order is <result_type, arg1_type, ...> (this
     * differs from std::, where the result comes last).
     *
     * \todo drop it an use boost::ref
    */

    /////////////////////////////////////////////////////////////////
    namespace functor_detail
    {
      template <class _Functor, class res_type>
        struct FunctorRef0
        {
          FunctorRef0( _Functor & f_r )
          : _f( f_r )
          {}

          res_type operator()() const
          {
          return _f();
          }

        private:
          _Functor & _f;
        };

      template <class _Functor, class res_type, class arg1_type>
        struct FunctorRef1 : public std::unary_function<arg1_type, res_type>
        {
          FunctorRef1( _Functor & f_r )
          : _f( f_r )
          {}

          res_type operator()( arg1_type a1 ) const
          {
            return _f( a1 );
          }

        private:
          _Functor & _f;
        };

      template <class _Functor, class res_type, class arg1_type, class arg2_type>
        struct FunctorRef2 : public std::binary_function<arg1_type, arg2_type, res_type>
        {
          FunctorRef2( _Functor & f_r )
          : _f( f_r )
          {}

          res_type operator()( arg1_type a1, arg2_type a2 ) const
          {
            return _f( a1, a2 );
          }

        private:
          _Functor & _f;
        };

      struct nil
      {};
    }
    /////////////////////////////////////////////////////////////////

    /** A binary \ref FunctorRef.
     * Create it using \ref functorRef convenience function.
    */
    template <class _Functor, class res_type, class arg1_type = functor_detail::nil,
                                              class arg2_type = functor_detail::nil>
      struct FunctorRef
      : public functor_detail::FunctorRef2<_Functor, res_type, arg1_type, arg2_type>
      {
        FunctorRef( _Functor & f_r )
        : functor_detail::FunctorRef2<_Functor, res_type, arg1_type, arg2_type>( f_r )
        {}
      };

    /** A unary \ref FunctorRef.
     * Create it using \ref functorRef convenience function.
    */
    template <class _Functor, class res_type, class arg1_type>
      struct FunctorRef<_Functor, res_type, arg1_type>
      : public functor_detail::FunctorRef1<_Functor, res_type, arg1_type>
      {
        FunctorRef( _Functor & f_r )
        : functor_detail::FunctorRef1<_Functor, res_type, arg1_type>( f_r )
        {}
      };

    /** A nullary \ref FunctorRef.
     * Create it using \ref functorRef convenience function.
    */
    template <class _Functor, class res_type>
      struct FunctorRef<_Functor, res_type>
      : public functor_detail::FunctorRef0<_Functor, res_type>
      {
        FunctorRef( _Functor & f_r )
        : functor_detail::FunctorRef0<_Functor, res_type>( f_r )
        {}
      };

    /** Convenience function creating a binary \ref FunctorRef. */
    template <class res_type, class arg1_type, class arg2_type, class _Functor>
      FunctorRef<_Functor, res_type, arg1_type, arg2_type>
      functorRef( _Functor & f_r )
      { return FunctorRef<_Functor, res_type, arg1_type, arg2_type>( f_r ); }
    template <class res_type, class arg1_type, class _Functor>
      FunctorRef<_Functor, res_type, arg1_type>
      functorRef( _Functor & f_r )
      { return FunctorRef<_Functor, res_type, arg1_type>( f_r ); }
    template <class res_type, class _Functor>
      FunctorRef<_Functor, res_type>
      functorRef( _Functor & f_r )
      { return FunctorRef<_Functor, res_type>( f_r ); }

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
     *  struct Print; // functor printing elements
     *  struct Count; // functor counting number of elements
     *
     *  std::for_each( c.begin(), c.end(),
     *                 chain( Print(), Count() ) );
     * \endcode
    */
    //@{

    /* functor that always returns a copied
       value */
    template<class T>
    struct Constant
    {
      Constant( const T &value )
        : _value(value)
      {}

      template<class _Tp>
      T operator()( _Tp ) const
      { return _value; }

      T operator()() const
      { return _value; }

      T _value;
    };

    template<class T>
    inline Constant<T> constant( const T &value )
    { return Constant<T>(value); }

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

    /** Logical functor chaining \a _ACondition \c OR \a _BCondition.
    */
    template<class _ACondition, class _BCondition>
      struct Or
      {
        Or( _ACondition conda_r, _BCondition condb_r )
        : _conda( conda_r )
        , _condb( condb_r )
        {}

        template<class _Tp>
          bool operator()( _Tp t ) const
          {
            return _conda( t ) || _condb( t );
          }

        _ACondition _conda;
        _BCondition _condb;
      };

    /** Convenience function for creating a Or from two conditions
     *  \a conda_r OR \a condb_r.
    */
    template<class _ACondition, class _BCondition>
      inline Or<_ACondition, _BCondition> or_c( _ACondition conda_r, _BCondition condb_r )
      {
        return Or<_ACondition, _BCondition>( conda_r, condb_r );
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

    /** \defgroup ACTIONFUNCTOR
     * \ingroup g_Functor
     */
    //@{

    /** Strore the 1st result found in the variable passed to the ctor.
     * \code
     *   PoolItem result;
     *   invokeOnEach( pool.byIdentBegin(installed), pool.byIdentEnd(installed),
     *                 filter::SameItem( installed ),
     *                 getFirst( result ) );
     * \endcode
     */
    template<class _Tp>
    struct GetFirst
    {
      GetFirst( _Tp & result_r )
        : _result( &result_r )
      {}
      bool operator()( const _Tp & val_r )
      { *_result = val_r; return false; }

      private:
        _Tp * _result;
    };

    /** Convenience function for creating \ref GetFirst. */
    template<class _Tp>
    GetFirst<_Tp> getFirst( _Tp & result_r )
    { return GetFirst<_Tp>( result_r ); }


    /** Strore the last result found in the variable passed to the ctor.
     */
    template<class _Tp>
    struct GetLast
    {
      GetLast( _Tp & result_r )
        : _result( &result_r )
      {}
      bool operator()( const _Tp & val_r )
      { *_result = val_r; return true; }

      private:
        _Tp * _result;
    };

    /** Convenience function for creating \ref GetLast. */
    template<class _Tp>
    GetLast<_Tp> getLast( _Tp & result_r )
    { return GetLast<_Tp>( result_r ); }


    /** Store all results found to some output_iterator.
     * \code
     * std::vector<parser::ProductFileData> result;
     * parser::ProductFileReader::scanDir( functor::getAll( std::back_inserter( result ) ),
                                           sysRoot / "etc/products.d" );
     * \endcode
     */
    template<class _OutputIterator>
    struct GetAll
    {
      GetAll( _OutputIterator result_r )
        : _result( result_r )
      {}

      template<class _Tp>
      bool operator()(  const _Tp & val_r ) const
      { *(_result++) = val_r; return true; }

      private:
        mutable _OutputIterator _result;
    };

    /** Convenience function for creating \ref GetAll. */
    template<class _OutputIterator>
    GetAll<_OutputIterator> getAll( _OutputIterator result_r )
    { return GetAll<_OutputIterator>( result_r ); }

    //@}
    ///////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////
  } // namespace functor
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_FUNCTIONAL_H
