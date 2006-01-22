/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResFilters.h
 *
*/
#ifndef ZYPP_RESFILTERS_H
#define ZYPP_RESFILTERS_H

#include <iosfwd>

#include "zypp/Resolvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace resfilter
  { /////////////////////////////////////////////////////////////////
    /** \defgroup RESFILTERS Filter functors operating on Resolvables.
     *
     * A simple filter is a function or functor matching the signature:
     * \code
     *   bool simplefilter( Resolvable::Ptr );
     * \endcode
     *
     * \note It's not neccessary that your function or functor actually
     * returns \c bool. Anything which is convertible into a \c bool
     * will do;
     *
     * Besides basic filter functors which actually evaluate the
     * \c Resolvable (e.g. \ref ByKind, \ref ByName), there are some
     * special functors to build more complex queries:
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
     * // some 'action' functor, printing and counting
     * // Resolvables.
     * struct PrintAndCount
     * {
     *   PrintAndCount( unsigned & counter_r )
     *   : _counter( counter_r )
     *   {}
     *
     *   bool operator()( Resolvable::Ptr p ) const
     *   {
     *     DBG << *p << endl;
     *     ++_counter;
     *     return true;
     *   }
     *
     *   unsigned & _counter;
     * };
     *
     *
     * ResStore store;
     * unsigned counter = 0;
     *
     * // print and count all resolvables
     * store.forEach( PrintAndCount(counter) );
     *
     * // print and count all resolvables named "kernel"
     * counter = 0;
     * store.forEach( ByName("kernel"), PrintAndCount(counter) );
     *
     * // print and count all Packages named "kernel"
     * counter = 0;
     * store.forEach( chain( ByKind(ResTraits<Package>::kind),
     *                       ByName("kernel") ),
     *                PrintAndCount(counter) );
     *
     * // print and count all Packages not named "kernel"
     * counter = 0;
     * store.forEach( chain( ByKind(ResTraits<Package>::kind),
     *                       not_c(ByName("kernel")) ),
     *                PrintAndCount(counter) );
     *
     * // same as above ;)
     * counter = 0;
     * store.forEach( chain( ByKind(ResTraits<Package>::kind),
     *                       chain( not_c(ByName("kernel")),
     *                              PrintAndCount(counter) ) ),
     *                true_c() );
     * \endcode
     *
     * As you can see in the last example there is no difference in using
     * a filter or an action functor, as both have the same signature.
     * A difference of course is the way forEach interprets the returned
     * value.
     *
     * Consequently you can netgate and chain actions as well. Thus
     * <tt>PrintAndCount(counter)</tt> could be
     * <tt>chain(Print(),Count(counter))</tt>, if these functors are
     * provided.
     *
     * \note These functors are not limited to be used with ResStore::forEach.
     * You can use them with std::algorithms as well.
     *
     * \note In case you already have functions or methods which do what you
     * want, but thet don't perfectly match the required signature: Make yourself
     * familiar with <tt>std::ptr_fun, mem_fun, bind1st, bind2nd and compose</tt>.
     * They are sometimes quite helpfull.
     *
     * \c PrintAndCount is an example how a functor can return data collected
     * during the query. You ca easily write a collector, that takes a
     * <tt>std:list\<Resolvable::Ptr\>\&</tt> and fills it with the matches
     * found.
     *
     * But as a rule of thumb, a functor should be lightweight. If you
     * want to get data out, pass references to variables in (and assert
     * these variables live as long as the quiery lasts).
     *
     * Internally all functors are passed by value. Thus it would not help
     * you to create an instance of some collecting functor, and pass it
     * to the query. The query will then fill a copy of your functor, you
     * won't get the data back. (Well, you probabely could, by using
     * boosr::ref).
     *
     * Why functors and not plain functions?
     *
     * You can use plain functions if they don't have to deliver data back to
     * the application.
     * The \c C-style approach is having functions that take a <tt>void * data</tt>
     * as last argument. This \c data pointer is then passed arround and casted
     * up and down.
     *
     * If you look at a functor, you'll see that it contains both, the function
     * to call (it's <tt>operator()</tt> ) and the data you'd otherwise pass as
     * <tt>void * data</tt>. That's nice and safe.
    */
    //@{
    ///////////////////////////////////////////////////////////////////
    //
    // Predefined filters
    //
    ///////////////////////////////////////////////////////////////////

    struct True
    {
      bool operator()( Resolvable::Ptr ) const
      {
        return true;
      }
    };

    True true_c()
    { return True(); }

    ///////////////////////////////////////////////////////////////////

    struct False
    {
      bool operator()( Resolvable::Ptr ) const
      {
        return false;
      }
    };

    False false_c()
    { return False(); }

    ///////////////////////////////////////////////////////////////////

    template<class _Condition>
      struct Not
      {
        Not( _Condition cond_r )
        : _cond( cond_r )
        {}
        bool operator()( Resolvable::Ptr p ) const
        {
          return ! _cond( p );
        }
        _Condition _cond;
      };

    template<class _Condition>
      Not<_Condition> not_c( _Condition cond_r )
      {
        return Not<_Condition>( cond_r );
      }

    ///////////////////////////////////////////////////////////////////

    template<class _ACondition, class _BCondition>
      struct Chain
      {
        Chain( _ACondition conda_r, _BCondition condb_r )
        : _conda( conda_r )
        , _condb( condb_r )
        {}
        bool operator()( Resolvable::Ptr p ) const
        {
          return _conda( p ) && _condb( p );
        }
        _ACondition _conda;
        _BCondition _condb;
      };

    template<class _ACondition, class _BCondition>
      Chain<_ACondition, _BCondition> chain( _ACondition conda_r, _BCondition condb_r )
      {
        return Chain<_ACondition, _BCondition>( conda_r, condb_r );
      }

    ///////////////////////////////////////////////////////////////////
    //
    // Now some Resolvable attributes
    //
    ///////////////////////////////////////////////////////////////////

    struct ByKind
    {
      ByKind( const Resolvable::Kind & kind_r )
      : _kind( kind_r )
      {}

      bool operator()( Resolvable::Ptr p ) const
      {
        return p->kind() == _kind;
      }
      Resolvable::Kind _kind;
    };

    template<class _Res>
      ByKind byKind()
      { return ByKind( ResTraits<_Res>::kind ); }

    struct ByName
    {
      ByName( const std::string & name_r )
      : _name( name_r )
      {}
      bool operator()( Resolvable::Ptr p ) const
      {
        return p->name() == _name;
      }
      std::string _name;
    };

    ///////////////////////////////////////////////////////////////////

    //@}
    /////////////////////////////////////////////////////////////////
  } // namespace resfilter
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESFILTERS_H
