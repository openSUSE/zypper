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

#include <iosfwd>
#include <set>
#include <map>

#include "zypp/base/Regex.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Function.h"

#include "zypp/sat/SolvIterMixin.h"
#include "zypp/sat/LookupAttr.h"
#include "zypp/sat/AttrMatcher.h"
#include "zypp/sat/Pool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace detail
  {
    class PoolQueryIterator;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : PoolQuery
  //
  /**
   * Meta-data query API. Returns solvables of specified kinds from specified
   * repositories with attributes matching the specified search strings.
   *
   * The search strings can be specified via \ref addString() and
   * \ref addAttribute() methods. String matching type can be set using the
   * setMatch*() methods. Multiple search strings for a particular attribute
   * will be combined into a regex (see \ref addString() and
   * \ref addAttribute() for more details).
   *
   * The begin() and end() methods return a PoolQueryIterator returning
   * \ref sat::Solvable objects which can easily be turned into \ref Resolvable
   * objects. Additionally, thanx to the \ref sat::SolvIterMixin, a Selectable
   * and PoolItem iterators are automatically available.
   *
   * <code>
   * PoolQuery q;
   * q.addAttribute(sat::SolvAttr::name, "zypp*");
   * q.addKind(ResKind::package);
   * q.setMatchGlob();
   *
   * for (PoolQuery::Selectable_iterator it = q.selectableBegin();
   *     it != q.selectableEnd(); ++it)
   * {
   *   ui::Selectable::constPtr s = *it;
   *   // ...
   * }
   * </code>
   *
   * Performance considerations
   *
   * Results of simple queries like those using one string and/or one attribute
   * and/or one repository are filtered by sat-solver's Dataiterator directly,
   * and thus it is fast.
   *
   * Queries with multiple strings are implemented using regexes. Queries based
   * on kinds, multiple repos, and multiple attributes are filtered inside
   * the PoolQuery, so these tend to be slower.
   *
   * \see tests/zypp/PoolQuery_test.cc for more examples
   * \see sat::SolvIterMixin
   */
  class PoolQuery : public sat::SolvIterMixin<PoolQuery, detail::PoolQueryIterator>
  {
  public:
    typedef std::set<std::string>                           StrContainer;
    typedef std::set<ResKind>                               Kinds;
    typedef std::map<sat::SolvAttr, StrContainer>           AttrRawStrMap;
    typedef std::map<sat::SolvAttr, std::string>            AttrCompiledStrMap;
    typedef std::map<sat::SolvAttr, str::regex>             AttrRegexMap;

    typedef detail::PoolQueryIterator                       const_iterator;
    typedef unsigned int                                    size_type;

  public:
    typedef function<bool( const sat::Solvable & )> ProcessResolvable;

    PoolQuery();
    ~PoolQuery();

    /** Query result accessers. */
    //@{
    /**
     * Compile the query and return an iterator to the result.
     *
     * \return An iterator (\ref detail::PoolQueryIterator) returning
     *         sat::Solvable objects pointing at the beginning of the query result.
     * \throws sat::MatchInvalidRegexException if the query was about to use a regex which
     *         failed to compile.
     *
     * \note Note that PoolQuery is derived from \ref sat::SolvIterMixin which
     *       makes PoolItem and Selectable iterators automatically available.
     * \see \ref sat::SolvIterMixin
     */
    const_iterator begin() const;

    /** An iterator pointing to the end of the query result. */
    const_iterator end() const;

    /** Whether the result is empty. */
    bool empty() const;

    /** Number of solvables in the query result. */
    size_type size() const;
    //@}

    /**
     * Executes the query with the current settings.
     * Results are yielded via the \a fnc callback.
     */
    void execute(ProcessResolvable fnc);

    /**
     * Filter by selectable kind.
     *
     * By default, all kinds will be returned. If addKind() is used,
     * only the specified kinds will be returned (multiple kinds will be ORed).
     *
     * Pass ResKind constants to this method, (e.g. ResKind::package).
     */
    void addKind(const ResKind & kind);

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
      ALL = 0, // both install filter and uninstall filter bits are 0
      INSTALLED_ONLY = 1,
      UNINSTALLED_ONLY = 2
    };

    /** Return only @System repo packages */
    void setInstalledOnly();
    /** Return only packages from repos other than @System. */
    void setUninstalledOnly();
    /** Set status filter directly \see StatusFilter */
    void setStatusFilterFlags( StatusFilter flags );

    //@}

    /**
     * Add a global query string. The string added via this method is applied
     * to all query attributes as if addAttribute(..., \value) was called
     * for all of them.
     *
     * This method can be used multiple times in which case the query strings
     * will be combined (together with strings added via addAttribute()) into
     * a regex. Searched attribute value will match this regex if <b>any</b>
     * of these strings will match the value. This can be changed by
     * (not yet implemented) \ref setRequireAll() method.
     */
    void addString(const std::string & value);

    /**
     * Filter by the \a value of the specified \a attr attribute. This can
     * be any of the available solvable attributes.
     *
     * This method can be used multiple times with the same \a attr in which
     * case the query strings will be combined (together with strings added
     * via addString()) into a regex. Searched attribute value will match
     * this regex if <b>any</b> of these strings will match the value.
     * This can be changed by (not yet implemented) \ref setRequireAll()
     * method.
     *
     * \note Though it is possible to use dependency attributes like
     * \ref Solv::Attr::provides here, note that the query string is
     * matched against a dependencies \c "name" part only. Any
     * <tt>"op edition"</tt> part of a \ref Capability is \b not
     * considered at all. \see \ref addDependency on how to query for
     * capabilities including edition ranges.
     *
     * \note Solvables of a kind not supporting the specified attribute will
     * <b>not</b> be returned.
     * \todo check the above
     *
     * \param attr Attribute identfier. Use sat::Solvattr::* constants
     * \param value What to search for.
     *
     * \see sat::SolvAttr
     */
    void addAttribute(const sat::SolvAttr & attr, const std::string & value = "");
#if 0
    /**
     * Query for dependencies matching a broken down capability.
     *
     * The capabilities \c name part may be defined as ordinary query
     * string (\see \ref addAttribute), so globing and regex are supported.
     * \code
     *   addDependency( sat::SolvAttr::provides, "kde*", Edition("2.0"), Rel::GE );
     * \endcode
     * \throws Exception in case \a attr is not a dependency attribute.
     */
    void addDependency( const sat::SolvAttr & attr, const std::string & name,
                        const Edition & edition, const Rel & op = Rel::EQ );
    /** \overload Query provides */
    void addProvides( const std::string & name, const Edition & edition, const Rel & op = Rel::EQ )
    { addDependency( sat::SolvAttr::provides, name, edition, op ); }
    /** \overload Query requires */
    void addRequires( const std::string & name, const Edition & edition, const Rel & op = Rel::EQ )
    { addDependency( sat::SolvAttr::requires, name, edition, op ); }
    /** \overload Query obsoletes */
    void addObsoletes( const std::string & name, const Edition & edition, const Rel & op = Rel::EQ )
    { addDependency( sat::SolvAttr::obsoletes, name, edition, op ); }
    /** \overload Query conflicts */
    void addConflicts( const std::string & name, const Edition & edition, const Rel & op = Rel::EQ )
    { addDependency( sat::SolvAttr::conflicts, name, edition, op ); }
    /** \overload Query recommends */
    void addRecommends( const std::string & name, const Edition & edition, const Rel & op = Rel::EQ )
    { addDependency( sat::SolvAttr::recommends, name, edition, op ); }
    /** \overload Query suggests */
    void addSuggests( const std::string & name, const Edition & edition, const Rel & op = Rel::EQ )
    { addDependency( sat::SolvAttr::suggests, name, edition, op ); }
    /** \overload Query supplements */
    void addSupplements( const std::string & name, const Edition & edition, const Rel & op = Rel::EQ )
    { addDependency( sat::SolvAttr::supplements, name, edition, op ); }
    /** \overload Query enhances */
    void addEnhances( const std::string & name, const Edition & edition, const Rel & op = Rel::EQ )
    { addDependency( sat::SolvAttr::enhances, name, edition, op ); }

    /** \overload Query taking a \ref Capability (always exact name match) */
    void addDependency( const sat::SolvAttr & attr, Capability cap_r );
#endif

    /**
     * Set version condition. This will filter out solvables not matching
     * <tt>solvableEdition \a op \a edition</tt>.
     *
     * \param edition Edition to look for.
     * \param op      Found-wanted relation operator.
     */
    void setEdition(const Edition & edition, const Rel & op = Rel::EQ);

    /** \name Text Matching Options
     * \note The implementation treats an empty search string as
     * <it>"match always"</it>. So if you want to actually match
     * an empty value, try <tt>( "^$", setMatchRegex )</tt>.
     */
    //@{
    /**
     * Turn case sentitivity on or off (unsets or sets \ref SEARCH_NOCASE flag).
     * PoolQuery defaults to case insensitive search unless this method
     * is used.
     *
     * \param value Whether to turn the case sensitivity on (default) or off.
     */
    void setCaseSensitive( bool value = true );

    /**
     * If set (default), look at the full path when searching in filelists.
     * Otherwise just match the the basenames.
     * \see \ref Match::FILES
     */
    void setFilesMatchFullPath( bool value = true );
    /** \overload */
    void setFilesMatchBasename( bool value = true )
    { setFilesMatchFullPath( !value ); }

    /** Set to match exact string instead of substring.*/
    void setMatchExact();
    /** Set to substring (the default). */
    void setMatchSubstring();
    /** Set to match globs. */
    void setMatchGlob();
    /** Set to use the query strings as regexes */
    void setMatchRegex();
    /** Set to match words (uses regex) */
    void setMatchWord();
    //void setLocale(const Locale & locale);
    //@}

    /**
     * Require that all of the values set by addString or addAttribute
     * match the values of respective attributes.
     *
     * \todo doesn't work yet, don't use this function
     */
    void setRequireAll( bool require_all = true );


    /** \name getters */
    //@{

    /** Search strings added via addString() */
    const StrContainer & strings() const;
    /**
     * Map (map<SolvAttr, StrContainer>) of attribute values added via
     * addAttribute(), addDep in string form */
    const AttrRawStrMap & attributes() const;

    const StrContainer & attribute(const sat::SolvAttr & attr) const;

    const Kinds & kinds() const;

    const StrContainer & repos() const;

    const Edition edition() const;
    const Rel editionRel() const;

    /**
     * returns true if search is case sensitive
     */
    bool caseSensitive() const;

    /** Whether searching in filelists looks at the full path or just at the basenames. */
    bool filesMatchFullPath() const;
    /** \overload */
    bool filesMatchBasename() const
    { return !filesMatchFullPath(); }

    bool matchExact() const;
    bool matchSubstring() const;
    bool matchGlob() const;
    bool matchRegex() const;
    bool matchWord() const;

    /** Returns string matching mode as enum.
     * \see \ref Match::Mode
     */
    Match::Mode matchMode() const
    { return flags().mode(); }

    /**
     * Whether all values added via addString() or addAttribute() are required
     * to match the values of the respective attributes.
     */
    bool requireAll() const;

    StatusFilter statusFilterFlags() const;
    //@}

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

    bool operator==(const PoolQuery& b) const;
    bool operator!=(const PoolQuery& b) const { return !(*this == b ); }

    // low level API

    /**
     * Free function to get the satsolver repo search
     * flags.
     *
     * \see \ref Match
     */
    Match flags() const;

    /**
     * Free function to set the satsolver repo search
     * flags.
     *
     * \see \ref Match
     */
    void setFlags( const Match & flags );

  public:
    class Impl;

    /** \deprecated unused, buggy and useless. */
    ZYPP_DEPRECATED void setMatchFiles() {}
    /** \deprecated unused, buggy and useless. */
    ZYPP_DEPRECATED bool matchFiles() const { return false; }
    /** \deprecated There should be no need for this internal value. To
     * switch across all match mode types, use the enum values returned
     * by \ref matchMode().  \see \ref Match::Mode.
     *
     */
    ZYPP_DEPRECATED int  matchType() const { return flags().modeval(); }


  private:
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PoolQuery Stream output. */
  std::ostream & operator<<( std::ostream & str, const PoolQuery & obj );


  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

  class PoolQueryMatcher;

  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : PoolQuery::PoolQueryIterator
  //
  /** \ref PoolQuery iterator as returned by \ref PoolQuery::begin.
  */
  class PoolQueryIterator : public boost::iterator_adaptor<
    PoolQueryIterator                  // Derived
    , sat::LookupAttr::iterator        // Base
    , const sat::Solvable              // Value
    , boost::forward_traversal_tag     // CategoryOrTraversal
    , const sat::Solvable              // Reference
  >
  {
    public:
      /** Default ctor is also \c end.*/
      PoolQueryIterator()
      {}

      /** \Ref PoolQuery ctor. */
      PoolQueryIterator( const shared_ptr<PoolQueryMatcher> & matcher_r )
      : _matcher( matcher_r )
      { increment(); }

    private:
      friend class boost::iterator_core_access;

      sat::Solvable dereference() const
      { return base_reference().inSolvable(); }

      void increment();

   private:
      shared_ptr<PoolQueryMatcher> _matcher;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PoolQueryIterator Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const PoolQueryIterator & obj )
  { return str << obj.base(); }

  ///////////////////////////////////////////////////////////////////
  } //namespace detail
  ///////////////////////////////////////////////////////////////////

  inline detail::PoolQueryIterator PoolQuery::end() const
  { return detail::PoolQueryIterator(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_POOLQUERY_H
