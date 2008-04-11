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
#include <boost/algorithm/string/replace.hpp>

#include "zypp/base/Gettext.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Regex.h"
#include "zypp/base/Algorithm.h"
#include "zypp/base/String.h"
#include "zypp/repo/RepoException.h"

#include "zypp/sat/Pool.h"
#include "zypp/sat/Solvable.h"

#include "zypp/PoolQuery.h"

extern "C"
{
#include "satsolver/repo.h"
}

using namespace std;
using namespace zypp::sat;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : PoolQuery::Impl
  //
  class PoolQuery::Impl
  {
  public:
    Impl()
      : _flags( SEARCH_ALL_REPOS | SEARCH_NOCASE | SEARCH_SUBSTRING )
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
    /** Regex-compiled search strings. */
    mutable string _rcstrings;
    mutable regex_t _regex;
    /** Raw attributes */
    AttrMap _attrs;
    /** Regex-compiled attributes */
    mutable CompiledAttrMap _rcattrs;
    mutable map<sat::SolvAttr, regex_t> _rattrs;

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
    PoolQuery::StatusFilter _status_flags;

    bool _match_word;

    bool _require_all;

    /** Sat solver Dataiterator structure */
    mutable ::_Dataiterator _rdit;

    mutable bool _compiled;

    /** Function for processing found solvables. Used in execute(). */
    mutable PoolQuery::ProcessResolvable _fnc;
    
  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };

  static void
  compileRegex(regex_t * regex, const string & str, bool nocase)
  {
    /* We feed multiple lines eventually (e.g. authors or descriptions),
       so set REG_NEWLINE. */
    if (regcomp(regex, str.c_str(),
        REG_EXTENDED | REG_NOSUB | REG_NEWLINE | (nocase ? REG_ICASE : 0)) != 0)
      ZYPP_THROW(Exception(
        str::form(_("Invalid regular expression '%s'"), str.c_str())));
  }

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
    }

    // // MULTIPLE ATTRIBUTES
    else
    {
      // check whether there are any per-attribute strings 
      bool attrvals_empty = true;
      for (AttrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai)
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
      AttrMap::const_iterator ai = _attrs.begin();
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

        if ((_cflags & SEARCH_STRINGMASK) == SEARCH_REGEX)
          compileRegex(&_regex, _rcstrings, _cflags & SEARCH_NOCASE);
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

          if ((_cflags & SEARCH_STRINGMASK) == SEARCH_REGEX)
          {
            regex_t regex;
            compileRegex(&regex, s, _cflags & SEARCH_NOCASE);
            _rattrs.insert(pair<sat::SolvAttr, regex_t>(ai->first, regex));
          }
        }
      }
    }

    // tell the Dataiterator to search only in one repo if only one specified
    if (_repos.size() == 1)
      _cflags &= ~SEARCH_ALL_REPOS;

    _compiled = true;
    
    DBG << asString() << endl;
  }

  /**
   * Converts '*' and '?' wildcards within str into their regex equivalents.
   */
  static string wildcards2regex(const string & str)
  {
    string regexed = str;

    str::regex all("\\*"); // regex to search for '*'
    str::regex one("\\?"); // regex to search for '?'
    string r_all(".*"); // regex equivalent of '*'
    string r_one(".");  // regex equivalent of '?'
    string::size_type pos;

    // replace all "*" in input with ".*"
    for (pos = 0; (pos = regexed.find("*", pos)) != std::string::npos; pos+=2)
      regexed = regexed.replace(pos, 1, r_all);

    // replace all "?" in input with "."
    for (pos = 0; (pos = regexed.find('?', pos)) != std::string::npos; ++pos)
      regexed = regexed.replace(pos, 1, r_one);

    DBG << " -> " << regexed << endl;

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
      if (_match_word)
        return ".*" + WB + *container.begin() + WB + ".*";

      return *container.begin();
    }

    // multiple strings

    bool use_wildcards = (_cflags & SEARCH_STRINGMASK) == SEARCH_GLOB;
    StrContainer::const_iterator it = container.begin();
    string tmp;

    if (use_wildcards)
      tmp = wildcards2regex(*it);

    if (_require_all)
    {
      if (!(_cflags & SEARCH_STRING)) // not match exact
        tmp += ".*" + WB + tmp;
      rstr = "(?=" + tmp + ")";
    }
    else
    {
      if (_cflags & SEARCH_STRING) // match exact
        rstr = "^";
      else
        rstr = ".*" + WB;

      rstr += "(" + tmp;
    }

    ++it;

    for (; it != container.end(); ++it)
    {
      if (use_wildcards)
        tmp = wildcards2regex(*it);

      if (_require_all)
      {
        if (!(_cflags & SEARCH_STRING)) // not match exact
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
      if (!(_cflags & SEARCH_STRING)) // not match exact
        rstr += WB + ".*";
    }
    else
    {
      rstr += ")";
      if (_cflags & SEARCH_STRING) // match exact
        rstr += "$";
      else
        rstr += WB + ".*";
    }

    return rstr;
  }


  PoolQuery::const_iterator PoolQuery::Impl::begin() const
  {
    compile();

    // if only one repository has been specified, find it in the pool
    sat::Pool pool(sat::Pool::instance());
    sat::Pool::RepositoryIterator itr = pool.reposBegin();
    if (!(_cflags & SEARCH_ALL_REPOS) && _repos.size() == 1)
    {
      string theone = *_repos.begin();
      for (; itr->info().alias() != theone && itr != pool.reposEnd(); ++itr);
      if (itr == pool.reposEnd())
      {
        RepoInfo info; info.setAlias(theone);
        ERR << "Repository not found in sat pool." <<  endl;
        ZYPP_THROW(repo::RepoNotFoundException(info));
      }
    }

    DBG << "_cflags:" << _cflags << endl;

    if (_rcattrs.empty())
    {
    ::dataiterator_init(&_rdit,
      _cflags & SEARCH_ALL_REPOS ? pool.get()->repos[0] : itr->get(), // repository \todo fix this
      0,                                           // search all solvables
      0,                                           // attribute id - only if 1 attr key specified
      _rcstrings.empty() ? 0 : _rcstrings.c_str(), // compiled search string
      _cflags);
    }
    else if (_rcattrs.size() == 1)
    {
      ::dataiterator_init(&_rdit,
        _cflags & SEARCH_ALL_REPOS ? pool.get()->repos[0] : itr->get(), // repository \todo fix this 
        0,                                           // search all solvables
        _rcattrs.begin()->first.id(),                // keyname - attribute id - only if 1 attr key specified
        _rcstrings.empty() ? 0 : _rcstrings.c_str(), // compiled search string 
        _cflags);
    }
    else
    {
      ::dataiterator_init(&_rdit,
        _cflags & SEARCH_ALL_REPOS ? pool.get()->repos[0] : itr->get(), /* repository - switch to next at the end of current one in increment() */ 
        0, /*search all resolvables */
        0, /*keyname - if only 1 attr key specified, pass it here, otherwise do more magic */
        0, //qs.empty() ? 0 : qs.c_str(), /* create regex, pass it here */
        _cflags);
    }

    if ((_cflags & SEARCH_STRINGMASK) == SEARCH_REGEX && _rdit.regex_err != 0)
      ZYPP_THROW(Exception(
        str::form(_("Invalid regular expression '%s'"), _rcstrings.c_str())));

    PoolQuery::const_iterator it(this);
    it.increment();
    return it;
  }

  PoolQuery::const_iterator PoolQuery::Impl::end() const
  {
    INT << "end" << endl;
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
    o << "* SEARCH_ALL_REPOS: " << ((_cflags & SEARCH_ALL_REPOS) ? "yes" : "no") << endl;
    o << "status filter flags:" << _status_flags << endl;

    // raw

    o << "strings: ";
    for(StrContainer::const_iterator it = _strings.begin();
        it != _strings.end(); ++it)
      o << *it << " ";
    o << endl;

    o << "attributes: " << endl;
    for(AttrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai)
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
    for (CompiledAttrMap::const_iterator ai = _rcattrs.begin(); ai != _rcattrs.end(); ++ai)
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

  PoolQueryIterator::PoolQueryIterator(const PoolQuery::Impl * pqimpl)
  : PoolQueryIterator::iterator_adaptor_(pqimpl ? &pqimpl->_rdit : 0)
  , _rdit(pqimpl ? &pqimpl->_rdit : 0)
  , _pqimpl(pqimpl)
  , _sid(0)
  , _has_next(true)
  , _do_matching(false)
  , _pool((sat::Pool::instance()))
  {
    if (_pqimpl->_rcattrs.size() > 1)
      _do_matching = true;
  }

  void PoolQueryIterator::increment()
  {
    if (!_rdit)
      return;

    bool got_match = false;
    if (_has_next)
    {
      DBG << "last: " << _sid << endl;
      while (_has_next && !(got_match = matchSolvable()));
    }

    // no more solvables and the last did not match
    if (!got_match && !_has_next)
    {
      base_reference() = 0;
      _sid = 0;
    }

    DBG << "next: " << _sid << endl;
  }

  bool PoolQueryIterator::matchSolvable()
  {
    _sid = _rdit->solvid;

    bool new_solvable = true;
    bool matches = !_do_matching;
    bool in_repo;
    bool drop_by_kind_status = false;
    bool drop_by_repo = false;
    do
    {
      //! \todo FIXME Dataiterator returning resolvables belonging to current repo?
      in_repo = _sid >= _rdit->repo->start;

      if (in_repo && new_solvable)
      {
        while(1)
        {
          drop_by_repo = false;
          if (!_pqimpl->_repos.empty() && 
            _pqimpl->_repos.find(_rdit->repo->name) == _pqimpl->_repos.end())
          {
            drop_by_repo = true;
            break;
          }

          drop_by_kind_status = false;

          // whether to drop an uninstalled (repo) solvable
          if ( (_pqimpl->_status_flags & PoolQuery::INSTALLED_ONLY) &&
               _rdit->repo->name != _pool.systemRepoName() )
          {
            drop_by_kind_status = true;
            break;
          }

          // whether to drop an installed (target) solvable
          if ((_pqimpl->_status_flags & PoolQuery::UNINSTALLED_ONLY) &&
               _rdit->repo->name == _pool.systemRepoName())
          {
            drop_by_kind_status = true;
            break;
          }

          // whether to drop unwanted kind
          if (!_pqimpl->_kinds.empty())
          {
            sat::Solvable s(_sid);
            // the user wants to filter by kind.
            if (_pqimpl->_kinds.find(s.kind()) == _pqimpl->_kinds.end())
              drop_by_kind_status = true;
          }

          break;
        }

        matches = matches && !drop_by_kind_status && !drop_by_repo;
      }

      if (_do_matching && !drop_by_kind_status)
      {
        if (!matches && in_repo)
        {
          SolvAttr attr(_rdit->key->name);
          PoolQuery::CompiledAttrMap::const_iterator ai = _pqimpl->_rcattrs.find(attr);
          if (ai != _pqimpl->_rcattrs.end())
          {
            if ((_pqimpl->_cflags & SEARCH_STRINGMASK) == SEARCH_REGEX)
            {
              const regex_t * regex;
              if (_pqimpl->_rcstrings.empty())
              {
                map<sat::SolvAttr, regex_t>::const_iterator rai = _pqimpl->_rattrs.find(attr);
                if (rai != _pqimpl->_rattrs.end())
                  regex = &rai->second;
                else
                {
                  ERR << "no compiled regex found for " <<  attr << endl;
                  continue;
                }
              }
              else
                regex = &_pqimpl->_regex;
              matches = ::dataiterator_match(_rdit, _pqimpl->_cflags, regex);
            }
            else
            {
              const string & sstr =
                _pqimpl->_rcstrings.empty() ? ai->second : _pqimpl->_rcstrings;
              matches = ::dataiterator_match(_rdit, _pqimpl->_cflags, sstr.c_str());
            }

//            if (matches)
	      /* After calling dataiterator_match (with any string matcher set)
	         the kv.str member will be filled with something sensible.  */
  /*            INT << "value: " << _rdit->kv.str << endl
                  << " mstr: " <<  sstr << endl;*/ 
          }
        }
      }

      if (drop_by_repo)
      {
        Repository nextRepo(Repository(_rdit->repo).nextInPool());
        ::dataiterator_skip_repo(_rdit);
        if (nextRepo)
          ::dataiterator_jump_to_repo(_rdit, nextRepo.get());
        drop_by_repo = false;
      }
      else if (drop_by_kind_status)
      {
        ::dataiterator_skip_solvable(_rdit);
        drop_by_kind_status = false;
      }

      if ((_has_next = ::dataiterator_step(_rdit)))
      {
        new_solvable = _rdit->solvid != _sid;
        if (!in_repo)
          _sid = _rdit->solvid;
      }
      // no more attributes in this repo, return
      else
      {
        // check for more repos to jump to
        if (!_pqimpl->_repos.empty())
        {
          Repository nextRepo(Repository(_rdit->repo).nextInPool());
          if (nextRepo)
          {
            ::dataiterator_jump_to_repo(_rdit, nextRepo.get());
            _has_next = ::dataiterator_step(_rdit);
          }
        }

        // did the last solvable match conditions?
        return matches && in_repo;
      }
    }
    while (!new_solvable || !in_repo);

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
    _pimpl->_repos.insert(repoalias);
    _pimpl->_flags &= ~SEARCH_ALL_REPOS;
  }


  void PoolQuery::addKind(const Resolvable::Kind &kind)
  { _pimpl->_kinds.insert(kind); }


  void PoolQuery::addString(const string & value)
  { _pimpl->_strings.insert(value); }


  void PoolQuery::addAttribute(const sat::SolvAttr & attr, const std::string & value)
  { _pimpl->_attrs[attr].insert(value); }


  void PoolQuery::setCaseSensitive(const bool value)
  {
    if (value)
      _pimpl->_flags &= ~SEARCH_NOCASE;
    else
      _pimpl->_flags |= SEARCH_NOCASE;
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

  const PoolQuery::AttrMap &
  PoolQuery::attributes() const
  { return _pimpl->_attrs; }

  const PoolQuery::Kinds &
  PoolQuery::kinds() const
  { return _pimpl->_kinds; }

  const PoolQuery::StrContainer &
  PoolQuery::repos() const
  { return _pimpl->_repos; }

  bool PoolQuery::caseSensitive() const
  { return _pimpl->_flags & SEARCH_NOCASE; }

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


  bool PoolQuery::empty()
  { return _pimpl->begin() == _pimpl->end(); }

  //! \todo collect the result, reuse if not dirty
  PoolQuery::size_type PoolQuery::size()
  {
    size_type count = 0;
    for(const_iterator it = _pimpl->begin(); it != _pimpl->end(); ++it, ++count);
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

    //unknown atributes
    static const PoolQueryAttr noAttr;

    // own attributes
    static const PoolQueryAttr repoAttr;
    static const PoolQueryAttr kindAttr;
    static const PoolQueryAttr stringAttr;
    static const PoolQueryAttr stringTypeAttr;
    static const PoolQueryAttr requireAllAttr;
    static const PoolQueryAttr caseSensitiveAttr;
    static const PoolQueryAttr installStatusAttr;
  };

  const PoolQueryAttr PoolQueryAttr::noAttr;

  const PoolQueryAttr PoolQueryAttr::repoAttr( "repo" );
  const PoolQueryAttr PoolQueryAttr::kindAttr( "kind" );
  const PoolQueryAttr PoolQueryAttr::stringAttr( "global_string" );
  const PoolQueryAttr PoolQueryAttr::stringTypeAttr("string_type");
  const PoolQueryAttr PoolQueryAttr::requireAllAttr("require_all");
  const PoolQueryAttr PoolQueryAttr::caseSensitiveAttr("case_sensitive");
  const PoolQueryAttr PoolQueryAttr::installStatusAttr("install_status");

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

      if ( attribute==PoolQueryAttr::repoAttr )
      {
        addRepo( attrValue );
      }
      else if ( attribute==PoolQueryAttr::kindAttr )
      {
        addKind( Resolvable::Kind(attrValue) );
      }
      else if ( attribute==PoolQueryAttr::stringAttr )
      {
        addString( attrValue );
      }
      else if ( attribute==PoolQueryAttr::stringTypeAttr )
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
      else if ( attribute==PoolQueryAttr::noAttr )
      {
        WAR << "empty attribute name" << endl;
      }
      else
      {
        string s = attrName;
        boost::replace_all( s,"_",":" );
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
      str << "kind: " << it->idStr() << delim ;
    }

    if (matchType()!=q.matchType())
    {
      switch( matchType() )
      {
      case SEARCH_STRING:
        str << "string_type: exact" << delim;
        break;
      case SEARCH_SUBSTRING:
        str << "string_type: substring" << delim;
        break;
      case SEARCH_GLOB:
        str << "string_type: glob" << delim;
        break;
      case SEARCH_REGEX:
        str << "string_type: regex" << delim;
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
      str << "global_string: " << *it << delim;
    }

    for_( it, attributes().begin(), attributes().end() )
    {
      string s = it->first.asString();
      boost::replace_all(s,":","_"); 
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
    if( matchType()!=a.matchType() )
      return false;
    if( a.matchWord()!=matchWord())
      return false;
    if( a.requireAll()!=requireAll() )
      return false;
    if(!equalContainers(a.kinds(), kinds()))
      return false;
    if(!equalContainers(a.repos(), repos()))
      return false;
    if(a._pimpl->_rcstrings!=_pimpl->_rcstrings)
      return false;
    if(!equalContainers(a._pimpl->_rcattrs, _pimpl->_rcattrs))
      return false;
    if(a._pimpl->_cflags!= _pimpl->_cflags)
      return false;

    return true;
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

