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
#include "zypp/CapFilters.h"

#include "zypp/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace resfilter
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

    /** Select ResObject by kind. */
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

    /** Select ResObject by name. */
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


    /** Select ResObject by Edition using \a _Compare functor.
     *
     * Selects ResObject if <tt>_Compare( ResObject->edition(), _edition )<\tt>
     * is \c true.
     * \code
     * // use the convenience funktions to create ByEdition:
     *
     * byEdition( someedition ); // selects ResObjects with edition == someedition
     *
     * byEdition( someedition, CompareByGT<Edition>() ) //  edition >  someedition
     * \endcode
    */
    template<class _Compare = CompareByEQ<Edition> >
      struct ByEdition : public ResObjectFilterFunctor
      {
        ByEdition( const Edition & edition_r,
                   _Compare cmp_r )
        : _edition( edition_r )
        , _cmp( cmp_r )
        {}

        bool operator()( ResObject::constPtr p ) const
        {
          return _cmp( p->edition(), _edition );
        }

        Edition  _edition;
        _Compare _cmp;
      };

    /** */
    template<class _Compare>
      ByEdition<_Compare> byEdition( const Edition & edition_r, _Compare cmp_r )
      { return ByEdition<_Compare>( edition_r, cmp_r ); }

    /** */
    template<class _Compare>
      ByEdition<_Compare> byEdition( const Edition & edition_r )
      { return byEdition( edition_r, _Compare() ); }


    /** Select ResObject if at least one Capability with
     *  index \a index_r was found in dependency \a depType_r.
    */
    struct ByCapabilityIndex : public ResObjectFilterFunctor
    {
      ByCapabilityIndex( const std::string & index_r, Dep depType_r )
      : _dep( depType_r )
      , _index( index_r )
      {}
      ByCapabilityIndex( const Capability & cap_r, Dep depType_r )
      : _dep( depType_r )
      , _index( cap_r.index() )
      {}

      bool operator()( ResObject::constPtr p ) const
      {
        using capfilter::ByIndex;
        return(    make_filter_begin( ByIndex(_index), p->dep( _dep ) )
                != make_filter_end( ByIndex(_index), p->dep( _dep ) ) );
      }

      Dep         _dep;
      std::string _index;
    };

    ///////////////////////////////////////////////////////////////////

    typedef std::binary_function<PoolItem,Capability,
                                 bool> OnCapMatchCallbackFunctor;

    /** Find matching Capabilities in a ResObjects dependency and invoke a
     *  callback on matches.
     *
     * Iterates through the PoolItem (in fact the ResObject it holds)
     * CapSet denoted by \a dep_r. For each Capability matching the
     * provided \a cap_r the callback functor \a fnc_r is called with
     * the PoolItem and the PoolItem's matching Capability.
     *
     * \returns \c true, unless an invokation of the callback functor
     * returned \c false.
     *
     * \todo Unfortunately a pure PoolItem Filter, but woud be usefull with
     * plain ResObjects too. But the Solver urgently needs the PoolItem in
     * the OnCapMatchCallback.
    */
    template<class _OnCapMatchCallback>
      struct CallOnCapMatchIn
      {
        bool operator()( const PoolItem & p ) const
        {
          const CapSet & depSet( p->dep( _dep ) ); // dependency set in p to iterate
          capfilter::ByCapMatch matching( _cap );  // predicate: true if match with _cap

          int res
          = invokeOnEach( depSet.begin(), depSet.end(), // iterate this set
                          matching,                     // Filter: if match
                          std::bind1st(_fnc,p) );       // Action: invoke _fnc(p,match)
          // Maybe worth to note: Filter and Action are invoked with the same
          // iterator, thus Action will use the same capability that cause
          // the match in Filter.
          return ( res >= 0 );
        }

        CallOnCapMatchIn( Dep dep_r, const Capability & cap_r,
                          _OnCapMatchCallback fnc_r )
        : _dep( dep_r )
        , _cap( cap_r )
        , _fnc( fnc_r )
        {}
        Dep                 _dep;
        const Capability &  _cap;
        _OnCapMatchCallback _fnc;
      };

    /** */
    template<class _OnCapMatchCallback>
      inline CallOnCapMatchIn<_OnCapMatchCallback>
      callOnCapMatchIn( Dep dep_r, const Capability & cap_r,
                        _OnCapMatchCallback fnc_r )
      {
        return CallOnCapMatchIn<_OnCapMatchCallback>( dep_r, cap_r, fnc_r );
      }

    ///////////////////////////////////////////////////////////////////

    //@}
    /////////////////////////////////////////////////////////////////
  } // namespace resfilter
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESFILTERS_H
