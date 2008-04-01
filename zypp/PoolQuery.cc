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
#include <list>
#include <vector>
#include <algorithm>

#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/PoolQuery.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/sat/Pool.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/SolvAttr.h"

extern "C"
{
#include "satsolver/repo.h"
}

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  struct PoolQuery::Impl
  {

    Impl()
      : _flags( 0 | SEARCH_NOCASE | SEARCH_SUBSTRING )
      , _status_flags(ALL)
    {}

    ~Impl()
    {
      //MIL << std::endl;
    }

  public:

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
        r = me->_pimpl->_fnc( makeResObject(solvable) );
      
      if (!r)
        return SEARCH_STOP;
      return SEARCH_NEXT_SOLVABLE;
    }

    vector<string> _repos;
    vector<string> _names;
    vector<Resolvable::Kind> _kinds;
    int _flags;
    int _status_flags;
    mutable PoolQuery::ProcessResolvable _fnc;
  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PoolQuery::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PoolQuery::Impl & obj )
  {
    return str << "PoolQuery::Impl";
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

  void PoolQuery::addKind(const Resolvable::Kind &kind)
  { _pimpl->_kinds.push_back(kind); }

  void PoolQuery::setCaseSensitive(const bool value)
  {
    if (value)
      _pimpl->_flags = (_pimpl->_flags &  ~SEARCH_NOCASE);
    else
      _pimpl->_flags = (_pimpl->_flags | SEARCH_NOCASE);
  }

  void PoolQuery::setMatchExact(const bool value)
  {
    if (value)
    {
      _pimpl->_flags = (_pimpl->_flags | SEARCH_STRING);
      _pimpl->_flags = (_pimpl->_flags &  ~SEARCH_REGEX);
      _pimpl->_flags = (_pimpl->_flags &  ~SEARCH_SUBSTRING);
      _pimpl->_flags = (_pimpl->_flags &  ~SEARCH_GLOB);
    }
    else
    {
      _pimpl->_flags = (_pimpl->_flags & ~SEARCH_STRING);
    }
  }

  void PoolQuery::addRepo(const std::string &repoalias)
  {  _pimpl->_repos.push_back(repoalias);  }

  void PoolQuery::setFlags(int flags)
  { _pimpl->_flags = flags; }

  void PoolQuery::setInstalledOnly()
  {
    _pimpl->_status_flags = (_pimpl->_status_flags | INSTALLED_ONLY);
  }

  void PoolQuery::setUninstalledOnly()
  {
    _pimpl->_status_flags = (_pimpl->_status_flags | UNINSTALLED_ONLY);
  }

  void PoolQuery::setStatusFilterFlags( int flags )
  {
    _pimpl->_status_flags = (_pimpl->_status_flags | flags);
  }
  
  void PoolQuery::execute(const string &term, ProcessResolvable fnc) const
  {
    _pimpl->_fnc = fnc;

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
  }
  
  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : ostream &
  */
  ostream & operator<<( ostream & str, const PoolQuery & obj )
  {
    return str;
  }
  

  /**
   * represents all atributes in PoolQuery except SolvAtributes, which is
   * used as is (not needed extend anythink if someone add new solv attr)
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

  void PoolQuery::serialize( ostream &str, char delim )
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
     

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

