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
#include <boost/functional.hpp>
#include <zypp/base/Function.h>

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
    /** \defgroup LOGICALFILTERS Functors for building compex queries.
     * \ingroup g_Functor
     *
     * Some logical functors to build more complex queries:
     *
     * \li \ref True and \ref False. No supprise, they always return
     *     \c true or \c false.
     * \li \ref Not\<TCondition\>. TCondition is a functor, and
     *     its result is inverted.
     * \li \ref Chain\<TACondition,TBCondition\>. \c TACondition and \c TBCondition
     *     are functors, and Chain evaluates <tt>TACondition && TBCondition</tt>.
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

    /* functor that always returns a copied value */
    template<class TConst>
    struct Constant
    {
      Constant( const TConst &value )
        : _value(value)
      {}

      template<class Tp>
      TConst operator()( Tp ) const
      { return _value; }

      TConst operator()() const
      { return _value; }

      TConst _value;
    };

    template<class TConst>
    inline Constant<TConst> constant( const TConst &value )
    { return Constant<TConst>(value); }

    /** Logical functor always \c true. */
    struct True
    {
      template<class Tp>
        bool operator()( Tp ) const
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
      template<class Tp>
        bool operator()( Tp ) const
        {
          return false;
        }
    };

    /** Convenience function for creating a False. */
    inline False false_c()
    { return False(); }

    /** Logical functor inverting \a TCondition.
    */
    template<class TCondition>
      struct Not
      {
        Not( TCondition cond_r )
        : _cond( cond_r )
        {}

        template<class Tp>
          bool operator()( Tp t ) const
          {
            return ! _cond( t );
          }

        TCondition _cond;
      };

    /** Convenience function for creating a Not from \a TCondition. */
    template<class TCondition>
      inline Not<TCondition> not_c( TCondition cond_r )
      {
        return Not<TCondition>( cond_r );
      }

    /** Logical functor chaining \a TACondition \c OR \a TBCondition.
    */
    template<class TACondition, class TBCondition>
      struct Or
      {
        Or( TACondition conda_r, TBCondition condb_r )
        : _conda( conda_r )
        , _condb( condb_r )
        {}

        template<class Tp>
          bool operator()( Tp t ) const
          {
            return _conda( t ) || _condb( t );
          }

        TACondition _conda;
        TBCondition _condb;
      };

    /** Convenience function for creating a Or from two conditions
     *  \a conda_r OR \a condb_r.
    */
    template<class TACondition, class TBCondition>
      inline Or<TACondition, TBCondition> or_c( TACondition conda_r, TBCondition condb_r )
      {
        return Or<TACondition, TBCondition>( conda_r, condb_r );
      }

    /** Logical functor chaining \a TACondition \c AND \a TBCondition.
    */
    template<class TACondition, class TBCondition>
      struct Chain
      {
        Chain( TACondition conda_r, TBCondition condb_r )
        : _conda( conda_r )
        , _condb( condb_r )
        {}

        template<class Tp>
          bool operator()( Tp t ) const
          {
            return _conda( t ) && _condb( t );
          }

        TACondition _conda;
        TBCondition _condb;
      };

    /** Convenience function for creating a Chain from two conditions
     *  \a conda_r and \a condb_r.
    */
    template<class TACondition, class TBCondition>
      inline Chain<TACondition, TBCondition> chain( TACondition conda_r, TBCondition condb_r )
      {
        return Chain<TACondition, TBCondition>( conda_r, condb_r );
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
    template<class Tp>
    struct GetFirst
    {
      GetFirst( Tp & result_r )
        : _result( &result_r )
      {}
      bool operator()( const Tp & val_r )
      { *_result = val_r; return false; }

      private:
        Tp * _result;
    };

    /** Convenience function for creating \ref GetFirst. */
    template<class Tp>
    GetFirst<Tp> getFirst( Tp & result_r )
    { return GetFirst<Tp>( result_r ); }


    /** Strore the last result found in the variable passed to the ctor.
     */
    template<class Tp>
    struct GetLast
    {
      GetLast( Tp & result_r )
        : _result( &result_r )
      {}
      bool operator()( const Tp & val_r )
      { *_result = val_r; return true; }

      private:
        Tp * _result;
    };

    /** Convenience function for creating \ref GetLast. */
    template<class Tp>
    GetLast<Tp> getLast( Tp & result_r )
    { return GetLast<Tp>( result_r ); }


    /** Store all results found to some output_iterator.
     * \code
     * std::vector<parser::ProductFileData> result;
     * parser::ProductFileReader::scanDir( functor::getAll( std::back_inserter( result ) ),
                                           sysRoot / "etc/products.d" );
     * \endcode
     */
    template<class TOutputIterator>
    struct GetAll
    {
      GetAll( TOutputIterator result_r )
        : _result( result_r )
      {}

      template<class Tp>
      bool operator()(  const Tp & val_r ) const
      { *(_result++) = val_r; return true; }

      private:
        mutable TOutputIterator _result;
    };

    /** Convenience function for creating \ref GetAll. */
    template<class TOutputIterator>
    GetAll<TOutputIterator> getAll( TOutputIterator result_r )
    { return GetAll<TOutputIterator>( result_r ); }

    //@}
    ///////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////
  } // namespace functor
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_FUNCTIONAL_H
