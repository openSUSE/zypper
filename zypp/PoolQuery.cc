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
#include "zypp/base/Logger.h"
#include "zypp/base/Algorithm.h"
#include "zypp/base/String.h"
#include "zypp/repo/RepoException.h"
#include "zypp/RelCompare.h"

#include "zypp/sat/Pool.h"
#include "zypp/sat/Solvable.h"

#include "zypp/PoolQuery.h"

extern "C"
{
#include "satsolver/repo.h"
}

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "PoolQuery"

using namespace std;
using namespace zypp::sat;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

#warning CHECK FOR MEMLEAK DUE TO MISSING dataiterator_free

  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : PoolQuery::Impl
  //
  class PoolQuery::Impl
  {
  public:
    Impl()
      : _flags( SEARCH_NOCASE | SEARCH_SUBSTRING | SEARCH_SKIP_KIND )
      , _status_flags(ALL)
      , _match_word(false)
      , _require_all(false)
      , _compiled(false)
    {}

    ~Impl()
    {}

  public:
    const_iterator begin() const;
    const_iterator end() const;

    string asString() const;

    void compile() const;
  private:
    string createRegex(const StrContainer & container) const;

  public:
    /** Raw search strings. */
    StrContainer _strings;
    /** Compiled search strings. */
    mutable string _rcstrings;
    /** Compiled regex struct */
    mutable str::regex _regex;
    /** Raw attributes */
    AttrRawStrMap _attrs;
    /** Regex-compiled attributes */
    mutable AttrCompiledStrMap _rcattrs;
    mutable AttrRegexMap _rattrs;

    /** Edition condition operand */
    mutable Edition _edition;
    /** Operator for edition condition */
    mutable Rel _op;

    /** Repos to search. */
    StrContainer _repos;
    /** Kinds to search */
    Kinds _kinds;

    /** Sat solver search flags */
    int _flags;
    /** Backup of search flags. compile() may change the flags if needed, so
     * in order to reuse the query, the original flags need to be stored
     * at the start of compile() */
    mutable int _cflags;
    /** Sat solver status flags */
    StatusFilter _status_flags;

    bool _match_word;

    bool _require_all;

    mutable bool _compiled;

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };


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
    _cflags = _flags;

    // 'different'         - will have to iterate through all and match by ourselves (slow)
    // 'same'              - will pass the compiled string to dataiterator_init
    // 'one-attr'          - will pass it to dataiterator_init
    // 'one-non-regex-str' - will pass to dataiterator_init, set flag to SEARCH_STRING or SEARCH_SUBSTRING

    // // NO ATTRIBUTE
    // else
    //   for all _strings
    //     create regex; store in _rcstrings; if more strings flag regex;
    if (_attrs.empty())
    {
      _rcstrings = createRegex(_strings);
      if (_strings.size() > 1)
        _cflags = (_cflags & ~SEARCH_STRINGMASK) | SEARCH_REGEX;//setMatchRegex();
    }

    // // ONE ATTRIBUTE
    // else if _attrs is not empty but it contains just one attr
    //   for all _strings and _attr[key] strings
    //     create regex; store in _rcattrs; flag 'one-attr'; if more strings flag regex;
    else if (_attrs.size() == 1)
    {
      StrContainer joined;
      invokeOnEach(_strings.begin(), _strings.end(), EmptyFilter(), MyInserter(joined));
      invokeOnEach(_attrs.begin()->second.begin(), _attrs.begin()->second.end(), EmptyFilter(), MyInserter(joined));
      _rcstrings = createRegex(joined);
      _rcattrs.insert(pair<sat::SolvAttr, string>(_attrs.begin()->first, string()));
      // switch to regex for multiple strings
      if (joined.size() > 1)
        _cflags = (_cflags & ~SEARCH_STRINGMASK) | SEARCH_REGEX;
      if ((_cflags & SEARCH_STRINGMASK) == SEARCH_REGEX)
        /* We feed multiple lines eventually (e.g. authors or descriptions), so set REG_NEWLINE. */
        _regex = str::regex(_rcstrings, REG_EXTENDED | REG_NOSUB | REG_NEWLINE | (_cflags & SEARCH_NOCASE ? REG_ICASE : 0));
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
      //     create regex; store in _rcattrs and _rcstrings; flag 'same'; if more strings flag regex;
      if (attrvals_empty || attrvals_thesame)
      {
        StrContainer joined;
        if (attrvals_empty)
        {
          invokeOnEach(_strings.begin(), _strings.end(), EmptyFilter(), MyInserter(joined));
          _rcstrings = createRegex(joined);
        }
        else
        {
          invokeOnEach(_strings.begin(), _strings.end(), EmptyFilter(), MyInserter(joined));
          invokeOnEach(_attrs.begin()->second.begin(), _attrs.begin()->second.end(), EmptyFilter(), MyInserter(joined));
          _rcstrings = createRegex(joined);
        }
        // copy the _attrs keys to _rcattrs
        for_(ai, _attrs.begin(), _attrs.end())
          _rcattrs.insert(pair<sat::SolvAttr, string>(ai->first, string()));

        // switch to regex for multiple strings
        if (joined.size() > 1)
          _cflags = (_cflags & ~SEARCH_STRINGMASK) | SEARCH_REGEX;

        if ((_cflags & SEARCH_STRINGMASK) == SEARCH_REGEX)
          /* We feed multiple lines eventually (e.g. authors or descriptions), so set REG_NEWLINE. */
          _regex = str::regex(_rcstrings, REG_EXTENDED | REG_NOSUB | REG_NEWLINE | (_cflags & SEARCH_NOCASE ? REG_ICASE : 0));
      }

      // // DIFFERENT STRINGS FOR DIFFERENT ATTRS
      // if _attrs is not empty and it contains non-empty vectors with non-empty strings
      //   for each key in _attrs take all _strings + all _attrs[key] strings
      //     create regex; store in _rcattrs; flag 'different'; if more strings flag regex;
      else
      {
        for_(ai, _attrs.begin(), _attrs.end())
        {
          StrContainer joined;
          invokeOnEach(_strings.begin(), _strings.end(), EmptyFilter(), MyInserter(joined));
          invokeOnEach(ai->second.begin(), ai->second.end(), EmptyFilter(), MyInserter(joined));
          string s = createRegex(joined);
          _rcattrs.insert(pair<sat::SolvAttr, string>(ai->first, s));

          // switch to regex for multiple strings
          if (joined.size() > 1)
            _cflags = (_cflags & ~SEARCH_STRINGMASK) | SEARCH_REGEX;
          if ((_cflags & SEARCH_STRINGMASK) == SEARCH_REGEX)
          {
            str::regex regex(s, REG_EXTENDED | REG_NOSUB | REG_NEWLINE | (_cflags & SEARCH_NOCASE ? REG_ICASE : 0));
            _rattrs.insert(pair<sat::SolvAttr, str::regex>(ai->first, regex));
          }
        }
      }
    }

    _compiled = true;
    DBG << asString() << endl;
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

//! macro for word boundary tags for regexes
#define WB (_match_word ? string("\\b") : string())

  string PoolQuery::Impl::createRegex(const StrContainer & container) const
  {
    string rstr;

    if (container.empty())
      return rstr;

    if (container.size() == 1)
    {
      return WB + *container.begin() + WB;
    }

    // multiple strings

    bool use_wildcards = (_cflags & SEARCH_STRINGMASK) == SEARCH_GLOB;
    StrContainer::const_iterator it = container.begin();
    string tmp;

    if (use_wildcards)
      tmp = wildcards2regex(*it);
    else
      tmp = *it;

    if (_require_all)
    {
      if ((_cflags & SEARCH_STRINGMASK) != SEARCH_STRING) // not match exact
        tmp += ".*" + WB + tmp;
      rstr = "(?=" + tmp + ")";
    }
    else
    {
      if ((_cflags & SEARCH_STRINGMASK) == SEARCH_STRING || // match exact
          (_cflags & SEARCH_STRINGMASK) == SEARCH_GLOB)     // match glob
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
        if ((_cflags & SEARCH_STRINGMASK) != SEARCH_STRING) // not match exact
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
      if ((_cflags & SEARCH_STRINGMASK) != SEARCH_STRING) // not match exact
        rstr += WB + ".*";
    }
    else
    {
      rstr += ")" + WB;
      if ((_cflags & SEARCH_STRINGMASK) == SEARCH_STRING || // match exact
          (_cflags & SEARCH_STRINGMASK) == SEARCH_GLOB)     // match glob
        rstr += "$";
    }

    return rstr;
  }


  PoolQuery::const_iterator PoolQuery::Impl::begin() const
  {
    compile();

    sat::Pool pool(sat::Pool::instance());
    // no pool or no repos
    if (!pool.get() || pool.reposEmpty())
      return end();

    // if only one repository has been specified, find it in the pool
    Repository repo;
    if ( _repos.size() == 1 )
    {
      string theone = *_repos.begin();
      repo = pool.reposFind(theone);
      if (repo == Repository::noRepository)
      {
        DBG << "Repository '" << theone << "' not found in sat pool." << endl;
        return end();
      }
    }

    DBG << "_cflags:" << _cflags << endl;

    scoped_ptr< ::_Dataiterator> _rdit( new ::Dataiterator );
    // needed while LookupAttr::iterator::dip_equal does ::memcmp:
    ::memset( _rdit.get(), 0, sizeof(::_Dataiterator) );

    // initialize the Dataiterator for different cases
    if (_rcattrs.empty())
    {
      ::dataiterator_init(_rdit.get(),
        pool.get(),
        repo.get(),                                  // either NULL or the repo to search
        0,                                           // search all solvables
        0,                                           // attribute id - only if 1 attr key specified
        _rcstrings.empty() ? 0 : _rcstrings.c_str(), // compiled search string
        _cflags);
    }
    else if (_rcattrs.size() == 1)
    {
      ::dataiterator_init(_rdit.get(),
        pool.get(),
        repo.get(),                                  // either NULL or the repo to search
        0,                                           // search all solvables
        _rcattrs.begin()->first.id(),                // keyname - attribute id - only if 1 attr key specified
        _rcstrings.empty() ? 0 : _rcstrings.c_str(), // compiled search string
        _cflags);
    }
    else
    {
      ::dataiterator_init(_rdit.get(),
        pool.get(),
        repo.get(),                                  // either NULL or the repo to search
        0, /*search all resolvables */
        0, /*keyname - if only 1 attr key specified, pass it here, otherwise do more magic */
        0, //qs.empty() ? 0 : qs.c_str(), /* create regex, pass it here */
        _cflags);
    }

    if ((_cflags & SEARCH_STRINGMASK) == SEARCH_REGEX && _rdit->matcher.error != 0)
      ZYPP_THROW(Exception(str::form(
          _("Invalid regular expression '%s': regcomp returned %d"),
          _rcstrings.c_str(), _rdit->matcher.error)));

    PoolQuery::const_iterator it(_rdit, this);
    it.increment();
    return it;
  }


  PoolQuery::const_iterator PoolQuery::Impl::end() const
  {
    return PoolQuery::const_iterator();
  }


  string PoolQuery::Impl::asString() const
  {
    ostringstream o;

    o << "compiled: " << _compiled << endl;

    o << "kinds: ";
    for(Kinds::const_iterator it = _kinds.begin();
        it != _kinds.end(); ++it)
      o << *it << " ";
    o << endl;

    o << "repos: ";
    for(StrContainer::const_iterator it = _repos.begin();
        it != _repos.end(); ++it)
      o << *it << " ";
    o << endl;

    o << "string match flags:" << endl;
    o << "* string/substring/glob/regex: " << (_cflags & SEARCH_STRINGMASK) << endl;
    o << "* SEARCH_NOCASE: " << ((_cflags & SEARCH_NOCASE) ? "yes" : "no") << endl;
    o << "* SEARCH_ALL_REPOS: " << (_repos.empty() ? "yes" : "no") << endl;
    o << "status filter flags:" << _status_flags << endl;
    o << "version: "<< _op << " " << _edition.asString() << endl;

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

    // compiled

    o << "compiled strings: " << _rcstrings << endl;
    o << "compiled attributes:" << endl;
    for (AttrCompiledStrMap::const_iterator ai = _rcattrs.begin(); ai != _rcattrs.end(); ++ai)
      o << "* " << ai->first << ": " << ai->second << endl;

    return o.str();
  }

  /** \relates PoolQuery::Impl Stream output *//*
  inline std::ostream & operator<<( std::ostream & str, const PoolQuery::Impl & obj )
  {
    return str << "PoolQuery::Impl";
  }
  */
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : PoolQuery::ResultIterator
  //
  ///////////////////////////////////////////////////////////////////

  PoolQueryIterator::PoolQueryIterator()
    : _sid(0), _has_next(false), _do_matching(false)
  { this->base_reference() = LookupAttr::iterator(); }


  PoolQueryIterator::PoolQueryIterator(
      scoped_ptr< ::_Dataiterator> & dip_r,
      const PoolQuery::Impl * pqimpl)
  : _sid(0)
  , _has_next(true)
  , _do_matching(pqimpl->_rcattrs.size() > 1)
  , _flags(pqimpl->_cflags)
  , _str(pqimpl->_rcstrings)
  , _regex(pqimpl->_regex)
  , _attrs_str(pqimpl->_rcattrs)
  , _attrs_regex(pqimpl->_rattrs)
  , _repos(pqimpl->_repos)
  , _kinds(pqimpl->_kinds)
  , _status_flags(pqimpl->_status_flags)
  , _edition(pqimpl->_edition)
  , _op(pqimpl->_op)
  {
    this->base_reference() = LookupAttr::iterator(dip_r, true); //!\todo pass chain_repos
    _has_next = (*base_reference() != sat::detail::noId);
  }


  PoolQueryIterator::PoolQueryIterator(const PoolQueryIterator & rhs)
    : _sid(rhs._sid)
    , _has_next(rhs._has_next)
    , _do_matching(rhs._do_matching)
    , _flags(rhs._flags)
    , _str(rhs._str)
    , _regex(rhs._regex)
    , _attrs_str(rhs._attrs_str)
    , _attrs_regex(rhs._attrs_regex)
    , _repos(rhs._repos)
    , _kinds(rhs._kinds)
    , _status_flags(rhs._status_flags)
    , _edition(rhs._edition)
    , _op(rhs._op)
  { base_reference() = LookupAttr::iterator(rhs.base()); }


  PoolQueryIterator::~PoolQueryIterator()
  {}


  PoolQueryIterator & PoolQueryIterator::operator=( const PoolQueryIterator & rhs )
  {
    base_reference() = rhs.base();
    _sid = rhs._sid;
    _has_next = rhs._has_next;
    _do_matching = rhs._do_matching;
    _flags = rhs._flags;
    _str = rhs._str;
    _regex = rhs._regex;
    _attrs_str = rhs._attrs_str;
    _attrs_regex = rhs._attrs_regex;
    _repos = rhs._repos;
    _kinds = rhs._kinds;
    _status_flags = rhs._status_flags;
    _edition = rhs._edition;
    _op = rhs._op;
    return *this;
  }


  void PoolQueryIterator::increment()
  {
    if (!base().get())
      return;

    bool got_match = false;
    if (_has_next)
      while (_has_next && !(got_match = matchSolvable()));

    // no more solvables and the last did not match
    if (!got_match && !_has_next)
    {
      base_reference() = LookupAttr::iterator();
      _sid = 0;
    }
  }

  bool PoolQueryIterator::matchSolvable()
  {
    _sid = base().get()->solvid;

    bool new_solvable = true;
    bool matches = !_do_matching;
    bool drop_by_kind_status_edition = false;
    bool drop_by_repo = false;
    do
    {
      if (new_solvable)
      {
        while(1)
        {
          // whether to drop a resolvable not belonging to this repo
          drop_by_repo = false;
          if(!_repos.empty() &&
             _repos.find(base().get()->repo->name) == _repos.end())
          {
            drop_by_repo = true;
            break;
          }

          drop_by_kind_status_edition = false;

          // whether to drop a resolvable not matching the edition condition
          if (_op != Rel::ANY)
          {
            sat::Solvable s(_sid);
            if (!compareByRel<Edition>( _op, s.edition(), _edition, Edition::Match()))
            {
              drop_by_kind_status_edition = true;
              break;
            }
          }

          // whether to drop an uninstalled (repo) solvable
          if ( (_status_flags & PoolQuery::INSTALLED_ONLY) &&
              base().get()->repo->name != sat::Pool::instance().systemRepoAlias() )
          {
            drop_by_kind_status_edition = true;
            break;
          }

          // whether to drop an installed (target) solvable
          if ((_status_flags & PoolQuery::UNINSTALLED_ONLY) &&
              base().get()->repo->name == sat::Pool::instance().systemRepoAlias())
          {
            drop_by_kind_status_edition = true;
            break;
          }

          // whether to drop unwanted kind
          if (!_kinds.empty())
          {
            sat::Solvable s(_sid);
            if (_kinds.find(s.kind()) == _kinds.end())
              drop_by_kind_status_edition = true;
          }

          break;
        }

        matches = matches && !drop_by_kind_status_edition && !drop_by_repo;
      }

      if (_do_matching && !drop_by_kind_status_edition)
      {
        if (!matches)
        {
          SolvAttr attr(base().get()->key->name);
          PoolQuery::AttrCompiledStrMap::const_iterator ai = _attrs_str.find(attr);
          if (ai != _attrs_str.end())
          {
            if ((_flags & SEARCH_STRINGMASK) == SEARCH_REGEX)
            {
              const regex_t * regex_p;
              if (_str.empty())
              {
                PoolQuery::AttrRegexMap::iterator rai = _attrs_regex.find(attr);
                if (rai != _attrs_regex.end())
                  regex_p = rai->second.get();
                else
                {
                  ERR << "no compiled regex found for " <<  attr << endl;
                  continue;
                }
              }
              else
                regex_p = _regex.get();
#warning wrap matcher an use it
              matches = ::dataiterator_match_obsolete(base().get(), _flags, regex_p);
            }
            else
            {
              const string & sstr =
                _str.empty() ? ai->second : _str;
              matches = ::dataiterator_match_obsolete(base().get(), _flags, sstr.c_str());
            }

              // if (matches)
	      /* After calling dataiterator_match (with any string matcher set)
	         the kv.str member will be filled with something sensible.  */
               /*INT << "value: " << base().get()->kv.str << endl
                  << " str: " <<  _str << endl;*/
          }
        }
      }

      if (drop_by_repo)
      {
        base_reference().nextSkipRepo();
        drop_by_repo = false;
      }
      else if (drop_by_kind_status_edition)
      {
        base_reference().nextSkipSolvable();
        drop_by_kind_status_edition = false;
      }

      // copy the iterator to forward check for the next attribute ***
      _tmpit = base_reference();
      _has_next = ++_tmpit != LookupAttr::iterator();

      if (_has_next)
      {
        // *** now increment. Had it not be done this way,
        // the LookupAttr::iterator could have reached the end() while
        // trying to reach a matching attribute or the next solvable
        // thus resulting to a problem in the equal() method
        ++base_reference();
        new_solvable = base().get()->solvid != _sid;
      }
      // no more attributes in this repo, return
      else
        return matches; // did the last solvable match conditions?
    }
    while (!new_solvable);

    return matches;
  }

  ///////////////////////////////////////////////////////////////////
  } //namespace detail
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


  void PoolQuery::setEdition(const Edition & edition, const Rel & op)
  {
    _pimpl->_edition = edition;
    _pimpl->_op = op;
  }


  void PoolQuery::setCaseSensitive(const bool value)
  {
    if (value)
      _pimpl->_flags &= ~SEARCH_NOCASE;
    else
      _pimpl->_flags |= SEARCH_NOCASE;
  }

  void PoolQuery::setMatchFiles()
  {
    _pimpl->_flags = (_pimpl->_flags & ~SEARCH_STRINGMASK) | SEARCH_FILES;
  }

  void PoolQuery::setMatchSubstring()
  { _pimpl->_flags = (_pimpl->_flags & ~SEARCH_STRINGMASK) | SEARCH_SUBSTRING; }
  void PoolQuery::setMatchExact()
  { _pimpl->_flags = (_pimpl->_flags & ~SEARCH_STRINGMASK) | SEARCH_STRING; }
  void PoolQuery::setMatchRegex()
  { _pimpl->_flags = (_pimpl->_flags & ~SEARCH_STRINGMASK) | SEARCH_REGEX; }
  void PoolQuery::setMatchGlob()
  { _pimpl->_flags = (_pimpl->_flags & ~SEARCH_STRINGMASK) | SEARCH_GLOB; }
  void PoolQuery::setMatchWord()
  {
    _pimpl->_match_word = true;
    _pimpl->_flags = (_pimpl->_flags & ~SEARCH_STRINGMASK) | SEARCH_REGEX;
  }

  void PoolQuery::setFlags(int flags)
  { _pimpl->_flags = flags; }

  int PoolQuery::flags() const
  { return _pimpl->_flags; }

  void PoolQuery::setInstalledOnly()
  { _pimpl->_status_flags = INSTALLED_ONLY; }
  void PoolQuery::setUninstalledOnly()
  { _pimpl->_status_flags = UNINSTALLED_ONLY; }
  void PoolQuery::setStatusFilterFlags( PoolQuery::StatusFilter flags )
  { _pimpl->_status_flags = flags; }


  void PoolQuery::setRequireAll(const bool require_all)
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
  { return !(_pimpl->_flags & SEARCH_NOCASE); }

  bool PoolQuery::matchExact() const
  { return (_pimpl->_flags & SEARCH_STRINGMASK) == SEARCH_STRING; }
  bool PoolQuery::matchSubstring() const
  { return (_pimpl->_flags & SEARCH_STRINGMASK) == SEARCH_SUBSTRING; }
  bool PoolQuery::matchGlob() const
  { return (_pimpl->_flags & SEARCH_STRINGMASK) == SEARCH_GLOB; }
  bool PoolQuery::matchRegex() const
  { return (_pimpl->_flags & SEARCH_STRINGMASK) == SEARCH_REGEX; }
  int PoolQuery::matchType() const
  { return _pimpl->_flags & SEARCH_STRINGMASK; }
  bool PoolQuery::matchFiles() const
  { return (_pimpl->_flags & SEARCH_STRINGMASK) == SEARCH_FILES; }

  bool PoolQuery::matchWord() const
  { return _pimpl->_match_word; }

  bool PoolQuery::requireAll() const
  { return _pimpl->_require_all; }

  PoolQuery::StatusFilter PoolQuery::statusFilterFlags() const
  { return _pimpl->_status_flags; }

  PoolQuery::const_iterator PoolQuery::begin() const
  { return _pimpl->begin(); }


  PoolQuery::const_iterator PoolQuery::end() const
  { return _pimpl->end(); }


  bool PoolQuery::empty() const
  {
    try { return _pimpl->begin() == _pimpl->end(); }
    catch (const Exception & ex) {}
    return true;
  }


  PoolQuery::size_type PoolQuery::size() const
  {
    size_type count = 0;
    try
    {
      for(const_iterator it = _pimpl->begin(); it != _pimpl->end(); ++it, ++count);
    }
    catch (const Exception & ex) {}

    return count;
  }


  void PoolQuery::execute(ProcessResolvable fnc)
  { invokeOnEach(_pimpl->begin(), _pimpl->end(), fnc); }


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

      MIL << "attribute name: " << attrName << endl;

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
      else if ( attribute==PoolQueryAttr::noAttr )
      {
        WAR << "empty attribute name" << endl;
      }
      else
      {
        string s = attrName;
        str::replaceAll( s,"_",":" );
        SolvAttr a(s);
        addAttribute(a,attrValue);
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

    if (matchType()!=q.matchType())
    {
      switch( matchType() )
      {
      case SEARCH_STRING:
        str << PoolQueryAttr::stringTypeAttr.asString() << ": exact" << delim;
        break;
      case SEARCH_SUBSTRING:
        str << PoolQueryAttr::stringTypeAttr.asString()
            << ": substring" << delim;
        break;
      case SEARCH_GLOB:
        str << PoolQueryAttr::stringTypeAttr.asString() << ": glob" << delim;
        break;
      case SEARCH_REGEX:
        str << PoolQueryAttr::stringTypeAttr.asString() << ": regex" << delim;
        break;
      default:
        WAR << "unknown match type "  << matchType() << endl;
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

    //separating delim - protection
    str << delim;

  }


  string PoolQuery::asString() const
  { return _pimpl->asString(); }


  ostream & operator<<( ostream & str, const PoolQuery & obj )
  { return str << obj.asString(); }

  //internal matching two containers O(n^2)
  template <class Container>
  bool equalContainers(const Container& a, const Container& b)
  {
    if (a.size()!=b.size())
      return false;

    for_(it,a.begin(),a.end())
    {
      bool finded = false;
      for_( it2, b.begin(),b.end() )
      {
        if (*it==*it2)
        {
          finded = true;
          break;
        }
      }

      if (!finded)
        return false;
    }
    return true;
  }

  bool PoolQuery::operator==(const PoolQuery& a) const
  {
    if (!_pimpl->_compiled)
      _pimpl->compile();
    if (!a._pimpl->_compiled)
      a._pimpl->compile();
    if( matchType() != a.matchType() )
      return false;
    if( a.matchWord() != matchWord())
      return false;
    if( a.requireAll() != requireAll() )
      return false;
    if(!equalContainers(a.kinds(), kinds()))
      return false;
    if(!equalContainers(a.repos(), repos()))
      return false;
    if(a._pimpl->_rcstrings != _pimpl->_rcstrings)
      return false;
    if(!equalContainers(a._pimpl->_rcattrs, _pimpl->_rcattrs))
      return false;
    if(a._pimpl->_cflags != _pimpl->_cflags)
      return false;
    if(a.edition() != edition())
      return false;
    if(a.editionRel() != editionRel())
      return false;

    return true;
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

