/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/PoolQuery.h
 *
*/
#ifndef ZYPP_POOLQUERY_H
#define ZYPP_POOLQUERY_H

#include "zypp/ui/Selectable.h"
#include "zypp/sat/SolvAttr.h"

#include "zypp/base/Function.h"

extern "C"
{
#include "satsolver/repo.h"
}

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////

  /**
   * Meta-data query API. Returns solvables of specified kinds from specified
   * repositories with attributes matching the specified search strings.
   *
   * TODO: details, examples.
   */
  class PoolQuery
  {
  public:
    typedef std::map<sat::SolvAttr, std::vector<std::string> > AttrMap;
    typedef std::map<sat::SolvAttr, std::string > CompiledAttrMap;
    typedef unsigned int size_type;

    class Impl;
    typedef Impl * ImplPtr;

    typedef ::_Dataiterator RepoDataIterator;

    class ResultIterator : public boost::iterator_adaptor<
      ResultIterator                     // Derived
      , RepoDataIterator *               // Base
      , sat::Solvable                    // Value
      , boost::forward_traversal_tag     // CategoryOrTraversal
      , sat::Solvable                    // Reference
    >
    {
    public:
      ResultIterator()
      : ResultIterator::iterator_adaptor_(0), _has_next(true),
        _attrs(CompiledAttrMap()), _do_matching(false), _pool((sat::Pool::instance()))
      { _rdit = 0; _sid = 0; }

    private:
      friend class boost::iterator_core_access;
      friend class PoolQuery;

      ResultIterator(ImplPtr pqimpl);

      sat::Solvable dereference() const
      {
        return _sid ? sat::Solvable(_sid) : sat::Solvable::noSolvable;
      }

      void increment();

      bool matchSolvable();

      template <class OtherDerived, class OtherIterator, class V, class C, class R, class D>
        bool equal( const boost::iterator_adaptor<OtherDerived, OtherIterator, V, C, R, D> & rhs ) const
      {
        if (!rhs.base() && !base())
          return true;
        if (!rhs.base() || !base())
          return false;
        if (rhs.base()->solvid == base()->solvid)
          return true;
        return false;
      }

    private:
      RepoDataIterator * _rdit;
      ImplPtr _pqimpl;
      /*SolvableId*/ int _sid;
      bool _has_next;
      const CompiledAttrMap & _attrs;
      bool _do_matching;
      sat::Pool _pool;
    };

  public:
    typedef function<bool( const sat::Solvable & )> ProcessResolvable;

    PoolQuery();
    ~PoolQuery();

    /** Query result accessers. */
    //@{

    /** */
    ResultIterator begin();
    /** */
    ResultIterator end();
    /** */
    bool empty();
    /** */
    size_type size();
    //@}

    /**
     * executes the query with the current settings
     * results are yielded on the callback passed on
     * construction
     */
    void execute(ProcessResolvable fnc);

    /**
     * Filter by selectable kind.
     *
     * By default, all kinds will be returned. If addKind() is used,
     * only the specified kinds will be returned (multiple kinds will be ORed).
     *
     * Pass ResTraits<T>::kind to this method, where T is one of the
     * \ref Resolvable child classes (e.g. ResTraits<Pattern>::kind).
     */
    void addKind(const Resolvable::Kind &kind);

    /**
     * Filter by repo.
     *
     * By default, all repos will be returned. If addRepo() is used,
     * only the specified repo will be returned (multiple repos will be ORed).
     */
    void addRepo(const std::string &repoalias);

    /** Installed status filter setters. */
    //@{

    /**
     * Filter by status (installed uninstalled)
     */
    enum StatusFilter {
      ALL = 1,
      INSTALLED_ONLY = 2,
      UNINSTALLED_ONLY = 4
    };
    void setInstalledOnly();
    void setUninstalledOnly();
    void setStatusFilterFlags( int flags );

    //@}

    /**
     * 
     */
    void addString(const std::string & value);

    /**
     * Filter by the \a value of any available solvable attribute.
     *
     * \note Solvables of a kind not supporting the specified attribute will
     * <b>not</b> be returned.
     * \todo check the above
     *
     * \param attr Attribute identfier. Use sat::Solvattr::* constants
     * \param value What to search for.
     */
    void addAttribute(const sat::SolvAttr & attr, const std::string & value = "");

    /**
     * Filter by Selectable status.
     *
     * This should cover also plain 'is installed' and 'not installed' statuses.
     *
     * \param status Selectable status (zypp::ui::Status enum)
     */
    //void addStatus(const Status status);

    /**
     * Add dependency filter.
     *
     * \param dtype   depenedcy type
     * \param name    depenency name
     * \param edition edition for a versioned dependency
     * \param rel     operand for a versioned dependency
     *
     * \todo maybe a isRegexp bool as in addName() for the name parameter would
     *       be handy here as well.
     * \todo add more addDependecy() variants
     *//*
    void addDependency(const Dep & dtype,
                       const std::string & name,
                       const Edition & edition = Edition(),
                       const Rel & rel = Rel::EQ);
*/


    /** \name Text Matching Options */
    //@{
    /**
     * Turn case sentitivity on or off (unsets or sets \ref SEARCH_NOCASE flag).
     * PoolQuery defaults to case insensitive search unless this method
     * is used.
     *
     * \param value Whether to turn the case sensitivity on (default) or off.
     */
    void setCaseSensitive(const bool value = true);

    /** Set to match exact string instead of substring.*/
    void setMatchExact();
    /** Set to use the query strings as regexes */
    void setMatchRegex();
    /** Set to substring (the default). */
    void setMatchSubstring();
    /** Set to match words (uses regex) */
    void setMatchWord();

    /**
     * If true, convert * and ? to correspondent regex equivalent, otherwise
     * shield wildcards from being interpreted like regex control chars (if
     * SEARCH_REGEX is set)
     */
    void setUseWildcards(const bool value = true);

    //void setLocale(const Locale & locale);
    //@}


    /**
     * Require that all of the query conditions set by
     * addAttribute, addString, addDep are satisfied.
     */
    void requireAll(const bool require_all = true);

    /**
     * Reads from stream query. Attributes is sepated by delim. Query is
     * separated by two delim.
     *
     * \param str input stream which contains query
     * \param delim delimeter for attributes
     * \return true if non-empty query is recovered
     *
     * \see readPoolQueriesFromFile
     */
    bool recover( std::istream &str, char delim = '\n' );

    /**
     * Writes a machine-readable string representation of the query to stream.
     * Use \a delim as attribute delimiter.
     *
     * \param str output stream to write to
     * \param delim delimiter for attributes
     *
     * \see writePoolQueriesToFile
     */
    void serialize( std::ostream &str, char delim = '\n' ) const;

    /** Return a human-readable description of the query */
    std::string asString() const;


    // low level API

    /**
     * Free function to set the satsolver repo search
     * flags.
     *
     * \see SEARCH_STRINGMASK
     * \see SEARCH_STRING
     * \see SEARCH_SUBSTRING
     * \see SEARCH_GLOB
     * \see SEARCH_REGEX
     * \see SEARCH_NOCASE
     * \see SEARCH_NO_STORAGE_SOLVABLE
     */
    void setFlags(int flags);

  private:
    /** Pointer to implementation */
    ImplPtr _pimpl;
  };


  /** \relates PoolQuery Stream output. */
  std::ostream & operator<<( std::ostream & str, const PoolQuery & obj );


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_POOLQUERY_H
