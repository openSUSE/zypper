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

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

