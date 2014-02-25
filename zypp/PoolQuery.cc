/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PoolQuery.cc
 *
*/
#include <iostream>
#include <sstream>

#include "zypp/base/Gettext.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Algorithm.h"
#include "zypp/base/String.h"
#include "zypp/repo/RepoException.h"
#include "zypp/RelCompare.h"

#include "zypp/sat/Pool.h"
#include "zypp/sat/Solvable.h"
#include "zypp/base/StrMatcher.h"

#include "zypp/PoolQuery.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "PoolQuery"

using namespace std;
using namespace zypp::sat;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // some Helpers and Predicates
    /////////////////////////////////////////////////////////////////

    bool isDependencyAttribute( sat::SolvAttr attr_r )
    {
      static sat::SolvAttr deps[] = {
        SolvAttr::provides,
        SolvAttr::requires,
        SolvAttr::recommends,
        SolvAttr::obsoletes,
        SolvAttr::conflicts,
        SolvAttr::suggests,
        SolvAttr::supplements,
        SolvAttr::enhances,
      };
      for_( it, arrayBegin(deps), arrayEnd(deps) )
        if ( *it == attr_r )
          return true;
      return false;
    }

    /** Whether the current capabilities edition range ovelaps and/or its solvables arch matches.
     * Query asserts \a iter_r points to a capability and we
     * have to check the range only.
     */
    struct EditionRangePredicate
    {
      EditionRangePredicate( const Rel & op, const Edition & edition )
        : _range( op, edition )
        , _arch( Arch_empty )
      {}
      EditionRangePredicate( const Rel & op, const Edition & edition, const Arch & arch )
        : _range( op, edition )
        , _arch( arch )
      {}

      bool operator()( sat::LookupAttr::iterator iter_r )
      {
	if ( !_arch.empty() && iter_r.inSolvable().arch() != _arch )
	  return false;

        CapDetail cap( iter_r.id() );
        if ( ! cap.isSimple() )
          return false;
        if ( cap.isNamed() ) // no range to match
          return true;
        return overlaps( Edition::MatchRange( cap.op(), cap.ed() ), _range );
      }

      std::string serialize() const
      {
        std::string ret( "EditionRange" );
        str::appendEscaped( ret, _range.op.asString() );
        str::appendEscaped( ret, _range.value.asString() );
        str::appendEscaped( ret, _arch.asString() );
        return ret;
      }

      Edition::MatchRange _range;
      Arch                _arch;
   };

    /** Whether the current Solvables edition is within a given range and/or its arch matches. */
    struct SolvableRangePredicate
    {
      SolvableRangePredicate( const Rel & op, const Edition & edition )
        : _range( op, edition )
        , _arch( Arch_empty )
      {}

      SolvableRangePredicate( const Rel & op, const Edition & edition, const Arch & arch )
        : _range( op, edition )
        , _arch( arch )
      {}

      bool operator()( sat::LookupAttr::iterator iter_r )
      {
	if ( !_arch.empty() && iter_r.inSolvable().arch() != _arch )
	  return false;
	return overlaps( Edition::MatchRange( Rel::EQ, iter_r.inSolvable().edition() ), _range );
      }

      std::string serialize() const
      {
        std::string ret( "SolvableRange" );
        str::appendEscaped( ret, _range.op.asString() );
        str::appendEscaped( ret, _range.value.asString() );
        str::appendEscaped( ret, _arch.asString() );
        return ret;
      }

      Edition::MatchRange _range;
      Arch                _arch;
    };

    /** Whether the current capability matches a given one.
     * Query asserts \a iter_r points to a capability and we
     * have to check the match only.
     */
    struct CapabilityMatchPredicate
    {
      CapabilityMatchPredicate( Capability cap_r )
        : _cap( cap_r )
      {}

      bool operator()( sat::LookupAttr::iterator iter_r ) const
      {
        return _cap.matches( iter_r.asType<Capability>() ) == CapMatch::yes;
      }

      std::string serialize() const
      {
        std::string ret( "CapabilityMatch" );
        str::appendEscaped( ret, _cap.asString() );
        return ret;
      }

      Capability _cap;
    };

    /////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////
    /** Match data per attribtue.
     *
     * This includes the attribute itself, an optional \ref StrMatcher
     * to restrict the query to certain string values, and an optional
     * boolean \ref Predicate that may apply further restrictions that can
     * not be expressed by the \ref strMatcher.
     *
     * Example for such a \ref predicate would be an additional edition range
     * check whan looking for dependencies. The \ref strMatcher would
     * find potential matches by looking at the dependencies name, the
     * predicate will then check the edition ranges.
     *
     * As the \ref predicate takes an iterator pointing to the current
     * match, it's also suitable for sub-structure (flexarray) inspection
     * (\see \ref sat::LookupAttr::iterator::solvAttrSubEntry).
     *
     * \note: \see \ref addPredicate for further constraints.
     */
    struct AttrMatchData
    {
      typedef function<bool(sat::LookupAttr::iterator)> Predicate;

      static bool always( sat::LookupAttr::iterator ) { return true; }
      static bool never( sat::LookupAttr::iterator ) { return false; }

      AttrMatchData()
      {}

      AttrMatchData( sat::SolvAttr attr_r, const StrMatcher & strMatcher_r )
        : attr( attr_r )
        , strMatcher( strMatcher_r )
      {}

      AttrMatchData( sat::SolvAttr attr_r, const StrMatcher & strMatcher_r,
                     const Predicate & predicate_r, const std::string & predicateStr_r )
        : attr( attr_r )
        , strMatcher( strMatcher_r )
        , predicate( predicate_r )
        , predicateStr( predicateStr_r )
      {}

      /** A usable Predicate must provide a string serialization.
       * As there is no \c operator== for \ref Predicate, we compare it's
       * string representation instead. If you add new predicated, check the
       * deserialization code in \ref deserialize.
       */
      template<class _Predicate>
      void addPredicate( const _Predicate & predicate_r )
      {
        predicate    = predicate_r;
        predicateStr = predicate_r.serialize();
      }

      /** Dumb serialization.
       * \code
       *   AttrMatchData ATTRIBUTE SEARCHSTRING [C|X] SERIALIZED_PREDICATE
       * \endcode
      */
      std::string serialize() const
      {
        std::string ret( "AttrMatchData" );
        str::appendEscaped( ret, attr.asString() );
        str::appendEscaped( ret, strMatcher.searchstring() );
        // TODO: Actually the flag should be serialized too, but for PoolQuery
        // it's by now sufficient to differ between mode OTHER and others,
        // i.e. whether to compile or not compile.
        str::appendEscaped( ret, strMatcher.flags().mode() == Match::OTHER ? "C" : "X" );
        str::appendEscaped( ret, predicateStr );
        return ret;
      }

       /** Dumb restore from serialized string.
        * \throw Exception on parse error.
        */
      static AttrMatchData deserialize( const std::string & str_r )
      {
        std::vector<std::string> words;
        str::splitEscaped( str_r, std::back_inserter(words) );
        if ( words.empty() || words[0] != "AttrMatchData" )
          ZYPP_THROW( Exception( str::Str() << "Expecting AttrMatchData: " << str_r ) );
        if ( words.size() != 5 )
          ZYPP_THROW( Exception( str::Str() << "Wrong number of words: " << str_r ) );

        AttrMatchData ret;
        ret.attr = sat::SolvAttr( words[1] );
        ret.strMatcher = StrMatcher( words[2] );
        if ( words[3] == "C" )
          ret.strMatcher.setFlags( Match::OTHER );
        ret.predicateStr = words[4];

        // now the predicate
        words.clear();
        str::splitEscaped( ret.predicateStr, std::back_inserter(words) );
        if ( ! words.empty() )
        {
          if ( words[0] == "EditionRange" )
          {
	    switch( words.size() )
	    {
	      case 3:
		ret.predicate = EditionRangePredicate( Rel(words[1]), Edition(words[2]) );
		break;
	      case 4:
		ret.predicate = EditionRangePredicate( Rel(words[1]), Edition(words[2]), Arch(words[3]) );
		break;
	      default:
		ZYPP_THROW( Exception( str::Str() << "Wrong number of words: " << str_r ) );
		break;
	    }
          }
          else if ( words[0] == "SolvableRange" )
          {
	    switch( words.size() )
	    {
	      case 3:
		ret.predicate = SolvableRangePredicate( Rel(words[1]), Edition(words[2]) );
		break;
	      case 4:
		ret.predicate = SolvableRangePredicate( Rel(words[1]), Edition(words[2]), Arch(words[3]) );
		break;
	      default:
		ZYPP_THROW( Exception( str::Str() << "Wrong number of words: " << str_r ) );
		break;
	    }
          }
          else if ( words[0] == "CapabilityMatch" )
          {
            if ( words.size() != 2 )
              ZYPP_THROW( Exception( str::Str() << "Wrong number of words: " << str_r ) );
            ret.predicate = CapabilityMatchPredicate( Capability(words[1]) );
          }
          else
            ZYPP_THROW( Exception( str::Str() << "Unknown predicate: " << str_r ) );
        }
        return ret;
     }

      sat::SolvAttr    attr;
      StrMatcher strMatcher;
      Predicate        predicate;
      std::string      predicateStr;
    };

    /** \relates AttrMatchData */
    inline std::ostream & operator<<( std::ostream & str, const AttrMatchData & obj )
    {
      str << obj.attr << ": " << obj.strMatcher;
      if ( obj.predicate )
        str << " +(" << obj.predicateStr << ")";
      return str;
    }

    /** \relates AttrMatchData */
    inline bool operator==( const AttrMatchData & lhs, const AttrMatchData & rhs )
    {
      return ( lhs.attr == rhs.attr
               && lhs.strMatcher == rhs.strMatcher
               && lhs.predicateStr == rhs.predicateStr );
    }

    /** \relates AttrMatchData */
    inline bool operator!=( const AttrMatchData & lhs, const AttrMatchData & rhs )
    { return !( lhs == rhs ); }

    /** \relates AttrMatchData Arbitrary order for std::container. */
    inline bool operator<( const AttrMatchData & lhs, const AttrMatchData & rhs )
    {
      if ( lhs.attr != rhs.attr )
        return (  lhs.attr < rhs.attr );
      if ( lhs.strMatcher != rhs.strMatcher )
        return (  lhs.strMatcher < rhs.strMatcher );
      if ( lhs.predicateStr != rhs.predicateStr )
        return (  lhs.predicateStr < rhs.predicateStr );
      return false;
    }

    typedef std::list<AttrMatchData> AttrMatchList;


  } /////////////////////////////////////////////////////////////////
  // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : PoolQuery::Impl
  //
  /** */
  class PoolQuery::Impl
  {
  public:
    Impl()
      : _flags( Match::SUBSTRING | Match::NOCASE | Match::SKIP_KIND )
      , _match_word(false)
      , _require_all(false)
      , _status_flags(ALL)
    {}

    ~Impl()
    {}

  public:
    /** String representation */
    string asString() const;

    /** \name Raw query options. */
    //@{
    /** Raw search strings. */
    StrContainer _strings;
    /** Raw attributes */
    AttrRawStrMap _attrs;
    /** Uncompiled attributes with predicate. */
    std::set<AttrMatchData> _uncompiledPredicated;

    /** Sat solver search flags */
    Match _flags;
    bool _match_word;
    bool _require_all;

    /** Sat solver status flags */
    StatusFilter _status_flags;

    /** Edition condition operand */
    Edition _edition;
    /** Operator for edition condition */
    Rel _op;

    /** Repos to search. */
    StrContainer _repos;

    /** Kinds to search */
    Kinds _kinds;
    //@}

  public:

    bool operator==( const PoolQuery::Impl & rhs ) const
    {
      if ( _flags == rhs._flags
	// bnc#792901: while libzypp uses exact match mode for a single
	// package name lock, zypper always uses glob. :(
	// We unify those two forms to enable zypper to remove zypp locks
	// without need to actually evaluate the query (which would require
	// repos to be loaded).
	|| ( ( ( _flags.isModeString() && rhs._flags.isModeGlob() )
	    || ( _flags.isModeGlob() && rhs._flags.isModeString() ) )
	  && _strings.empty()
	  && _attrs.size() == 1
	  && _attrs.begin()->first == sat::SolvAttr::name ) )
      {
	return ( _strings == rhs._strings
	      && _attrs == rhs._attrs
	      && _uncompiledPredicated == rhs._uncompiledPredicated
	      && _match_word == rhs._match_word
	      && _require_all == rhs._require_all
	      && _status_flags == rhs._status_flags
	      && _edition == rhs._edition
	      && _op == rhs._op
	      && _repos == rhs._repos
	      && _kinds == rhs._kinds );
      }
      return false;
    }

    bool operator!=( const PoolQuery::Impl & rhs ) const
    { return ! operator==( rhs ); }

  public:
    /** Compile the regex.
     * Basically building the \ref _attrMatchList from strings.
     * \throws MatchException Any of the exceptions thrown by \ref StrMatcher::compile.
     */
    void compile() const;

    /** StrMatcher per attribtue. */
    mutable AttrMatchList _attrMatchList;

  private:
    /** Pass flags from \ref compile, as they may have been changed. */
    string createRegex( const StrContainer & container, const Match & flags ) const;

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };

  ///////////////////////////////////////////////////////////////////

  struct MyInserter
  {
    MyInserter(PoolQuery::StrContainer & cont) : _cont(cont) {}

    bool operator()(const string & str)
    {
      _cont.insert(str);
      return true;
    }

    PoolQuery::StrContainer & _cont;
  };


  struct EmptyFilter
  {
    bool operator()(const string & str)
    {
      return !str.empty();
    }
  };

  void PoolQuery::Impl::compile() const
  {
    _attrMatchList.clear();

    Match cflags( _flags );
    if ( cflags.mode() == Match::OTHER ) // this will never succeed...
      ZYPP_THROW( MatchUnknownModeException( cflags ) );

    /** Compiled search strings. */
    string rcstrings;


    // 'different'         - will have to iterate through all and match by ourselves (slow)
    // 'same'              - will pass the compiled string to dataiterator_init
    // 'one-attr'          - will pass it to dataiterator_init
    // 'one-non-regex-str' - will pass to dataiterator_init, set flag to SEARCH_STRING or SEARCH_SUBSTRING

    // // NO ATTRIBUTE
    // else
    //   for all _strings
    //     create regex; store in rcstrings; if more strings flag regex;
    if (_attrs.empty())
    {
      ; // A default 'query-all' will be added after all sources are processed.
    }

    // // ONE ATTRIBUTE
    // else if _attrs is not empty but it contains just one attr
    //   for all _strings and _attr[key] strings
    //     create regex; flag 'one-attr'; if more strings flag regex;
    else if (_attrs.size() == 1)
    {
      StrContainer joined;
      invokeOnEach(_strings.begin(), _strings.end(), EmptyFilter(), MyInserter(joined));
      invokeOnEach(_attrs.begin()->second.begin(), _attrs.begin()->second.end(), EmptyFilter(), MyInserter(joined));
      rcstrings = createRegex(joined, cflags);
      if (joined.size() > 1) // switch to regex for multiple strings
        cflags.setModeRegex();
      _attrMatchList.push_back( AttrMatchData( _attrs.begin()->first,
                                StrMatcher( rcstrings, cflags ) ) );
    }

    // // MULTIPLE ATTRIBUTES
    else
    {
      // check whether there are any per-attribute strings
      bool attrvals_empty = true;
      for (AttrRawStrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai)
        if (!ai->second.empty())
          for(StrContainer::const_iterator it = ai->second.begin();
              it != ai->second.end(); it++)
            if (!it->empty())
            {
              attrvals_empty = false;
              goto attremptycheckend;
            }
attremptycheckend:

      // chceck whether the per-attribute strings are all the same
      bool attrvals_thesame = true;
      AttrRawStrMap::const_iterator ai = _attrs.begin();
      const StrContainer & set1 = ai->second;
      ++ai;
      for (; ai != _attrs.end(); ++ai)
      {
        StrContainer result;
        set_difference(
          set1.begin(), set1.end(),
          ai->second.begin(), ai->second.end(),
          inserter(result, result.begin())/*, ltstr()*/);
        if (!result.empty())
        {
          attrvals_thesame = false;
          break;
        }
      }

      // // THE SAME STRINGS FOR DIFFERENT ATTRS
      // else if _attrs is not empty but it does not contain strings
      //   for each key in _attrs take all _strings
      //     create regex; store in rcstrings; flag 'same'; if more strings flag regex;
      if (attrvals_empty || attrvals_thesame)
      {
        StrContainer joined;
        if (attrvals_empty)
        {
          invokeOnEach(_strings.begin(), _strings.end(), EmptyFilter(), MyInserter(joined));
          rcstrings = createRegex(joined, cflags);
        }
        else
        {
          invokeOnEach(_strings.begin(), _strings.end(), EmptyFilter(), MyInserter(joined));
          invokeOnEach(_attrs.begin()->second.begin(), _attrs.begin()->second.end(), EmptyFilter(), MyInserter(joined));
          rcstrings = createRegex(joined, cflags);
        }
        if (joined.size() > 1) // switch to regex for multiple strings
          cflags.setModeRegex();
        // May use the same StrMatcher for all
        StrMatcher matcher( rcstrings, cflags );
        for_( ai, _attrs.begin(), _attrs.end() )
        {
          _attrMatchList.push_back( AttrMatchData( ai->first, matcher ) );
        }
      }

      // // DIFFERENT STRINGS FOR DIFFERENT ATTRS
      // if _attrs is not empty and it contains non-empty vectors with non-empty strings
      //   for each key in _attrs take all _strings + all _attrs[key] strings
      //     create regex; flag 'different'; if more strings flag regex;
      else
      {
        for_(ai, _attrs.begin(), _attrs.end())
        {
          StrContainer joined;
          invokeOnEach(_strings.begin(), _strings.end(), EmptyFilter(), MyInserter(joined));
          invokeOnEach(ai->second.begin(), ai->second.end(), EmptyFilter(), MyInserter(joined));
          string s = createRegex(joined, cflags);
          if (joined.size() > 1) // switch to regex for multiple strings
            cflags.setModeRegex();
          _attrMatchList.push_back( AttrMatchData( ai->first,
                                    StrMatcher( s, cflags ) ) );
        }
      }
    }

    // Now handle any predicated queries
    if ( ! _uncompiledPredicated.empty() )
    {
      StrContainer global;
      invokeOnEach( _strings.begin(), _strings.end(), EmptyFilter(), MyInserter(global) );
      for_( it, _uncompiledPredicated.begin(), _uncompiledPredicated.end() )
      {
        if ( it->strMatcher.flags().mode() == Match::OTHER )
        {
          // need to compile:
          StrContainer joined( global );
          const std::string & mstr( it->strMatcher.searchstring() );
          if ( ! mstr.empty() )
            joined.insert( mstr );

          cflags = _flags;
          rcstrings = createRegex( joined, cflags );
          if ( joined.size() > 1 ) // switch to regex for multiple strings
            cflags.setModeRegex();

          _attrMatchList.push_back( AttrMatchData( it->attr,
                                    StrMatcher( rcstrings, cflags ),
                                                      it->predicate, it->predicateStr ) );
        }
        else
        {
          // copy matcher
         _attrMatchList.push_back( *it );
        }
      }
    }

    // If no attributes defined at all, then add 'query all'
    if ( _attrMatchList.empty() )
    {
      cflags = _flags;
      rcstrings = createRegex( _strings, cflags );
      if ( _strings.size() > 1 ) // switch to regex for multiple strings
        cflags.setModeRegex();
      _attrMatchList.push_back( AttrMatchData( sat::SolvAttr::allAttr,
                                StrMatcher( rcstrings, cflags ) ) );
    }

    // Finally check here, whether all involved regex compile.
    for_( it, _attrMatchList.begin(), _attrMatchList.end() )
    {
      it->strMatcher.compile(); // throws on error
    }
    //DBG << asString() << endl;
  }


  /**
   * Converts '*' and '?' wildcards within str into their regex equivalents.
   */
  static string wildcards2regex(const string & str)
  {
    string regexed = str;

    string r_all(".*"); // regex equivalent of '*'
    string r_one(".");  // regex equivalent of '?'
    string::size_type pos;

    // replace all "*" in input with ".*"
    for (pos = 0; (pos = regexed.find("*", pos)) != std::string::npos; pos+=2)
      regexed = regexed.replace(pos, 1, r_all);

    // replace all "?" in input with "."
    for (pos = 0; (pos = regexed.find('?', pos)) != std::string::npos; ++pos)
      regexed = regexed.replace(pos, 1, r_one);

    return regexed;
  }

  string PoolQuery::Impl::createRegex( const StrContainer & container, const Match & flags ) const
  {
//! macro for word boundary tags for regexes
#define WB (_match_word ? string("\\b") : string())
    string rstr;

    if (container.empty())
      return rstr;

    if (container.size() == 1)
    {
      return WB + *container.begin() + WB;
    }

    // multiple strings

    bool use_wildcards = flags.isModeGlob();
    StrContainer::const_iterator it = container.begin();
    string tmp;

    if (use_wildcards)
      tmp = wildcards2regex(*it);
    else
      tmp = *it;

    if (_require_all)
    {
      if ( ! flags.isModeString() ) // not match exact
        tmp += ".*" + WB + tmp;
      rstr = "(?=" + tmp + ")";
    }
    else
    {
      if ( flags.isModeString() || flags.isModeGlob() )
        rstr = "^";
      rstr += WB + "(" + tmp;
    }

    ++it;

    for (; it != container.end(); ++it)
    {
      if (use_wildcards)
        tmp = wildcards2regex(*it);
      else
        tmp = *it;

      if (_require_all)
      {
        if ( ! flags.isModeString() ) // not match exact
          tmp += ".*" + WB + tmp;
        rstr += "(?=" + tmp + ")";
      }
      else
      {
        rstr += "|" + tmp;
      }
    }

    if (_require_all)
    {
      if ( ! flags.isModeString() ) // not match exact
        rstr += WB + ".*";
    }
    else
    {
      rstr += ")" + WB;
      if ( flags.isModeString() || flags.isModeGlob() )
        rstr += "$";
    }

    return rstr;
#undef WB
  }

  string PoolQuery::Impl::asString() const
  {
    ostringstream o;

    o << "kinds: ";
    if ( _kinds.empty() )
      o << "ALL";
    else
    {
      for(Kinds::const_iterator it = _kinds.begin();
          it != _kinds.end(); ++it)
        o << *it << " ";
    }
    o << endl;

    o << "repos: ";
    if ( _repos.empty() )
      o << "ALL";
    else
    {
      for(StrContainer::const_iterator it = _repos.begin();
          it != _repos.end(); ++it)
        o << *it << " ";
    }
    o << endl;

    o << "version: "<< _op << " " << _edition.asString() << endl;
    o << "status: " << ( _status_flags ? ( _status_flags == INSTALLED_ONLY ? "INSTALLED_ONLY" : "UNINSTALLED_ONLY" )
                                       : "ALL" ) << endl;

    o << "string match flags: " << Match(_flags) << endl;

    // raw
    o << "strings: ";
    for(StrContainer::const_iterator it = _strings.begin();
        it != _strings.end(); ++it)
      o << *it << " ";
    o << endl;

    o << "attributes: " << endl;
    for(AttrRawStrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai)
    {
      o << "* " << ai->first << ": ";
      for(StrContainer::const_iterator vi = ai->second.begin();
          vi != ai->second.end(); ++vi)
        o << *vi << " ";
      o << endl;
    }

    o << "predicated: " << endl;
    for_( it, _uncompiledPredicated.begin(), _uncompiledPredicated.end() )
    {
      o << "* " << *it << endl;
    }

    // compiled
    o << "last attribute matcher compiled: " << endl;
    if ( _attrMatchList.empty() )
    {
      o << "not yet compiled" << endl;
    }
    else
    {
      for_( it, _attrMatchList.begin(), _attrMatchList.end() )
      {
        o << "* " << *it << endl;
      }
    }
    return o.str();
  }

  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PoolQuery
  //
  ///////////////////////////////////////////////////////////////////

  PoolQuery::PoolQuery()
    : _pimpl(new Impl())
  {}

  PoolQuery::~PoolQuery()
  {}

  void PoolQuery::addRepo(const std::string &repoalias)
  {
    if (repoalias.empty())
    {
      WAR << "ignoring an empty repository alias" << endl;
      return;
    }
    _pimpl->_repos.insert(repoalias);
  }

  void PoolQuery::addKind(const ResKind & kind)
  { _pimpl->_kinds.insert(kind); }

  void PoolQuery::addString(const string & value)
  { _pimpl->_strings.insert(value); }

  void PoolQuery::addAttribute(const sat::SolvAttr & attr, const std::string & value)
  { _pimpl->_attrs[attr].insert(value); }

  void PoolQuery::addDependency( const sat::SolvAttr & attr, const std::string & name, const Rel & op, const Edition & edition )
  { return addDependency( attr, name, op, edition, Arch_empty ); }

  void PoolQuery::addDependency( const sat::SolvAttr & attr, const std::string & name, const Rel & op, const Edition & edition, const Arch & arch )
  {
    switch ( op.inSwitch() )
    {
      case Rel::ANY_e:	// no additional constraint on edition.
        if ( arch.empty() )	// no additional constraint on arch.
	{
	  addAttribute( attr, name );
	  return;
	}
	break;

      case Rel::NONE_e:	// will never match.
        return;

      default: // go and add the predicated query (uncompiled)
        break;
    }

    // Match::OTHER indicates need to compile
    // (merge global search strings into name).
    AttrMatchData attrMatchData( attr, StrMatcher( name, Match::OTHER ) );

    if ( isDependencyAttribute( attr ) )
      attrMatchData.addPredicate( EditionRangePredicate( op, edition, arch ) );
    else
      attrMatchData.addPredicate( SolvableRangePredicate( op, edition, arch ) );

    _pimpl->_uncompiledPredicated.insert( attrMatchData );
  }

  void PoolQuery::addDependency( const sat::SolvAttr & attr, Capability cap_r )
  {
    CapDetail cap( cap_r );
    if ( ! cap.isSimple() ) // will never match.
      return;

    // Matches STRING per default. (won't get compiled!)
    AttrMatchData attrMatchData( attr, StrMatcher( cap.name().asString() ) );

    if ( isDependencyAttribute( attr ) )
      attrMatchData.addPredicate( CapabilityMatchPredicate( cap_r ) );
    else
      attrMatchData.addPredicate( SolvableRangePredicate( cap.op(), cap.ed() ) );

    _pimpl->_uncompiledPredicated.insert( attrMatchData );
  }

  void PoolQuery::setEdition(const Edition & edition, const Rel & op)
  {
    _pimpl->_edition = edition;
    _pimpl->_op = op;
  }

  void PoolQuery::setMatchSubstring()	{ _pimpl->_flags.setModeSubstring(); }
  void PoolQuery::setMatchExact()	{ _pimpl->_flags.setModeString(); }
  void PoolQuery::setMatchRegex()	{ _pimpl->_flags.setModeRegex(); }
  void PoolQuery::setMatchGlob()	{ _pimpl->_flags.setModeGlob(); }
  void PoolQuery::setMatchWord()
  {
    _pimpl->_match_word = true;
    _pimpl->_flags.setModeRegex();
  }

  Match PoolQuery::flags() const
  { return _pimpl->_flags; }
  void PoolQuery::setFlags( const Match & flags )
  { _pimpl->_flags = flags; }


  void PoolQuery::setInstalledOnly()
  { _pimpl->_status_flags = INSTALLED_ONLY; }
  void PoolQuery::setUninstalledOnly()
  { _pimpl->_status_flags = UNINSTALLED_ONLY; }
  void PoolQuery::setStatusFilterFlags( PoolQuery::StatusFilter flags )
  { _pimpl->_status_flags = flags; }


  void PoolQuery::setRequireAll(bool require_all)
  { _pimpl->_require_all = require_all; }


  const PoolQuery::StrContainer &
  PoolQuery::strings() const
  { return _pimpl->_strings; }

  const PoolQuery::AttrRawStrMap &
  PoolQuery::attributes() const
  { return _pimpl->_attrs; }

  const PoolQuery::StrContainer &
  PoolQuery::attribute(const sat::SolvAttr & attr) const
  {
    static const PoolQuery::StrContainer nocontainer;
    AttrRawStrMap::const_iterator it = _pimpl->_attrs.find(attr);
    return it != _pimpl->_attrs.end() ? it->second : nocontainer;
  }

  const Edition PoolQuery::edition() const
  { return _pimpl->_edition; }
  const Rel PoolQuery::editionRel() const
  { return _pimpl->_op; }


  const PoolQuery::Kinds &
  PoolQuery::kinds() const
  { return _pimpl->_kinds; }

  const PoolQuery::StrContainer &
  PoolQuery::repos() const
  { return _pimpl->_repos; }


  bool PoolQuery::caseSensitive() const
  { return !_pimpl->_flags.test( Match::NOCASE ); }
  void PoolQuery::setCaseSensitive( bool value )
  { _pimpl->_flags.turn( Match::NOCASE, !value ); }

  bool PoolQuery::filesMatchFullPath() const
  { return _pimpl->_flags.test( Match::FILES ); }
  void PoolQuery::setFilesMatchFullPath( bool value )
  { _pimpl->_flags.turn( Match::FILES, value ); }

  bool PoolQuery::matchExact() const		{ return _pimpl->_flags.isModeString(); }
  bool PoolQuery::matchSubstring() const	{ return _pimpl->_flags.isModeSubstring(); }
  bool PoolQuery::matchGlob() const		{ return _pimpl->_flags.isModeGlob(); }
  bool PoolQuery::matchRegex() const		{ return _pimpl->_flags.isModeRegex(); }

  bool PoolQuery::matchWord() const
  { return _pimpl->_match_word; }

  bool PoolQuery::requireAll() const
  { return _pimpl->_require_all; }

  PoolQuery::StatusFilter PoolQuery::statusFilterFlags() const
  { return _pimpl->_status_flags; }

  bool PoolQuery::empty() const
  {
    try { return begin() == end(); }
    catch (const Exception & ex) {}
    return true;
  }

  PoolQuery::size_type PoolQuery::size() const
  {
    try
    {
      size_type count = 0;
      for_( it, begin(), end() )
        ++count;
      return count;
    }
    catch (const Exception & ex) {}
    return 0;
  }

  void PoolQuery::execute(ProcessResolvable fnc)
  { invokeOnEach( begin(), end(), fnc); }


  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : PoolQuery::Attr
  //
  /**
   * represents all atributes in PoolQuery except SolvAtributes, which are
   * used as is (not needed extend anything if someone adds new solv attr)
   */
  struct PoolQueryAttr : public IdStringType<PoolQueryAttr>
  {
  private:
    friend class IdStringType<PoolQueryAttr>;
    IdString _str;
  public:

    //noAttr
    PoolQueryAttr(){}

    explicit PoolQueryAttr( const char* cstr_r )
        : _str( cstr_r )
      {}

    explicit PoolQueryAttr( const std::string & str_r )
        : _str( str_r )
      {}

    // unknown atributes
    static const PoolQueryAttr noAttr;

    // PoolQuery's own attributes
    static const PoolQueryAttr repoAttr;
    static const PoolQueryAttr kindAttr;
    static const PoolQueryAttr stringAttr;
    static const PoolQueryAttr stringTypeAttr;
    static const PoolQueryAttr requireAllAttr;
    static const PoolQueryAttr caseSensitiveAttr;
    static const PoolQueryAttr installStatusAttr;
    static const PoolQueryAttr editionAttr;
    static const PoolQueryAttr complexAttr;
  };

  const PoolQueryAttr PoolQueryAttr::noAttr;

  const PoolQueryAttr PoolQueryAttr::repoAttr( "repo" );
  const PoolQueryAttr PoolQueryAttr::kindAttr( "type" );
  const PoolQueryAttr PoolQueryAttr::stringAttr( "query_string" );
  const PoolQueryAttr PoolQueryAttr::stringTypeAttr("match_type");
  const PoolQueryAttr PoolQueryAttr::requireAllAttr("require_all");
  const PoolQueryAttr PoolQueryAttr::caseSensitiveAttr("case_sensitive");
  const PoolQueryAttr PoolQueryAttr::installStatusAttr("install_status");
  const PoolQueryAttr PoolQueryAttr::editionAttr("version");
  const PoolQueryAttr PoolQueryAttr::complexAttr("complex");

  class StringTypeAttr : public IdStringType<PoolQueryAttr>
  {
    friend class IdStringType<StringTypeAttr>;
    IdString _str;

  public:
    StringTypeAttr(){}
    explicit StringTypeAttr( const char* cstr_r )
            : _str( cstr_r ){}
    explicit StringTypeAttr( const std::string & str_r )
             : _str( str_r ){}

    static const StringTypeAttr noAttr;

    static const StringTypeAttr exactAttr;
    static const StringTypeAttr substringAttr;
    static const StringTypeAttr regexAttr;
    static const StringTypeAttr globAttr;
    static const StringTypeAttr wordAttr;
  };

  const StringTypeAttr StringTypeAttr::noAttr;

  const StringTypeAttr StringTypeAttr::exactAttr("exact");
  const StringTypeAttr StringTypeAttr::substringAttr("substring");
  const StringTypeAttr StringTypeAttr::regexAttr("regex");
  const StringTypeAttr StringTypeAttr::globAttr("glob");
  const StringTypeAttr StringTypeAttr::wordAttr("word");

  ///////////////////////////////////////////////////////////////////


  //\TODO maybe ctor with stream can be usefull
  //\TODO let it throw, let it throw, let it throw.
  bool PoolQuery::recover( istream &str, char delim )
  {
    bool finded_something = false; //indicates some atributes is finded
    string s;
    do {
      if ( str.eof() )
        break;

      getline( str, s, delim );

      if ((!s.empty()) && s[0]=='#') //comment
      {
        continue;
      }

      string::size_type pos = s.find(':');
      if (s.empty() || pos == s.npos) // some garbage on line... act like blank line
      {
        if (finded_something) //is first blank line after record?
        {
          break;
        }
        else
        {
          continue;
        }
      }

      finded_something = true;

      string attrName(str::trim(string(s,0,pos))); // trimmed name of atribute
      string attrValue(str::trim(string(s,pos+1,s.npos))); //trimmed value

      PoolQueryAttr attribute( attrName );

      if ( attribute==PoolQueryAttr::repoAttr )
      {
        addRepo( attrValue );
      }
      /* some backwards compatibility */
      else if ( attribute==PoolQueryAttr::kindAttr || attribute=="kind" )
      {
        addKind( ResKind(attrValue) );
      }
      else if ( attribute==PoolQueryAttr::stringAttr
        || attribute=="global_string")
      {
        addString( attrValue );
      }
      else if ( attribute==PoolQueryAttr::stringTypeAttr
        || attribute=="string_type" )
      {
        StringTypeAttr s(attrValue);
        if( s == StringTypeAttr::regexAttr )
        {
          setMatchRegex();
        }
        else if ( s == StringTypeAttr::globAttr )
        {
          setMatchGlob();
        }
        else if ( s == StringTypeAttr::exactAttr )
        {
          setMatchExact();
        }
        else if ( s == StringTypeAttr::substringAttr )
        {
          setMatchSubstring();
        }
        else if ( s == StringTypeAttr::wordAttr )
        {
          setMatchWord();
        }
        else if ( s == StringTypeAttr::noAttr )
        {
          WAR << "unknown string type " << attrValue << endl;
        }
        else
        {
          WAR << "forget recover some attribute defined as String type attribute: " << attrValue << endl;
        }
      }
      else if ( attribute==PoolQueryAttr::requireAllAttr )
      {
        if ( str::strToTrue(attrValue) )
        {
          setRequireAll(true);
        }
        else if ( !str::strToFalse(attrValue) )
        {
          setRequireAll(false);
        }
        else
        {
          WAR << "unknown boolean value " << attrValue << endl;
        }
      }
      else if ( attribute==PoolQueryAttr::caseSensitiveAttr )
      {
        if ( str::strToTrue(attrValue) )
        {
          setCaseSensitive(true);
        }
        else if ( !str::strToFalse(attrValue) )
        {
          setCaseSensitive(false);
        }
        else
        {
          WAR << "unknown boolean value " << attrValue << endl;
        }
      }
      else if ( attribute==PoolQueryAttr::installStatusAttr )
      {
        if( attrValue == "all" )
        {
          setStatusFilterFlags( ALL );
        }
        else if( attrValue == "installed" )
        {
          setInstalledOnly();
        }
        else if( attrValue == "not-installed" )
        {
          setUninstalledOnly();
        }
        else
        {
          WAR << "Unknown value for install status " << attrValue << endl;
        }
      }
      else if ( attribute == PoolQueryAttr::editionAttr)
      {
        string::size_type pos;
        Rel rel("==");
        if (attrValue.find_first_of("=<>!") == 0)
        {
          pos = attrValue.find_last_of("=<>");
          rel = Rel(attrValue.substr(0, pos+1));
          attrValue = str::trim(attrValue.substr(pos+1, attrValue.npos));
        }

        setEdition(Edition(attrValue), rel);
      }
      else if ( attribute == PoolQueryAttr::complexAttr )
      {
        try
        {
          _pimpl->_uncompiledPredicated.insert( AttrMatchData::deserialize( attrValue ) );
        }
        catch ( const Exception & err )
        {
          WAR << "Unparsable value for complex: " << err.asUserHistory() << endl;

        }
      }
      else if ( attribute==PoolQueryAttr::noAttr )
      {
        WAR << "empty attribute name" << endl;
      }
      else
      {
        string s = attrName;
        str::replaceAll( s,"_",":" );
        SolvAttr a(s);
	if ( a == SolvAttr::name || isDependencyAttribute( a ) )
	{
	  Capability c( attrValue );
	  CapDetail d( c );
	  if ( d.isVersioned() )
	    addDependency( a, d.name().asString(), d.op(), d.ed() );
	  else
	    addDependency( a, attrValue );
	}
	else
	  addAttribute( a, attrValue );
      }

    } while ( true );

    return finded_something;
  }

  void PoolQuery::serialize( ostream &str, char delim ) const
  {
    //separating delim
    str << delim;
    //iterate thrue all settings and write it
    static const zypp::PoolQuery q; //not save default options, so create default query example

    for_( it, repos().begin(), repos().end() )
    {
      str << "repo: " << *it << delim ;
    }

    for_( it, kinds().begin(), kinds().end() )
    {
      str << PoolQueryAttr::kindAttr.asString() << ": "
          << it->idStr() << delim ;
    }

    if (editionRel() != Rel::ANY && edition() != Edition::noedition)
      str << PoolQueryAttr::editionAttr.asString() << ": " << editionRel() << " " << edition() << delim;

    if (matchMode()!=q.matchMode())
    {
      switch( matchMode() )
      {
      case Match::STRING:
        str << PoolQueryAttr::stringTypeAttr.asString() << ": exact" << delim;
        break;
      case Match::SUBSTRING:
        str << PoolQueryAttr::stringTypeAttr.asString()
            << ": substring" << delim;
        break;
      case Match::GLOB:
        str << PoolQueryAttr::stringTypeAttr.asString() << ": glob" << delim;
        break;
      case Match::REGEX:
        str << PoolQueryAttr::stringTypeAttr.asString() << ": regex" << delim;
        break;
      default:
        WAR << "unknown match type "  << matchMode() << endl;
      }
    }

    if( caseSensitive() != q.caseSensitive() )
    {
      str << "case_sensitive: ";
      if (caseSensitive())
      {
        str << "on" << delim;
      }
      else
      {
        str << "off" << delim;
      }
    }

    if( requireAll() != q.requireAll() )
    {
      str << "require_all: ";
      if (requireAll())
      {
        str << "on" << delim;
      }
      else
      {
        str << "off" << delim;
      }
    }

    if( statusFilterFlags() != q.statusFilterFlags() )
    {
      switch( statusFilterFlags() )
      {
      case ALL:
        str << "install_status: all" << delim;
        break;
      case INSTALLED_ONLY:
        str << "install_status: installed" << delim;
        break;
      case UNINSTALLED_ONLY:
        str << "install_status: not-installed" << delim;
        break;
      }
    }

    for_( it, strings().begin(), strings().end() )
    {
      str << PoolQueryAttr::stringAttr.asString()<< ": " << *it << delim;
    }

    for_( it, attributes().begin(), attributes().end() )
    {
      string s = it->first.asString();
      str::replaceAll(s,":","_");
      for_( it2,it->second.begin(),it->second.end() )
      {
        str << s <<": "<< *it2 << delim;
      }
    }

    for_( it, _pimpl->_uncompiledPredicated.begin(), _pimpl->_uncompiledPredicated.end() )
    {
      str << "complex: "<< it->serialize() << delim;
    }

    //separating delim - protection
    str << delim;
  }

  string PoolQuery::asString() const
  { return _pimpl->asString(); }

  ostream & operator<<( ostream & str, const PoolQuery & obj )
  { return str << obj.asString(); }

  std::ostream & dumpOn( std::ostream & str, const PoolQuery & obj )
  { return dumpRange( str << obj, obj.begin(), obj.end() ); }

  bool PoolQuery::operator==( const PoolQuery & rhs ) const
  { return *_pimpl == *rhs._pimpl; }

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : PoolQueryMatcher
    //
    /** Store \ref PoolQuery settings and assist \ref PoolQueryIterator.
     *
     * Basically the matcher performs a base query, which should preselect
     * candidates for a match. And has some filter conditions on top of it.
     * Query and fileter depend on the \ref PoolQuery settings.
     *
     * Matcher must be stateless, as it is shared between multiple
     * \ref PoolQueryIterator instances.
     *
     * If \ref base_iterator is at the \ref end, \ref advance moves it
     * to the first match. Otherwise advance moves to the next match, or
     * to the \ref end, if there is no more match.
     *
     * \note The original implementation treated an empty search string as
     * <it>"match always"</it>. We stay compatible.
     */
    class PoolQueryMatcher
    {
      public:
	typedef sat::LookupAttr::iterator base_iterator;

      public:
	const base_iterator & end() const
	{
	  static base_iterator _end;
	  return _end;
	}

	bool advance( base_iterator & base_r ) const
	{
	  if ( base_r == end() )
	    base_r = startNewQyery(); // first candidate
	  else
          {
            base_r.nextSkipSolvable(); // assert we don't visit this Solvable again
	    ++base_r; // advance to next candidate
          }

	  while ( base_r != end() )
	  {
	    if ( isAMatch( base_r ) )
	      return true;
	    // No match: try next
            ++base_r;
	  }
	  return false;
	}

        /** Provide all matching attributes within this solvable.
         *
         */
        void matchDetail( const base_iterator & base_r, std::vector<base_iterator> & return_r ) const
        {
          if ( base_r == end() )
            return;

          sat::Solvable inSolvable( base_r.inSolvable() );

          if ( _attrMatchList.size() == 1 )
          {
            // base_r is already on the 1st matching attribute!
            // String matching is done by the base iterator. We must check the predicate here.
            // Let's see if there are more matches for this solvable:
            base_iterator base( base_r );
            base.stayInThisSolvable(); // avoid discarding matches we found far away from here.
            return_r.push_back( base );

            const AttrMatchData::Predicate & predicate( _attrMatchList.front().predicate );
            for ( ++base; base.inSolvable() == inSolvable; ++base ) // safe even if base == end()
            {
              if ( ! predicate || predicate( base ) )
                return_r.push_back( base );
            }
          }
          else
          {
            // Here: search all attributes ;(
            for_( mi, _attrMatchList.begin(), _attrMatchList.end() )
            {
              const AttrMatchData & matchData( *mi );
              sat::LookupAttr q( matchData.attr, inSolvable );
              if ( matchData.strMatcher ) // an empty searchstring matches always
                q.setStrMatcher( matchData.strMatcher );

              if ( ! q.empty() ) // there are matches.
              {
                // now check any predicate:
                const AttrMatchData::Predicate & predicate( matchData.predicate );
                for_( it, q.begin(), q.end() )
                {
                  if ( ! predicate || predicate( it ) )
                    return_r.push_back( it );
                }
              }
            }
          }
        }

      public:
	/** Ctor stores the \ref PoolQuery settings.
         * \throw MatchException Any of the exceptions thrown by \ref PoolQuery::Impl::compile.
         */
	PoolQueryMatcher( const shared_ptr<const PoolQuery::Impl> & query_r )
	{
	  query_r->compile();

	  // Repo restriction:
	  sat::Pool satpool( sat::Pool::instance() );

	  for_( it, query_r->_repos.begin(), query_r->_repos.end() )
	  {
	    Repository r( satpool.reposFind( *it ) );
	    if ( r )
	      _repos.insert( r );
	    else
	      _neverMatchRepo = true;
	  }
	  // _neverMatchRepo: we just need to catch the case that no repo
	  // matched, so we'd interpret the empty list as 'take from all'
	  if ( _neverMatchRepo && ! _repos.empty() )
	    _neverMatchRepo = false;

	  // Kind restriction:
	  _kinds = query_r->_kinds;
	  // Edition restriction:
	  _op      = query_r->_op;
	  _edition = query_r->_edition;
	  // Status restriction:
	  _status_flags = query_r->_status_flags;
          // StrMatcher
          _attrMatchList = query_r->_attrMatchList;
	}

	~PoolQueryMatcher()
	{}

      private:
	/** Initialize a new base query. */
	base_iterator startNewQyery() const
	{
	  sat::LookupAttr q;

	  if ( _neverMatchRepo )
	    return q.end();

	  // Repo restriction:
	  if ( _repos.size() == 1 )
	    q.setRepo( *_repos.begin() );
	  // else: handled in isAMatch.

	  // Attribute restriction:
	  if ( _attrMatchList.size() == 1 ) // all (SolvAttr::allAttr) or 1 attr
	  {
            const AttrMatchData & matchData( _attrMatchList.front() );
	    q.setAttr( matchData.attr );
            if ( matchData.strMatcher ) // empty searchstring matches always
              q.setStrMatcher( matchData.strMatcher );
	  }
          else // more than 1 attr (but not all)
          {
            // no restriction, it's all handled in isAMatch.
            q.setAttr( sat::SolvAttr::allAttr );
          }

	  return q.begin();
	}


	/** Check whether we are on a match.
	 *
	 * The check covers the whole Solvable, not just the current
	 * attribute \c base_r points to. If there's no match, also
	 * prepare \c base_r to advance appropriately. If there is
	 * a match, simply return \c true. \ref advance always moves
	 * to the next Solvable if there was a match.
	 *
	 * \note: Caller asserts we're not at \ref end.
	*/
	bool isAMatch( base_iterator & base_r ) const
	{
	  /////////////////////////////////////////////////////////////////////
	  Repository inRepo( base_r.inRepo() );
	  // Status restriction:
	  if ( _status_flags
	     && ( (_status_flags == PoolQuery::INSTALLED_ONLY) != inRepo.isSystemRepo() ) )
	  {
	    base_r.nextSkipRepo();
	    return false;
	  }
	  // Repo restriction:
	  if ( _repos.size() > 1 && _repos.find( inRepo ) == _repos.end() )
	  {
	    base_r.nextSkipRepo();
	    return false;
	  }
	  /////////////////////////////////////////////////////////////////////
	  sat::Solvable inSolvable( base_r.inSolvable() );
	  // Kind restriction:
	  if ( ! _kinds.empty() && ! inSolvable.isKind( _kinds.begin(), _kinds.end() ) )
	  {
            base_r.nextSkipSolvable();
            return false;
	  }

	  // Edition restriction:
	  if ( _op != Rel::ANY && !compareByRel( _op, inSolvable.edition(), _edition, Edition::Match() ) )
	  {
	    base_r.nextSkipSolvable();
	    return false;
	  }
	  /////////////////////////////////////////////////////////////////////
	  // string and predicate matching:

          if ( _attrMatchList.size() == 1 )
          {
            // String matching was done by the base iterator.
            // Now check any predicate:
            const AttrMatchData::Predicate & predicate( _attrMatchList.front().predicate );
            if ( ! predicate || predicate( base_r ) )
              return true;

            return false; // no skip as there may be more occurrences od this attr.
          }

          // Here: search all attributes ;(
          for_( mi, _attrMatchList.begin(), _attrMatchList.end() )
          {
            const AttrMatchData & matchData( *mi );
            sat::LookupAttr q( matchData.attr, inSolvable );
            if ( matchData.strMatcher ) // an empty searchstring matches always
              q.setStrMatcher( matchData.strMatcher );

            if ( ! q.empty() ) // there are matches.
            {
              // now check any predicate:
              const AttrMatchData::Predicate & predicate( matchData.predicate );
              if ( predicate )
              {
                for_( it, q.begin(), q.end() )
                {
                  if ( predicate( it ) )
                    return true;
                }
              }
              else
                return true;
            }
          }
          base_r.nextSkipSolvable();
          return false;
	}

      private:
        /** Repositories include in the search. */
        std::set<Repository> _repos;
	DefaultIntegral<bool,false> _neverMatchRepo;
        /** Resolvable kinds to include. */
        std::set<ResKind> _kinds;
        /** Edition filter. */
        Rel _op;
        Edition _edition;
        /** Installed status filter flags. \see PoolQuery::StatusFilter */
        int _status_flags;
        /** StrMatcher per attribtue. */
        AttrMatchList _attrMatchList;
    };
    ///////////////////////////////////////////////////////////////////

    void PoolQueryIterator::increment()
    {
      // matcher restarts if at end! It is called from the ctor
      // to get the 1st match. But if the end is reached, it should
      // be deleted, otherwise we'd start over again.
      if ( !_matcher )
        return; // at end
      if ( _matches )
        _matches.reset(); // invalidate old matches
      if ( ! _matcher->advance( base_reference() ) )
        _matcher.reset();
    }

    const PoolQueryIterator::Matches & PoolQueryIterator::matches() const
    {
      if ( _matches )
        return *_matches;

      if ( !_matcher )
      {
        // at end of query:
        static const Matches _none;
        return _none;
      }

      _matches.reset( new Matches );
      _matcher->matchDetail( base_reference(), *_matches );
      return *_matches;
    }

    std::ostream & dumpOn( std::ostream & str, const PoolQueryIterator & obj )
    {
      str << *obj;
      if ( ! obj.matchesEmpty() )
      {
        for_( it, obj.matchesBegin(), obj.matchesEnd() )
        {
          str << endl << "    " << it->inSolvAttr() << "\t" << it->asString();
        }
      }
      return str;
    }

    ///////////////////////////////////////////////////////////////////
  } //namespace detail
  ///////////////////////////////////////////////////////////////////

  detail::PoolQueryIterator PoolQuery::begin() const
  {
    return shared_ptr<detail::PoolQueryMatcher>( new detail::PoolQueryMatcher( _pimpl.getPtr() ) );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

