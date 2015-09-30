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

#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/SolvAttr.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class CheckSum;
  class Match;
  class MatchException;
  class StrMatcher;

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
     * Per default \ref LookupAttr looks for attributes associated with
     * a \ref Solvable. But you may also pass \ref REPO_ATTR as
     * \ref Location argument, to lookup attributes associated with
     * the \ref Repository (e.g. DeltaRpm information).
     *
     * For convenience \see \ref LookupRepoAttr.
     *
     * Modifying the query will not affect any running
     * iterator.
     *
     * Use \ref SolvAttr::allAttr to search all attributes.
     *
     * To search for attributes located in a sub-structure (flexarray)
     * you also have to pass the sub-structures attribute as parent.
     * Passing \ref SolvAttr::allAttr a parent will lookup the attribute
     * in \c any sub-structure. Few attributes are known to have a parent
     * (\see \ref SolvAttr::parent). Setting those attributes will automatically
     * initialize their parent value.
     *
     * \code
     *    // Lookup all 'name' attributes:
     *    sat::LookupAttr q( sat::SolvAttr::name );
     *    // Lookup all 'name' attributes within a sub-structure 'data':
     *    sat::LookupAttr q( sat::SolvAttr::name, sat::SolvAttr::data );
     *    // Lookup all 'name' attributes within any sub-structure:
     *    sat::LookupAttr q( sat::SolvAttr::name, sat::SolvAttr::allAttr );
     * \endcode
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
     *
     * \code
     *  // look for a repo attribute in the pool.
     *  sat::LookupRepoAttr q( sat::SolvAttr::repositoryAddedFileProvides );
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
        typedef MatchException Exception;

      public:
        typedef unsigned size_type;

        /** Specify the where to look for the attribule. */
        enum Location {
          SOLV_ATTR = 0,  //!< Search for solvable attributes (default)
          REPO_ATTR = -1  //!< Search for repository attributes
        };

      public:
        /** Default ctor finds nothing. */
        LookupAttr();

        /** Lookup \ref SolvAttr in \ref Pool (all repositories). */
        explicit LookupAttr( SolvAttr attr_r, Location = SOLV_ATTR );
        /** \overload SolvAttr within sub-structure \a parent_r. */
        LookupAttr( SolvAttr attr_r, SolvAttr parent_r, Location = SOLV_ATTR );

        /** Lookup \ref SolvAttr in one\ref Repository. */
        LookupAttr( SolvAttr attr_r, Repository repo_r, Location = SOLV_ATTR );
        /** \overload SolvAttr within sub-structure \a parent_r. */
        LookupAttr( SolvAttr attr_r, SolvAttr parent_r, Repository repo_r, Location = SOLV_ATTR );

        /** Lookup \ref SolvAttr in one \ref Solvable. */
        LookupAttr( SolvAttr attr_r, Solvable solv_r );
        /** \overload SolvAttr within sub-structure \a parent_r. */
        LookupAttr( SolvAttr attr_r, SolvAttr parent_r, Solvable solv_r );

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

        /** TransformIterator returning an \ref iterator vaue of type \c TResult. */
        template<class TResult, class TAttr = TResult>
	class transformIterator;
        //@}

      public:
        /** \name What to search. */
        //@{
        /** The \ref SolvAttr to search. */
        SolvAttr attr() const;

        /** Set the \ref SolvAttr to search. */
        void setAttr( SolvAttr attr_r );
        //@}

        /** \name Restrict attributes to match a pattern. */
        //@{
        /** The pattern to match.
         * You can also evaluate \ref StrMatcher in a boolean context,
         * in order to test whether an \ref StrMatcher is set:
         * \code
         *   LookupAttr q;
         *   if ( q.strMatcher() )
         *     ...; // an StrMatcher is set
         * \endcode
        */
        const StrMatcher & strMatcher() const;

        /** Set the pattern to match.
         * \throws MatchException Any of the exceptions thrown by \ref StrMatcher::compile.
         */
        void setStrMatcher( const StrMatcher & matcher_r );

        /** Reset the pattern to match. */
        void resetStrMatcher();
       //@}

      public:
        /** \name Where to search. */
        //@{
        /** Whether to search in \ref Pool. */
        bool pool() const;

        /** Set search in \ref Pool (all repositories). */
        void setPool( Location = SOLV_ATTR );

        /** Whether to search in one \ref Repository. */
        Repository repo() const;

        /** Set search in one \ref Repository. */
        void setRepo( Repository repo_r, Location = SOLV_ATTR );

        /** Whether to search in one \ref Solvable. */
        Solvable solvable() const;

        /** Set search in one \ref Solvable. */
        void setSolvable( Solvable solv_r );

        /** Whether to search within a sub-structure (\ref SolvAttr::noAttr if not) */
        SolvAttr parent() const;

        /** Set search within a sub-structure (\ref SolvAttr::noAttr for none) */
        void setParent( SolvAttr attr_r );
        //@}

      private:
        class Impl;
        RWCOW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LookupAttr Stream output. */
    std::ostream & operator<<( std::ostream & str, const LookupAttr & obj );

    /** \relates LookupAttr Verbose stream output including the query result. */
    std::ostream & dumpOn( std::ostream & str, const LookupAttr & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupRepoAttr
    //
    /** Lightweight repository attribute value lookup.
     *
     * This is just a convenience class that overloads all
     * \ref LookupAttr methods which take a \ref LookupAttr::Location
     * argument and sets it to \ref REPO_ATTR.
     *
     * \code
     * // look for a repo attribute in the pool:
     * sat::LookupAttr     p( sat::SolvAttr::repositoryAddedFileProvides, sat::LookupAttr::REPO_ATTR );
     *
     * // Equivalent but using LookupRepoAttr:
     * sat::LookupRepoAttr q( sat::SolvAttr::repositoryAddedFileProvides );
     * \endcode
     *
     * \see \ref LookupAttr
     */
    class LookupRepoAttr : public LookupAttr
    {
      public:
        /** \copydoc LookupAttr::LookupAttr() */
        LookupRepoAttr()
        {}
        /** \copydoc LookupAttr::LookupAttr(SolvAttr) */
        explicit LookupRepoAttr( SolvAttr attr_r )
        : LookupAttr( attr_r, REPO_ATTR )
        {}
        /** \copydoc LookupAttr::LookupAttr(SolvAttr,Repository) */
        explicit LookupRepoAttr( SolvAttr attr_r, Repository repo_r );

      public:
        /** \copydoc LookupAttr::setPool */
        void setPool()
        { LookupAttr::setPool( REPO_ATTR ); }
        /** \copydoc LookupAttr::setRepo */
        void setRepo( Repository repo_r );
      private:
        // Hide. You can't look inside and outside Solvables at the same time.
        using LookupAttr::solvable;
        using LookupAttr::setSolvable;
    };
    ///////////////////////////////////////////////////////////////////

    namespace detail
    {
      /** Wrapper around sat \c ::_Dataiterator.
       *
       * Manages copy and assign, and release of allocated
       * resources like datamatcher inside the dataiterator.
       * Also maintains a copy of the matchstring in order to
       * keep the char* passed to the dataiterator valid.
       */
      class DIWrap
      {
        public:
          /** \c NULL \c ::_Dataiterator */
          DIWrap()
          : _dip( 0 )
          {}
          /** Initializes */
          DIWrap( RepoIdType repoId_r, SolvableIdType solvId_r, IdType attrId_r,
                  const std::string & mstring_r = std::string(), int flags_r = 0 );
          /** \overload to catch \c NULL \a mstring_r. */
          DIWrap( RepoIdType repoId_r, SolvableIdType solvId_r, IdType attrId_r,
                  const char * mstring_r, int flags_r = 0 );
          DIWrap( const DIWrap & rhs );
          ~DIWrap();
        public:
          void swap( DIWrap & rhs )
          {
            if ( &rhs != this ) // prevent self assign!
            {
              std::swap( _dip, rhs._dip );
              std::swap( _mstring, rhs._mstring );
            }
          }
          DIWrap & operator=( const DIWrap & rhs )
          {
            if ( &rhs != this ) // prevent self assign!
              DIWrap( rhs ).swap( *this );
            return *this;
          }
          void reset()
          { DIWrap().swap( *this ); }
        public:
          /** Evaluate in a boolean context <tt>( _dip != NULL )</tt>. */
          explicit operator bool() const
          { return _dip; }

        public:
          ::_Dataiterator * operator->() const  { return _dip; }
          ::_Dataiterator * get()        const  { return _dip; }
          const std::string & getstr()   const  { return _mstring; }

	private:
          ::_Dataiterator * _dip;
          std::string _mstring;
      };
      /** \relates DIWrap Stream output. */
      std::ostream & operator<<( std::ostream & str, const DIWrap & obj );
    }

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

        /** Stop after all matches in the current \ref Solvable are processed. */
        void stayInThisSolvable();

        /** Stop after all matches in the current \ref Repository are processed. */
        void stayInThisRepo();
        //@}

        /** \name Current position info. */
        //@{
        /** The current \ref Repository. */
        Repository inRepo() const;

        /** The current \ref Solvable. */
        Solvable inSolvable() const;

        /** The current \ref SolvAttr. */
        SolvAttr inSolvAttr() const;

        /** Whether this points to the end of a query (Iterator is invalid). */
        bool atEnd() const
        { return !_dip; }
        //@}

        /** \name Test attribute value type. */
        //@{
        /** The current \ref SolvAttr type. */
        detail::IdType solvAttrType() const;

        /** Whether this is a numeric attribute (incl. boolean). */
        bool solvAttrNumeric() const;

        /** Whether this is a string attribute. */
        bool solvAttrString() const;

        /** Whether this string attribute is available as \ref IdString. */
        bool solvAttrIdString() const;

        /** Whether this is a CheckSum attribute.*/
        bool solvAttrCheckSum() const;

        /** Whether this is the entry to a sub-structure (flexarray).
         * This is the entry to a sequence of attributes. To
         * acces them use \ref subBegin and  \ref subEnd.
        */
        bool solvAttrSubEntry() const;
        //@}

        /** \name Iterate sub-structures.
         *
         * These are usable iff \ref solvAttrSubEntry is \c true.
         *
         * \note Unfortunately the underlying libsolv dataiterator as returned
         * by \ref subBegin and \ref subFind loses some context when being created.
         * Thus it's not possible to invoke \ref subBegin and \ref subFind on an
         * iterator that was previously returned by one of those methods. The result
         * will be an \c end iterator. For the same reason it is not possible for an
         * iterator to leave the sub-structure again.
         *
         * \code
         * // Lookup all "update:reference" entries for a specific solvable
         * sat::LookupAttr q( sat::SolvAttr::updateReference, p->satSolvable() );
         * for_( res, q.begin(), q.end() )
         * {
         *   // List all sub values
         *   for_( sub, res.subBegin(), res.subEnd() )
         *   {
         *     cout << sub.asString() << endl;
         *   }
         *
         *   // Directly access c specific value:
         *   sat::LookupAttr::iterator it( res.subFind( sat::SolvAttr::updateReferenceHref ) );
         *   if ( it != res.subEnd() )
         *     cout << it.asString() << endl;
         * }
         * \endcode
         */
        //@{
        /** Whether the sub-structure is empty. */
        bool subEmpty() const;

        /** Ammount of attributes in the sub-structure.
         * \note This is not a cheap call. It runs the query.
        */
        size_type subSize() const;

        /** Iterator to the begin of a sub-structure.
         * \see \ref solvAttrSubEntry
        */
        iterator subBegin() const;
        /** Iterator behind the end of a sub-structure.
         * \see \ref solvAttrSubEntry
        */
        iterator subEnd() const;
         /** Iterator pointing to the first occurance of \ref SolvAttr \a attr_r in sub-structure.
          * If \ref sat::SolvAttr::allAttr is passed, \ref subBegin is returned.
          * \see \ref solvAttrSubEntry
         */
        iterator subFind( SolvAttr attr_r ) const;
        /** \overload Extending the current attribute name with by \c ":attrname_r".
         *
         * This assumes a sub-structur \c "update:reference" has attributes
         * like \c "update:reference:type", \c "update:reference:href".
         *
         * If an empty \c attrname_r is passed, \ref subBegin is returned.
        */
        iterator subFind( const C_Str & attrname_r ) const;
        //@}

        /** \name Retrieving attribute values. */
        //@{
        /** Conversion to numeric types. */
        int asInt() const;
        /** \overload */
        unsigned asUnsigned() const;
        /** \overload */
        bool asBool() const;
        /** \overload */
        unsigned long long asUnsignedLL() const;

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
	/** \overload Directly returning the \c Id */
        detail::IdType id() const
        { return idStr().id(); }

        /** As \ref CheckSum. */
        CheckSum asCheckSum() const;

        /** Templated return type.
         * Per default assumes an Id based type, so try to construct
         * it from the Id.
         *
         * Should be specialized for supported types above.
        */
        template<class Tp> Tp asType() const { return Tp(id()); }
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
         * C-tor taking over ownership of the passed \c ::_Dataiterator
         * and doing it's first iteration (::dataiterator_step)
         */
        iterator( detail::DIWrap & dip_r );

      private:
        friend class boost::iterator_core_access;

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
        detail::DIWrap _dip;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LookupAttr::iterator Stream output. */
    std::ostream & operator<<( std::ostream & str, const LookupAttr::iterator & obj );

    ///////////////////////////////////////////////////////////////////

    /** \name Helpers and forward declarations from LookupAttrTools.h */
    //@{
    template<> inline int          LookupAttr::iterator::asType<int>()          const { return asInt(); }
    template<> inline unsigned     LookupAttr::iterator::asType<unsigned>()     const { return asUnsigned(); }
    template<> inline unsigned long long LookupAttr::iterator::asType<unsigned long long>()     const { return asUnsignedLL(); }
    template<> inline bool         LookupAttr::iterator::asType<bool>()         const { return asBool(); }
    template<> inline const char * LookupAttr::iterator::asType<const char *>() const { return c_str(); }
    template<> inline std::string  LookupAttr::iterator::asType<std::string>()  const { return asString(); }
    template<> inline IdString     LookupAttr::iterator::asType<IdString>()     const { return idStr(); }
    template<>        CheckSum     LookupAttr::iterator::asType<CheckSum>()     const;

    template<class TResult, class TAttr>
    class ArrayAttr;
    //@}

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

/** \relates LookupAttr::iterator Stream output of the underlying iterator for debug. */
std::ostream & operator<<( std::ostream & str, const ::_Dataiterator * obj );

/** \relates LookupAttr::iterator Stream output of the underlying iterator for debug. */
inline std::ostream & operator<<( std::ostream & str, const ::_Dataiterator & obj )
{ return str << &obj; }

#endif // ZYPP_SAT_LOOKUPATTR_H
