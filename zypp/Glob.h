/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Glob.h
 *
*/
#ifndef ZYPP_GLOB_H
#define ZYPP_GLOB_H

extern "C"
{
#include <glob.h>
}

#include <iosfwd>

#include "zypp/base/Easy.h"
#include "zypp/base/Flags.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/DefaultIntegral.h"

#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace filesystem
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Glob
    //
    /** Find pathnames matching a pattern.
     * \code
     * Glob glob( Glob::kBrace );
     * glob.add( "/somewhere/solverTestcase/ *{.xml,.xml.gz}" );
     * glob.add( "/somewhere/else/a*" );
     * for_( it, glob.begin(), glob.end() )
     *   ...
     * \endcode
     * \code
     * std::list<Pathname> plist;
     * Glob::collect( "/somewherre/solverTestcase/ *{.xml,.xml.gz}", Glob::kBrace,
     *                std::back_inserter( plist ) );
     * \endcode
     * \see Manual page glob(3)
     */
    class Glob : private base::NonCopyable
    {
      public:
        typedef size_t size_type;
        typedef const char * value_type;

        /** Iterate NULL terminated \c char* array. */
        class const_iterator : public boost::iterator_adaptor<
              const_iterator                // Derived
            , char **                       // Base
            , value_type                    // Value
            , boost::forward_traversal_tag  // CategoryOrTraversal
            , const value_type              // Reference
            >
        {
          public:
            const_iterator()
            : const_iterator::iterator_adaptor_( 0 )
            {}

            explicit const_iterator( char ** _idx )
            : const_iterator::iterator_adaptor_( _idx && *_idx ? _idx : 0 )
            {}

          private:
            friend class boost::iterator_core_access;
            void increment()
            {
              if ( base_reference() && !*(++base_reference()) )
                base_reference() = 0;
            }
            reference dereference() const
            { return( base() ? *base() : 0 ); }
        };
        ///////////////////////////////////////////////////////////////////

      public:
        /** Individual bits to combine in \ref Flags. */
        enum Bits {
          kErr		= GLOB_ERR,		//!< Return on read errors.
          kMark		= GLOB_MARK,		//!< Append a slash to each name.
          kNoSort	= GLOB_NOSORT,		//!< Don't sort the names.
          // unsupported _DOOFFS = GLOB_DOOFFS,	//!< Insert PGLOB->gl_offs NULLs.
          kNoCheck	= GLOB_NOCHECK,		//!< If nothing matches, return the pattern.
          // autoapplied _APPEND = GLOB_APPEND,	//!< Append to results of a previous call.
          kNoEscape	= GLOB_NOESCAPE,	//!< Backslashes don't quote metacharacters.
          kPeriod	= GLOB_PERIOD,		//!< Leading `.' can be matched by metachars.
          // unsupported _MAGCHAR = GLOB_MAGCHAR,//!< Set in gl_flags if any metachars seen.
          kAltDirFunc	= GLOB_ALTDIRFUNC,	//!< Use gl_opendir et al functions.
          kBrace	= GLOB_BRACE,		//!< Expand "{a,b}" to "a" "b".
          kNoMagic	= GLOB_NOMAGIC,		//!< If no magic chars, return the pattern.
          kTilde	= GLOB_TILDE,		//!< Expand ~user and ~ to home directories.
          kOnlyDir	= GLOB_ONLYDIR,		//!< Match only directories.
          kTildeCheck	= GLOB_TILDE_CHECK,	//!< Like GLOB_TILDE but return an error if the user name is not available.
        };

        /** type Flags: Type-safe OR-combination of \ref Bits. */
        ZYPP_DECLARE_FLAGS( Flags, Bits );

      public:
        /** Default ctor optionally taking the default flags.
         * The flags passed here are the default for \ref add.
         * \see \ref setDefaultFlags
        */
        Glob( Flags flags_r = Flags() )
        : _defaultFlags( flags_r )
        {}

        /** Ctor adding pathnames matching \a pattern_r.
         * The flags passed here are the default for \ref add.
         * \see \ref setDefaultFlags
        */
        explicit Glob( const Pathname & pattern_r, Flags flags_r = Flags() )
        : _defaultFlags( flags_r )
        { add( pattern_r, flags_r ); }
        /** \overload */
        explicit Glob( const std::string & pattern_r, Flags flags_r = Flags() )
        : _defaultFlags( flags_r )
        { add( pattern_r, flags_r ); }
        /** \overload */
        explicit Glob( const char * pattern_r, Flags flags_r = Flags() )
        : _defaultFlags( flags_r )
        { add( pattern_r, flags_r ); }

        /** Dtor */
        ~Glob()
        { if ( _result ) ::globfree( &(*_result) ); }

        /** Add pathnames matching \a pattern_r to the current result.
         *
         * Any flags passed here override the global default passed to
         * the ctor. GLOB_APPEND is atomatically added to the flags
         * f needed.
         *
         * This invalidates all iterators.
         * \see \ref setDefaultFlags
         * \return the value returned by ::glob().
         */
        int add( const Pathname & pattern_r, Flags flags_r = Flags() )
        { return add( pattern_r.c_str(), flags_r ); }
        /** \overload */
        int add( const std::string & pattern_r, Flags flags_r = Flags() )
        { return add( pattern_r.c_str(), flags_r ); }
        /** \overload */
        int add( const char * pattern_r, Flags flags_r = Flags() );

        /** Clear all results found so far. \ref defaultFlags remain active. */
        void clear();

        /** Clear all results and reset \ref defaultFlags. */
        void reset( Flags flags_r = Flags() )
        { clear(); setDefaultFlags( flags_r ); }


      public:
        /** The default flags passed to \c ::glob(). */
        Flags defaultFlags() const
        { return _defaultFlags; }

        /** Set the default flags passed to \c ::glob(). */
        void setDefaultFlags( Flags flags_r = Flags() )
        { _defaultFlags = flags_r; }

        /** Returns the value returned by the last call to \c ::glob().
         * \c Zero on successful completion. Otherwise \c GLOB_NOSPACE or \c GLOB_ABORTED
         * or \c GLOB_NOMATCH.
         */
        int lastGlobReturn() const
        { return _lastGlobReturn; }

      public:
        /** Whether matches were found. */
        bool empty() const
        { return ! ( _result && _result->gl_pathc ); }

        /** The number of matches found so far. */
        size_type size() const
        { return( _result ? _result->gl_pathc : 0 ); }

        /** Iterator pointing to the first result. */
        const_iterator begin() const
        { return( _result ? const_iterator( _result->gl_pathv ) : const_iterator() ); }

        /** Iterator pointing behind the last result. */
        const_iterator end() const
        { return const_iterator(); }

      public:

        /** \name Collecting Glob results to some _OutputIterator
         * \code
         * std::list<Pathname> p;
         * Glob::collect( "/bin/a*.dat}", std::back_inserter(p) );
         * Glob::collect( "/bin/a*{.xml,.xml.gz}", Glob::_BRACE, std::back_inserter(p) );
         * \endcode
         */
        //@{
        /** Write glob result to some \c OutputIterator. */
        template<class _OutputIterator>
        static int collect( const Pathname & pattern_r, _OutputIterator result_r )
        { return collect( pattern_r.c_str(), Flags(), result_r ); }
        /** \overload */
        template<class _OutputIterator>
        static int collect( const std::string & pattern_r, _OutputIterator result_r )
        { return collect( pattern_r.c_str(), Flags(), result_r ); }
        /** \overload */
        template<class _OutputIterator>
        static int collect( const char * pattern_r, _OutputIterator result_r )
        { return collect( pattern_r, Flags(), result_r ); }

        /** \overload With \ref Flags */
        template<class _OutputIterator>
        static int collect( const Pathname & pattern_r, Flags flags_r, _OutputIterator result_r )
        { return collect( pattern_r.c_str(), flags_r, result_r ); }
        /** \overload */
        template<class _OutputIterator>
        static int collect( const std::string & pattern_r, Flags flags_r, _OutputIterator result_r )
        { return collect( pattern_r.c_str(), flags_r, result_r ); }
        /** \overload */
        template<class _OutputIterator>
        static int collect( const char * pattern_r, Flags flags_r, _OutputIterator result_r )
        {
          Glob glob( pattern_r, flags_r );
          if ( glob.lastGlobReturn() == 0 )
            for_( it, glob.begin(), glob.end() )
              (*result_r)++ = typename _OutputIterator::container_type::value_type(*it);
          return glob.lastGlobReturn();
        }
        //@}

      private:
        Flags _defaultFlags;
        scoped_ptr< ::glob_t> _result;
        DefaultIntegral<int,0> _lastGlobReturn;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Glob Stream output */
    std::ostream & operator<<( std::ostream & str, const Glob & obj );

    /** \relates Glob::const_iterator Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Glob::const_iterator & obj )
    { return str << *obj; }

    ZYPP_DECLARE_OPERATORS_FOR_FLAGS( Glob::Flags );

    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace filesystem
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_GLOB_H
