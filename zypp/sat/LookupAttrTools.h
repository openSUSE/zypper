/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/LookupAttrTools.h
 *
*/
#ifndef ZYPP_SAT_LOOKUPATTRTOOLS_H
#define ZYPP_SAT_LOOKUPATTRTOOLS_H

#include "zypp/sat/LookupAttr.h"
#include "zypp/Repository.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr::TransformIterator
    //
    /** TransformIterator returning an \ref iterator value of type \c TResult.
     *
     * The underlying LookupAttr::iterators value is retrieved \ref asType<TAttr>
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
     *       typedef sat::LookupAttr::TransformIterator<PackageKeyword,IdString> iterator;
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
    template<class TResult, class TAttr>
    class LookupAttr::TransformIterator : public boost::iterator_adaptor<
          TransformIterator<TResult,TAttr> // Derived
          , LookupAttr::iterator         // Base
          , TResult                      // Value
          , boost::forward_traversal_tag // CategoryOrTraversal
          , TResult                      // Reference
    >
    {
      public:
        TransformIterator()
        {}

        explicit
        TransformIterator( const LookupAttr::iterator & val_r )
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

        TResult dereference() const
        {
          const LookupAttr::iterator lit( this->base_reference() );
          return TResult( lit.asType<TAttr>() );
        }
    };
    ///////////////////////////////////////////////////////////////////

    template<class TResult, class TAttr>
    class ArrayAttr;

    template<class TResult, class TAttr>
    std::ostream & operator<<( std::ostream & str, const ArrayAttr<TResult,TAttr> & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ArrayAttr
    //
    /** \ref LookupAttr::TransformIterator based container to retrieve list attributes.
     *
     * You may pass \ref LookupAttr::REPO_ATTR as \ref LookupAttr::Location argument,
     * to lookup attributes associated with the \ref Repository as a whole
     * (e.g. repository keywords).
     *
     * \see \ref LookupAttr for details.
     *
     * \code
     *  typedef ArrayAttr<PackageKeyword,IdString> Keywords;
     *  Keywords k( sat::SolvAttr::keywords );
     *  dumpRange( MIL << "All Keywords: ", k.begin(), k.end() ) << endl;
     * \endcode
     *
     * \todo Maybe add some way to unify the result.
     */
    template<class TResult, class TAttr>
    class ArrayAttr
    {
      friend std::ostream & operator<< <TResult,TAttr>( std::ostream & str, const ArrayAttr<TResult,TAttr> & obj );

      public:
        ArrayAttr()
        {}

        ArrayAttr( SolvAttr attr_r, LookupAttr::Location loc_r = LookupAttr::SOLV_ATTR )
        : _q( attr_r, loc_r )
        {}

        ArrayAttr( SolvAttr attr_r, Repository repo_r, LookupAttr::Location loc_r = LookupAttr::SOLV_ATTR )
        : _q( attr_r, repo_r, loc_r )
        {}

        ArrayAttr( SolvAttr attr_r, Solvable solv_r )
        : _q( attr_r, solv_r )
        {}

      public:
        typedef LookupAttr::TransformIterator<TResult,TAttr> iterator;
        typedef LookupAttr::size_type size_type;

        iterator begin() const
        { return iterator( _q.begin() ); }

        iterator end() const
        { return iterator( _q.end() ); }

        bool empty() const
        { return _q.empty(); }

        size_type size() const
        {
          size_type count = 0;
          for_( it, begin(), end() )
            ++count;
          return count;
        }

      public:

        iterator find( const TResult & key_r ) const
        {
          for_( it, begin(), end() )
          {
            if ( *it == key_r )
              return it;
          }
          return end();
        }

      private:
        LookupAttr _q;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LookupAttr::iterator Stream output. */
    template<class TResult, class TAttr>
    inline std::ostream & operator<<( std::ostream & str, const ArrayAttr<TResult,TAttr> & obj )
    { return dumpOn( str, obj._q ); }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_LOOKUPATTRTOOLS_H
