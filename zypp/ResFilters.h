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

#include "zypp/base/Functional.h"
#include "zypp/Resolvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace functor
  { /////////////////////////////////////////////////////////////////
    /** \defgroup RESFILTERS Filter functors operating on ResObjects.
     * \ingroup g_Functor
     *
     * A simple filter is a function or functor matching the signature:
     * \code
     *   bool simplefilter( ResObject::Ptr );
     * \endcode
     *
     * \note It's not neccessary that your function or functor actually
     * returns \c bool. Anything which is convertible into a \c bool
     * will do;
     *
     * Besides basic filter functors which actually evaluate the
     * \c ResObject (e.g. \ref ByKind, \ref ByName) you may
     * use \ref LOGICALFILTERS to build more complex filters.
     *
     * \code
     * // some 'action' functor, printing and counting
     * // ResObjects.
     * struct PrintAndCount
     * {
     *   PrintAndCount( unsigned & counter_r )
     *   : _counter( counter_r )
     *   {}
     *
     *   bool operator()( ResObject::Ptr p ) const
     *   {
     *     DBG << *p << endl;
     *     ++_counter;
     *     return true;
     *   }
     *
     *   unsigned & _counter;
     * };
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
     * <tt>std:list\<ResObject::Ptr\>\&</tt> and fills it with the matches
     * found.
     *
     * But as a rule of thumb, a functor should be lightweight. If you
     * want to get data out, pass references to variables in (and assert
     * these variables live as long as the query lasts). Or use \ref FunctorRef.
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
    // Some ResObject attributes
    //
    ///////////////////////////////////////////////////////////////////

    /** */
    typedef std::unary_function<ResObject::constPtr, bool> ResObjectFilterFunctor;

    /** */
    struct ByKind : public ResObjectFilterFunctor
    {
      ByKind( const ResObject::Kind & kind_r )
      : _kind( kind_r )
      {}

      bool operator()( ResObject::constPtr p ) const
      {
        return p->kind() == _kind;
      }

      ResObject::Kind _kind;
    };

    /** */
    template<class _Res>
      inline ByKind byKind()
      { return ByKind( ResTraits<_Res>::kind ); }

    /** */
    struct ByName : public ResObjectFilterFunctor
    {
      ByName( const std::string & name_r )
      : _name( name_r )
      {}

      bool operator()( ResObject::constPtr p ) const
      {
        return p->name() == _name;
      }

      std::string _name;
    };

#if 0
    /** \todo enumerate dependencies. */
    struct ByCapabilityIndex : public ResObjectFilterFunctor
    {
      ByCapabilityIndex( const std::string & index_r )
      : _index( index_r )
      {}

      bool operator()( ResObject::constPtr p ) const
      {
        p->provides()

        return p->index() == _index;

      }

      std::string _index;
    };
#endif

    ///////////////////////////////////////////////////////////////////

    //@}
    /////////////////////////////////////////////////////////////////
  } // namespace functor
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESFILTERS_H
