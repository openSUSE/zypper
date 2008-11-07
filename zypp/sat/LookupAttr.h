/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/LookupAttr.h
 *
*/
#ifndef ZYPP_SAT_LOOKUPATTR_H
#define ZYPP_SAT_LOOKUPATTR_H

extern "C"
{
struct _Dataiterator;
}
#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/DefaultIntegral.h"

#include "zypp/sat/Pool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr
    //
    /** Lightweight attribute value lookup.
     *
     * Search for an attribute in \ref Pool, one \ref Repository
     * or one \ref Solvable. \ref LookupAttr builds the query,
     * \ref LookupAttr::iterator iterates over the result.
     *
     * Modifying the query will not affect any running
     * iterator.
     *
     * Use \ref SolvAttr::allAttr to search all attributes.
     *
     * \code
     *  // look for all attributes of one solvable
     *  void ditest( sat::Solvable slv_r )
     *  {
     *    sat::LookupAttr q( sat::SolvAttr::allAttr, slv_r );
     *    MIL << q << ": " << endl;
     *    for_( it, q.begin(), q.end() )
     *    {
     *      MIL << "    " << it.inSolvAttr() << " = " << it.asString() << endl;
     *    }
     *  }
     * \endcode
     *
     * \code
     *  // look for an attribute in the pool.
     *  sat::LookupAttr q( sat::SolvAttr("susetags:datadir") );
     *  MIL << q << ": " << endl;
     *  for_( it, q.begin(), q.end() )
     *  {
     *    MIL << "    " << it << endl;
     *  }
     * \endcode
     */
    class LookupAttr
    {
      public:
        typedef unsigned size_type;

      public:
        /** Default ctor finds nothing. */
        LookupAttr()
        {}
        /** Lookup \ref SolvAttr in \ref Pool (all repositories). */
        explicit
        LookupAttr( SolvAttr attr_r )
        : _attr( attr_r )
        {}
        /** Lookup \ref SolvAttr in one\ref Repository. */
        explicit
        LookupAttr( SolvAttr attr_r, Repository repo_r )
        : _attr( attr_r ), _repo( repo_r )
        {}
        /** Lookup \ref SolvAttr in one \ref Solvable. */
        LookupAttr( SolvAttr attr_r, Solvable solv_r )
        : _attr( attr_r ), _solv( solv_r )
        {}

      public:
        /** \name Search result. */
        //@{
        /** Result iterator. */
        class iterator;

        /** Iterator to the begin of query results. */
        iterator begin() const;

        /** Iterator behind the end of query results. */
        iterator end() const;

        /** Whether the query is empty. */
        bool empty() const;

        /** Ammount of results.
         * \note This is not a cheap call. It runs the query.
        */
        size_type size() const;

        /** TransformIterator returning an \ref iterator vaue of type \c _ResultT. */
        template<class _ResultT, class _AttrT> class transformIterator;
        //@}

      public:
        /** \name What to search. */
        //@{

        /** The \ref SolvAttr to search. */
        SolvAttr attr() const
        { return _attr; }

        /** Set the \ref SolvAttr to search. */
        void setAttr( SolvAttr attr_r )
        { _attr = attr_r; }

        //@}
      public:
        /** \name Where to search. */
        //@{
        /** Wheter to search in \ref Pool. */
        bool pool() const
        { return ! (_repo || _solv); }

        /** Set search in \ref Pool (all repositories). */
        void setPool()
        {
          _repo = Repository::noRepository;
          _solv = Solvable::noSolvable;
        }

        /** Wheter to search in one \ref Repository. */
        Repository repo() const
        { return _repo; }

        /** Set search in one \ref Repository. */
        void setRepo( Repository repo_r )
        {
          _repo = repo_r;
          _solv = Solvable::noSolvable;
        }

        /** Wheter to search in one \ref Solvable. */
        Solvable solvable() const
        { return _solv; }

        /** Set search in one \ref Solvable. */
        void setSolvable( Solvable solv_r )
         {
          _repo = Repository::noRepository;
          _solv = solv_r;
        }

        //@}
      private:
        SolvAttr   _attr;
        Repository _repo;
        Solvable   _solv;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LookupAttr Stream output. */
    std::ostream & operator<<( std::ostream & str, const LookupAttr & obj );

    /** \relates LookupAttr Verbose stream output including the query result. */
    std::ostream & dumpOn( std::ostream & str, const LookupAttr & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr::iterator
    //
    /** Result iterator.
     * Extended iterator methods valid only if not @end.
     * \note Implementation: Keep iterator_adaptor base and _dip in sync!
     */
    class LookupAttr::iterator : public boost::iterator_adaptor<
        iterator                       // Derived
        , ::_Dataiterator *            // Base
        , detail::IdType               // Value
        , boost::forward_traversal_tag // CategoryOrTraversal
        , detail::IdType               // Reference
    >
    {
      public:
        /** \name Moving fast forward. */
        //@{
        /** On the next call to \ref operator++ advance to the next \ref SolvAttr. */
        void nextSkipSolvAttr();

        /** On the next call to \ref operator++ advance to the next \ref Solvable. */
        void nextSkipSolvable();

        /** On the next call to \ref operator++ advance to the next \ref Repository. */
        void nextSkipRepo();

        /** Immediately advance to the next \ref SolvAttr. */
        void skipSolvAttr()
        { nextSkipSolvAttr(); increment(); }

        /** Immediately advance to the next \ref Solvable. */
        void skipSolvable()
        { nextSkipSolvable(); increment(); }

        /** Immediately advance to the next \ref Repository. */
        void skipRepo()
        { nextSkipRepo(); increment(); }
        //@}

        /** \name Current position info. */
        //@{
        /** The current \ref Repository. */
        Repository inRepo() const;

        /** The current \ref Solvable. */
        Solvable inSolvable() const;

        /** The current \ref SolvAttr. */
        SolvAttr inSolvAttr() const;
        //@}

        /** \name Test attribute value type. */
        //@{
        /** The current \ref SolvAttr type. */
        detail::IdType solvAttrType() const;

        /** Whether this is a numeric attribute (incl. boolean). */
        bool solvAttrNumeric() const;

        /** Whether this is a string attribute. */
        bool solvAttrString() const;

        /** *Whether this string attribute is available as \ref IdString. */
        bool solvAttrIdString() const;

        /** Whether this is a CheckSum attribute.*/
        bool solvAttrCheckSum() const;
        //@}

        /** \name Retrieving attribute values. */
        //@{
        /** Conversion to numeric types. */
        int asInt() const;
        /** \overload */
        unsigned asUnsigned() const;
        /** \overload */
        bool asBool() const;

        /** Conversion to string types. */
        const char * c_str() const;
        /** \overload
         * If used with non-string types, this method tries to create
         * some appropriate string representation.
        */
        std::string asString() const;

        /** As \ref IdStr.
         * This is only done for poolized string types. Large strings like
         * summary or descriptions are not available via \ref IdStr, only
         * via \ref c_str and \ref asString.
         */
        IdString idStr() const;

        /** As \ref CheckSum. */
        CheckSum asCheckSum() const;

        /** Templated return type.
         * Specialized for supported types.
        */
        template<class _Tp> _Tp asType() const;
        //@}

        ///////////////////////////////////////////////////////////////////
        // internal stuff below
        ///////////////////////////////////////////////////////////////////
      public:
        iterator();

        iterator( const iterator & rhs );

        iterator & operator=( const iterator & rhs );

        ~iterator();

      public:
        /**
         * C-tor taking over ownership of the passed scoped _Dataiterator*
         * and doing it's first iteration (::dataiterator_step)
         */
        iterator( scoped_ptr< ::_Dataiterator> & dip_r );

      private:
        friend class boost::iterator_core_access;

        ::_Dataiterator * cloneFrom( const ::_Dataiterator * rhs );

        template <class OtherDerived, class OtherIterator, class V, class C, class R, class D>
        bool equal( const boost::iterator_adaptor<OtherDerived, OtherIterator, V, C, R, D> & rhs ) const
        {
          return ( bool(base()) == bool(rhs.base()) )
              && ( ! base() || dip_equal( *base(), *rhs.base() ) );
        }

        bool dip_equal( const ::_Dataiterator & lhs, const ::_Dataiterator & rhs ) const;

        detail::IdType dereference() const;

        void increment();

      public:
        /** Expert backdoor. */
        ::_Dataiterator * get() const
        { return _dip.get(); }
      private:
        scoped_ptr< ::_Dataiterator> _dip;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LookupAttr::iterator Stream output. */
    std::ostream & operator<<( std::ostream & str, const LookupAttr::iterator & obj );

    template<> inline int          LookupAttr::iterator::asType<int>()          const { return asInt(); }
    template<> inline unsigned     LookupAttr::iterator::asType<unsigned>()     const { return asUnsigned(); }
    template<> inline bool         LookupAttr::iterator::asType<bool>()         const { return asBool(); }
    template<> inline const char * LookupAttr::iterator::asType<const char *>() const { return c_str(); }
    template<> inline std::string  LookupAttr::iterator::asType<std::string>()  const { return asString(); }
    template<> inline IdString     LookupAttr::iterator::asType<IdString>()     const { return idStr(); }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr::transformIterator
    //
    /** TransformIterator returning an \ref iterator value of type \c _ResultT.
     *
     * The underlying LookupAttr::iterators value is retrieved \ref asType<_AttrT>
     * and the returned \ref ResultT is constructed fron that value.
     *
     * \code
     *   class Keywords
     *   {
     *     public:
     *       Keywords( sat::Solvable solv_r )
     *       : _q( sat::SolvAttr::keywords, solv_r )
     *       {}
     *
     *     public:
     *       typedef sat::LookupAttr::transformIterator<PackageKeyword,IdString> iterator;
     *
     *       iterator begin() const { return iterator( _q.begin() ); }
     *       iterator end() const   { return iterator( _q.end() ); }
     *
     *     private:
     *       sat::LookupAttr _q;
     *   };
     * \endcode
     *
     * \see \ref ArrayAttr.
     */
    template<class _ResultT, class _AttrT>
    class LookupAttr::transformIterator : public boost::iterator_adaptor<
          transformIterator<_ResultT,_AttrT> // Derived
          , LookupAttr::iterator         // Base
          , _ResultT                     // Value
          , boost::forward_traversal_tag // CategoryOrTraversal
          , _ResultT                     // Reference
    >
    {
      public:
        transformIterator()
        {}

        explicit
        transformIterator( const LookupAttr::iterator & val_r )
        { this->base_reference() = val_r; }

      public:

        /** \name Moving fast forward. */
        //@{
        /** On the next call to \ref operator++ advance to the next \ref SolvAttr. */
        void nextSkipSolvAttr()
        { this->base_reference().nextSkipSolvAttr(); }

        /** On the next call to \ref operator++ advance to the next \ref Solvable. */
        void nextSkipSolvable()
        { this->base_reference().nextSkipSolvable(); }

        /** On the next call to \ref operator++ advance to the next \ref Repository. */
        void nextSkipRepo()
        { this->base_reference().nextSkipRepo(); }

        /** Immediately advance to the next \ref SolvAttr. */
        void skipSolvAttr()
        { this->base_reference().skipSolvAttr(); }

        /** Immediately advance to the next \ref Solvable. */
        void skipSolvable()
        { this->base_reference().skipSolvable(); }

        /** Immediately advance to the next \ref Repository. */
        void skipRepo()
        { this->base_reference().skipRepo(); }
        //@}

        /** \name Current position info. */
        //@{
        /** The current \ref Repository. */
        Repository inRepo() const
        { return this->base_reference().inRepo(); }

        /** The current \ref Solvabele. */
        Solvable inSolvable() const
        { return this->base_reference().inSolvable(); }

        /** The current \ref SolvAttr. */
        SolvAttr inSolvAttr() const
        { return this->base_reference().inSolvAttr(); }
        //@}

      private:
        friend class boost::iterator_core_access;

        _ResultT dereference() const
        {
          const LookupAttr::iterator lit( this->base_reference() );
          return _ResultT( lit.asType<_AttrT>() );
        }
    };
    ///////////////////////////////////////////////////////////////////

    template<class _ResultT, class _AttrT>
    class ArrayAttr;

    template<class _ResultT, class _AttrT>
    std::ostream & operator<<( std::ostream & str, const ArrayAttr<_ResultT,_AttrT> & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ArrayAttr
    //
    /** \ref LookupAttr::transformIterator based container to retrieve list attributes.
     *
     * \code
     *  typedef ArrayAttr<PackageKeyword,IdString> Keywords;
     *  Keywords k( sat::SolvAttr::keywords );
     *  dumpRange( MIL << "All Keywords: ", k.begin(), k.end() ) << endl;
     * \endcode
     *
     * \todo Maybe add some way to unify the result.
     */
    template<class _ResultT, class _AttrT>
    class ArrayAttr
    {
      friend std::ostream & operator<< <_ResultT,_AttrT>( std::ostream & str, const ArrayAttr<_ResultT,_AttrT> & obj );

      public:
        ArrayAttr()
        {}

        ArrayAttr( SolvAttr attr_r )
        : _q( attr_r )
        {}

        ArrayAttr( SolvAttr attr_r, Repository repo_r )
        : _q( attr_r, repo_r )
        {}

        ArrayAttr( SolvAttr attr_r, Solvable solv_r )
        : _q( attr_r, solv_r )
        {}

      public:
        typedef sat::LookupAttr::transformIterator<_ResultT,_AttrT> iterator;

        iterator begin() const
        { return iterator( _q.begin() ); }

        iterator end() const
        { return iterator( _q.end() ); }

        bool empty() const
        { return _q.empty(); }

      public:

        iterator find( const _ResultT & key_r ) const
        {
          for_( it, begin(), end() )
          {
            if ( *it == key_r )
              return it;
          }
          return end();
        }

      private:
        sat::LookupAttr _q;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LookupAttr::iterator Stream output. */
    template<class _ResultT, class _AttrT>
    inline std::ostream & operator<<( std::ostream & str, const ArrayAttr<_ResultT,_AttrT> & obj )
    { return dumpOn( str, obj._q); }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_LOOKUPATTR_H
