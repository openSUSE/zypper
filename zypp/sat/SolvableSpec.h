/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/SolvableSpec.h
 */
#ifndef ZYPP_SAT_SOLVABLESPEC_H
#define ZYPP_SAT_SOLVABLESPEC_H

#include <iosfwd>

#include "zypp/APIConfig.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/String.h"

#include "zypp/sat/SolvableType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    ///////////////////////////////////////////////////////////////////
    /// \class SolvableSpec
    /// \brief Define a set of \ref Solvables by ident and provides.
    ///
    /// Able to keep the definition of solvable sets like in 'needreboot'
    /// or 'multiversion'. The associated file parser allows reading
    /// stored definitions (one `IDENT` or provides:CAPABILITY` per line;
    /// empty lines and lines starting with '#' are ignored).
    ///
    /// The use of provides requires re-computation of the solvable set,
    /// whenever the solvable pool changes. This computation via \ref WhatProvides
    /// is expensive, that's why a built in cache is also offered.
    ///
    /// \note \ref contains does not match srcpackage: per default.
    ///////////////////////////////////////////////////////////////////
    class SolvableSpec
    {
    public:
      /** Default ctor */
      SolvableSpec();

      /** Dtor */
      ~SolvableSpec();

    public:
      /** Add all \ref sat::Solvable with this \a ident_r */
      void addIdent( IdString ident_r );

      /** A all \ref sat::Solvable matching this \a provides_r. */
      void addProvides( Capability provides_r );

    public:
      /** Parse and add spec from a string (`IDENT` or provides:CAPABILITY`). */
      void parse( const C_Str & spec_r );

      /** Parse file \a istr_r and add it's specs (one per line, #-comments). */
      void parseFrom( const InputStream & istr_r );

      /** Parse and add specs from iterator range. */
      template <class TIterator>
      void parseFrom( TIterator begin, TIterator end )
      { for_( it, begin, end ) parse( *it ); }

      /** Convenience using \ref str::splitEscaped(", \t") to parse multiple specs from one line. */
      void splitParseFrom( const C_Str & multispec_r );

    public:
      /** Test whether \a solv_r matches the spec.
       * (Re-)builds the \ref WhatProvides cache on demand.
       *
       * \note Does not match srcpackage: per default.
       */
      bool contains( const sat::Solvable & solv_r ) const;
      /** \overload */
      template <class Derived>
      bool contains( const SolvableType<Derived> & solv_r ) const
      { return contains( solv_r.satSolvable() ); }

      /** Whether the cache is needed and dirty. */
      bool dirty() const;

      /** Explicitly flag the cache as dirty, so it will be rebuilt on the next request.
       * To be called if the \ref ResPool content has changed. Intentionally `const` as
       * this does not change the set of spects (though the set of matches might change).
       * \note This has no effect if no cache is needed (i.e. no provides: are used).
       */
      void setDirty() const;

    public:
      /** Whether \a ident_r has been added to the specs (mainly for parser tests). */
      bool containsIdent( const IdString & ident_r ) const;

      /** Whether \a provides_r has been added to the sepcs (mainly for parser tests).*/
      bool containsProvides( const Capability & provides_r ) const;

    public:
      class Impl;                 ///< Implementation class.
    private:
      RWCOW_pointer<Impl> _pimpl; ///< Pointer to implementation.
      friend std::ostream & operator<<( std::ostream & str, const SolvableSpec & obj );
    };

    /** \relates SolvableSpec Stream output */
    std::ostream & operator<<( std::ostream & str, const SolvableSpec & obj );

  } // namespace sat
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_SOLVABLESPEC_H
