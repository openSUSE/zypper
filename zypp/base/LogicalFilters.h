/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/LogicalFilters.h
 *
*/
#ifndef ZYPP_BASE_LOGICALFILTERS_H
#define ZYPP_BASE_LOGICALFILTERS_H

#include <functional>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace functor
  { /////////////////////////////////////////////////////////////////

    /** \defgroup LOGICALFILTERS Filter functors for building compex queries.
     * \ingroup g_Functor
     *
     * Some logical functors to build more complex queries:
     *
     * \li \ref True and \ref False. No supprise, they always return
     *     \c true or \c false.
     * \li \ref Not\<_Condition\>. _Condition is a filter functor, and
     *     it's result is inverted.
     * \li \ref Chain\<_ACondition,_BCondition\>. \c _ACondition and \c _BCondition
     *     are filter functors, and Chain evaluates
     *     <tt>_ACondition && _BCondition</tt>
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
    ///////////////////////////////////////////////////////////////////

    /** Logical filter always \c true. */
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

    ///////////////////////////////////////////////////////////////////

    /** Logical filter always \c false. */
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

    ///////////////////////////////////////////////////////////////////

    /** Logical filter inverting \a _Condition. */
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

    ///////////////////////////////////////////////////////////////////

    /** Logical filter chaining \a _ACondition \c AND \a _BCondition. */
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

    ///////////////////////////////////////////////////////////////////

    //@}
    /////////////////////////////////////////////////////////////////
  } // namespace functor
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_LOGICALFILTERS_H
