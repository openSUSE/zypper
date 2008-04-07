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
#include <list>
#include <vector>
#include <algorithm>

#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/base/Regex.h"
#include "zypp/base/Algorithm.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/repo/RepoException.h"

#include "zypp/sat/Pool.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/SolvAttr.h"
#include "zypp/sat/detail/PoolImpl.h"

extern "C"
{
#include "satsolver/repo.h"
}

#include "zypp/PoolQuery.h"

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
/*
    static int repo_search_cb(void *cbdata, ::Solvable *s, ::Repodata *data, ::Repokey *key, ::KeyValue *kv)
    {
      PoolQuery *me = (PoolQuery*) cbdata;

      bool r = false;

      sat::Solvable solvable(s - sat::Pool::instance().get()->solvables);

      // now filter by kind here (we cant do it before)
      if ( ! me->_pimpl->_kinds.empty() )
      {
        // the user wants to filter by kind.
        if ( find( me->_pimpl->_kinds.begin(),
                   me->_pimpl->_kinds.end(),
                   solvable.kind() )
             == me->_pimpl->_kinds.end() )
        {
          // we did not find the kind in the list
          // so this is not a result.
          return SEARCH_NEXT_SOLVABLE;
        }
      }

      if (me->_pimpl->_fnc)
        r = me->_pimpl->_fnc( solvable );//makeResObject(solvable) );
      
      if (!r)
        return SEARCH_STOP;
      return SEARCH_NEXT_SOLVABLE;
    }
  */  
    ResultIterator begin();
    ResultIterator end();
    
    string asString() const;
    
  private:
    void compile();
    string createRegex(vector<string> & container);

  public:
    /** Raw search strings. */
    vector<string> _strings;
    /** Regex-compiled search strings. */
    string _rcstrings;
    /** Raw attributes */
    AttrMap _attrs;
    /** Regex-compiled attributes */
    CompiledAttrMap _rcattrs;

    /** Repos to search. */
    vector<string> _repos;
    /** Kinds to search */
    vector<Resolvable::Kind> _kinds;

    /** Sat solver search flags */
    int _flags;
    /** Backup of search flags. compile() may change the flags if needed, so
     * in order to reuse the query, the original flags need to be stored
     * at the start of compile() */
    int _flags_old;
    /** Sat solver status flags */
    int _status_flags;

    bool _match_word;

    bool _require_all;

    /** Sat solver Dataiterator structure */
    ::_Dataiterator _rdit;

    bool _compiled;

    /** Function for processing found solvables. Used in execute(). */
    mutable PoolQuery::ProcessResolvable _fnc;
  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
/*
  template <class _OutputIterator>
  struct CollectNonEmpty
  {
    CollectNonEmpty( _OutputIterator iter_r ) : _iter( iter_r ) {}

    template<class _Tp>
    bool operator()( const _Tp & value_r ) const
    {
      if (value_r.empty())
        return true;
      *_iter++ = value_r;
      return true;
    }

    private:
      mutable _OutputIterator _iter;
  };
*/
  void PoolQuery::Impl::compile()
  {
    // backup the flags
    _flags_old = _flags;

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
        _flags = (_flags & ~SEARCH_STRINGMASK) | SEARCH_REGEX;//setMatchRegex();
    }

    // // ONE ATTRIBUTE 
    // else if _attrs is not empty but it contains just one attr
    //   for all _strings and _attr[key] strings
    //     create regex; store in _rcattrs; flag 'one-attr'; if more strings flag regex;
    else if (_attrs.size() == 1)
    {
      vector<string> joined;
      for(vector<string>::const_iterator it = _strings.begin(); it != _strings.end(); ++it)
        if (!it->empty())
          joined.push_back(*it);
      for(vector<string>::const_iterator it = _attrs.begin()->second.begin(); it != _attrs.begin()->second.end(); ++it)
        if (!it->empty())
          joined.push_back(*it);
      _rcstrings = createRegex(joined);
      _rcattrs.insert(pair<sat::SolvAttr, string>(_attrs.begin()->first, string()));
    }


    // // MULTIPLE ATTRIBUTES
    else
    {
      bool attrvals_empty = true;
      for (AttrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai)
        if (!ai->second.empty())
          for(vector<string>::const_iterator it = ai->second.begin();
              it != ai->second.end(); it++)
            if (!it->empty())
            {
              attrvals_empty = false;
              goto attremptycheckend;
            }

attremptycheckend:

      bool attrvals_thesame = true;
      for (AttrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai)
      {
        
      }

      // // THE SAME STRINGS FOR DIFFERENT ATTRS
      // else if _attrs is not empty but it does not contain strings
      //   for each key in _attrs take all _strings
      //     create regex; store in _rcattrs and _rcstrings; flag 'same'; if more strings flag regex;
      if (attrvals_empty || attrvals_thesame)
      {
        if (attrvals_empty)
        {
          // compile the search string
          vector<string> joined;
          for(vector<string>::const_iterator it = _strings.begin(); it != _strings.end(); ++it)
            if (!it->empty())
              joined.push_back(*it);
          _rcstrings = createRegex(joined);

          // copy the _attrs keys to _rcattrs
          for (AttrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai)
            _rcattrs.insert(pair<sat::SolvAttr, string>(ai->first, string()));
        }
      }
      // // DIFFERENT STRINGS FOR DIFFERENT ATTRS
      // if _attrs is not empty and it contains non-empty vectors with non-empty strings
      //   for each key in _attrs take all _strings + all _attrs[key] strings
      //     create regex; store in _rcattrs; flag 'different'; if more strings flag regex;
      else
      {
        
      }
    }

    // tell the Dataiterator to search only in one repo if only one specified
    if (_repos.size() == 1)
      _flags &= ~SEARCH_ALL_REPOS;

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

  string PoolQuery::Impl::createRegex(vector<string> & container)
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

    bool _use_wildcards = (_flags & SEARCH_STRINGMASK) == SEARCH_GLOB;
    vector<string>::const_iterator it = container.begin();
    string tmp;

    if (_use_wildcards)
      tmp = wildcards2regex(*it);

    if (_require_all)
    {
      if (!(_flags & SEARCH_STRING)) // not match exact
        tmp += ".*" + WB + tmp;
      rstr = "(?=" + tmp + ")";
    }
    else
    {
      if (_flags & SEARCH_STRING) // match exact
        rstr = "^";
      else
        rstr = ".*" + WB;

      rstr += "(" + tmp;
    }

    ++it;

    for (; it != container.end(); ++it)
    {
      if (_use_wildcards)
        tmp = wildcards2regex(*it);

      if (_require_all)
      {
        if (!(_flags & SEARCH_STRING)) // not match exact
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
      if (!(_flags & SEARCH_STRING)) // not match exact
        rstr += WB + ".*";
    }
    else
    {
      rstr += ")";
      if (_flags & SEARCH_STRING) // match exact
        rstr += "$";
      else
        rstr += WB + ".*";
    }

    return rstr;
  }


  PoolQuery::ResultIterator PoolQuery::Impl::begin()
  {
    compile();

    // if only one repository has been specified, find it in the pool
    sat::Pool pool(sat::Pool::instance());
    sat::Pool::RepositoryIterator itr = pool.reposBegin();
    if (!(_flags & SEARCH_ALL_REPOS) && _repos.size() == 1)
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

    DBG << "_flags:" << _flags << endl;

    if (_rcattrs.empty())
    {
    ::dataiterator_init(&_rdit,
      _flags & SEARCH_ALL_REPOS ? pool.get()->repos[0] : itr->get(), // repository \todo fix this
      0,                                           // search all solvables
      0,                                           // attribute id - only if 1 attr key specified
      _rcstrings.empty() ? 0 : _rcstrings.c_str(), // compiled search string
      _flags);
    }
    else if (_rcattrs.size() == 1)
    {
      ::dataiterator_init(&_rdit,
        _flags & SEARCH_ALL_REPOS ? pool.get()->repos[0] : itr->get(), // repository \todo fix this 
        0,                                           // search all solvables
        _rcattrs.begin()->first.id(),                // keyname - attribute id - only if 1 attr key specified
        _rcstrings.empty() ? 0 : _rcstrings.c_str(), // compiled search string 
        _flags);
    }
    else
    {
      ::dataiterator_init(&_rdit,
        _flags & SEARCH_ALL_REPOS ? pool.get()->repos[0] : itr->get(), /* repository - switch to next at the end of current one in increment() */ 
        0, /*search all resolvables */
        0, /*keyname - if only 1 attr key specified, pass it here, otherwise do more magic */
        0, //qs.empty() ? 0 : qs.c_str(), /* create regex, pass it here */
        _flags);
    }

    PoolQuery::ResultIterator it(this);
    it.increment();
    return it;
  }

  PoolQuery::ResultIterator PoolQuery::Impl::end()
  {
    INT << "end" << endl;
    return PoolQuery::ResultIterator();
  }


  string PoolQuery::Impl::asString() const
  {
    ostringstream o;

    o << "compiled: " << _compiled << endl;

    o << "string match flags:" << endl;
    o << "* sat: " << (_flags & SEARCH_STRINGMASK) << endl; 
    o << "* SEARCH_REGEX: " << ((_flags & SEARCH_STRINGMASK) == SEARCH_REGEX ? "yes" : "no") << endl;
    o << "status filter flags:" << _status_flags << endl; 

    // raw
    
    o << "strings: ";
    for(vector<string>::const_iterator it = _strings.begin();
        it != _strings.end(); ++it)
      o << *it << " ";
    o << endl;

    o << "attributes: " << endl;
    for(AttrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai)
    {
      o << "* " << ai->first << ": ";
      for(vector<string>::const_iterator vi = ai->second.begin();
          vi != ai->second.end(); ++vi)
        o << *vi << " ";
      o << endl;
    }

    // compiled

    o << "regex compiled strings: " << _rcstrings << endl;
    o << "regex compiled attributes:" << endl;
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
  //
  //  CLASS NAME : PoolQuery::ResultIterator
  //
  ///////////////////////////////////////////////////////////////////

  PoolQuery::ResultIterator::ResultIterator(Impl * pqimpl)
  : PoolQuery::ResultIterator::iterator_adaptor_(pqimpl ? &pqimpl->_rdit : 0)
  , _rdit(pqimpl ? &pqimpl->_rdit : 0)
  , _pqimpl(pqimpl)
  , _sid(0)
  , _has_next(true)
  , _attrs(pqimpl->_rcattrs)
  , _do_matching(false)
  , _pool((sat::Pool::instance()))
  {
    if (_attrs.size() > 1)
      _do_matching = true;
  }

  void PoolQuery::ResultIterator::increment()
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

  bool PoolQuery::ResultIterator::matchSolvable()
  {
    _sid = _rdit->solvid;

    bool new_solvable = true;
    bool matches = !_do_matching;
    bool in_repo;
    bool drop_by_kind_status;
    do
    {
      //! \todo FIXME Dataiterator returning resolvables belonging to current repo?
      in_repo = _sid >= _rdit->repo->start;

      if (in_repo && new_solvable)
      {
        drop_by_kind_status = false;

        // filter by installed uninstalled
        if ( (_pqimpl->_status_flags & INSTALLED_ONLY) &&
             _rdit->repo->name != _pool.systemRepoName() )
          drop_by_kind_status = true;

        if (!drop_by_kind_status)
          if ( (_pqimpl->_status_flags & UNINSTALLED_ONLY) &&
               _rdit->repo->name == _pool.systemRepoName() )
            drop_by_kind_status = true;

        matches = matches && !drop_by_kind_status;
      }

      if (_do_matching && !drop_by_kind_status)
      {
        if (!matches && in_repo)
        {
          SolvAttr attr(_rdit->key->name);

          CompiledAttrMap::const_iterator ai = _attrs.find(attr);
          if (ai != _attrs.end())
          {
            // exact match
            //matches = (ai->second == IdString(_rdit->kv.id).asString());
            // substring
            matches = (
              IdString(_rdit->kv.id).asString().find
                (_pqimpl->_rcstrings.empty() ? ai->second : _pqimpl->_rcstrings)
                != string::npos);
            if (matches)
              INT << "value: " << IdString(_rdit->kv.id).asString() << endl
                  << " mstr: " <<  (_pqimpl->_rcstrings.empty() ? ai->second : _pqimpl->_rcstrings) << endl; 
            // regex
          }
        }
      }

      if ((_has_next = ::dataiterator_step(_rdit)))
      {
        new_solvable = _rdit->solvid != _sid;
        if (!in_repo)
        {
          INT << "repo start: " << _rdit->repo->start << endl;
          _sid = _rdit->solvid;
        }
      }
      // no more attributes/solvables, return
      else
      {
        // did the last solvable match conditions?
        return matches && in_repo;
      }
    }
    while (!new_solvable || !in_repo);

    return matches;
  }

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
    _pimpl->_repos.push_back(repoalias);
    _pimpl->_flags &= ~SEARCH_ALL_REPOS;
  }


  void PoolQuery::addKind(const Resolvable::Kind &kind)
  { _pimpl->_kinds.push_back(kind); }


  void PoolQuery::addString(const string & value)
  { _pimpl->_strings.push_back(value); }


  void PoolQuery::addAttribute(const sat::SolvAttr & attr, const std::string & value)
  { _pimpl->_attrs[attr].push_back(value); }


  void PoolQuery::setCaseSensitive(const bool value)
  {
    if (value)
      _pimpl->_flags &= ~SEARCH_NOCASE;
    else
      _pimpl->_flags |= SEARCH_NOCASE;
  }


  void PoolQuery::setMatchSubstring()
  { _pimpl->_flags |= (_pimpl->_flags & ~SEARCH_STRINGMASK) | SEARCH_SUBSTRING; }
  void PoolQuery::setMatchExact()
  { _pimpl->_flags |= (_pimpl->_flags & ~SEARCH_STRINGMASK) | SEARCH_STRING; }
  void PoolQuery::setMatchRegex()
  { _pimpl->_flags |= (_pimpl->_flags & ~SEARCH_STRINGMASK) | SEARCH_REGEX; }
  void PoolQuery::setMatchGlob()
  { _pimpl->_flags |= (_pimpl->_flags & ~SEARCH_STRINGMASK) | SEARCH_GLOB; }
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
  void PoolQuery::setStatusFilterFlags( int flags )
  { _pimpl->_status_flags = flags; }


  void PoolQuery::requireAll(const bool require_all)
  { _pimpl->_require_all = require_all; }


  PoolQuery::ResultIterator PoolQuery::begin()
  { return _pimpl->begin(); }


  PoolQuery::ResultIterator PoolQuery::end()
  { return _pimpl->end(); }


  bool PoolQuery::empty()
  { return _pimpl->begin() == _pimpl->end(); }

  //! \todo collect the result, reuse if not dirty
  PoolQuery::size_type PoolQuery::size()
  {
    size_type count = 0;
    for(ResultIterator it = _pimpl->begin(); it != _pimpl->end(); ++it, ++count);
    return count;
  }


  void PoolQuery::execute(ProcessResolvable fnc)
  {
    invokeOnEach(_pimpl->begin(), _pimpl->end(), fnc);
    /*
    _pimpl->_fnc = fnc;
    string term;
    if (!_pimpl->_strings.empty())
      term = *_pimpl->_strings.begin();

    sat::Pool pool(sat::Pool::instance());
    for ( sat::Pool::RepositoryIterator itr = pool.reposBegin();
          itr != pool.reposEnd();
          ++itr )
    {
      // filter by installed uninstalled
      if ( ( _pimpl->_status_flags & INSTALLED_ONLY ) && (itr->name() != sat::Pool::instance().systemRepoName()) )
        continue;

      if ( ( _pimpl->_status_flags & UNINSTALLED_ONLY ) && (itr->name() == sat::Pool::instance().systemRepoName()) )
        continue;

      // is this repo in users repos?
      bool included = ( find(_pimpl->_repos.begin(), _pimpl->_repos.end(), itr->name()) != _pimpl->_repos.end() );

      // only look in user repos filter if the filter is not empty
      // in this case we search in all
      if ( _pimpl->_repos.empty() || included  )
      {
        repo_search( itr->get(), 0, 0, term.c_str(), _pimpl->_flags, Impl::repo_search_cb, (void*) (this));
      }

    }
    */
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : PoolQuery::Impl
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
    PoolQueryAttr():isSolvAttr(false){}

    explicit PoolQueryAttr( const char* cstr_r )
        : _str( cstr_r ),isSolvAttr(false){}

    explicit PoolQueryAttr( const std::string & str_r )
        : _str( str_r ),isSolvAttr(false)
    {
      if( _str==noAttr ){
        sat::SolvAttr sa(str_r);
        if( sa != sat::SolvAttr::noAttr )
        {
          isSolvAttr = true; 
        }
      }
    }

    //unknown atributes
    static const PoolQueryAttr noAttr;

    // own attributes
    static const PoolQueryAttr nameAttr;
    static const PoolQueryAttr repoAttr;
    static const PoolQueryAttr kindAttr;

    // exported attributes from SolvAtributes
    bool isSolvAttr;
  };

  const PoolQueryAttr PoolQueryAttr::noAttr;

  const PoolQueryAttr PoolQueryAttr::nameAttr( "name" );
  const PoolQueryAttr PoolQueryAttr::repoAttr( "repo" );
  const PoolQueryAttr PoolQueryAttr::kindAttr( "kind" );

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

      string atrName(str::trim(string(s,0,pos))); // trimmed name of atribute
      string atrValue(str::trim(string(s,pos+1,s.npos))); //trimmed value

      PoolQueryAttr attribute( atrName );

      if ( attribute==PoolQueryAttr::nameAttr)
      {
        //setName...maybe some regex test
        break;
      }
      else if ( attribute==PoolQueryAttr::repoAttr )
      {
        addRepo( atrValue );
      }
      else if ( attribute==PoolQueryAttr::kindAttr )
      {
        addKind( Resolvable::Kind(atrValue) );
      }
      else if ( attribute==PoolQueryAttr::noAttr )
      {
        if (attribute.isSolvAttr)
        {
          //setAtribute
        }
        else
        {
          //log unknwon atribute
        }
      }
      else
      {
        //some forget handle new atribute
        ;
      }
      
    } while ( true );

    return finded_something;
  }


  void PoolQuery::serialize( ostream &str, char delim ) const
  {
    //separating delim
    str << delim; 
    //iterate thrue all settings and write it
    
    for_( it, _pimpl->_repos.begin(), _pimpl->_repos.end() )
    {
      str << "repo: " << *it << delim ;
    }

    for_( it, _pimpl->_kinds.begin(), _pimpl->_kinds.end() )
    {
      str << "kind: " << it->idStr() << delim ;
    }

    //separating delim - protection
    str << delim; 

  }


  string PoolQuery::asString() const
  { return _pimpl->asString(); }


  ostream & operator<<( ostream & str, const PoolQuery & obj )
  { return str << obj.asString(); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

