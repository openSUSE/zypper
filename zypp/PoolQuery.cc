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

    Impl(PoolQuery::ProcessResolvable fnc)
      : _flags( 0 | SEARCH_NOCASE | SEARCH_SUBSTRING )
      , _fnc(fnc)
    {}

    ~Impl()
    {
      //MIL << std::endl;
    }

  public:

    static int repo_search_cb(void *cbdata, ::Solvable *s, ::Repodata *data, ::Repokey *key, ::KeyValue *kv)
    {
      //#define SEARCH_NEXT_KEY         1
      //#define SEARCH_NEXT_SOLVABLE    2
      //#define SEACH_STOP              3

      PoolQuery *me = (PoolQuery*) cbdata;
      bool r = me->_pimpl->_fnc(  makeResObject(sat::Solvable(s - sat::Pool::instance().get()->solvables)));
      
      if (!r)
        return SEARCH_STOP;
      return SEARCH_NEXT_SOLVABLE;
    }

    vector<string> _repos;
    vector<string> _names;
    vector<Resolvable::Kind> _kinds;
    int _flags;
    PoolQuery::ProcessResolvable _fnc;
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

  PoolQuery::PoolQuery( PoolQuery::ProcessResolvable fnc )
    : _pimpl(new Impl(fnc))
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

  void PoolQuery::setFlags(int flags)
  { _pimpl->_flags = flags; }

  
  void PoolQuery::execute(const string &term) const
  {
    
    sat::Pool pool(sat::Pool::instance());
    for ( sat::Pool::RepoIterator itr = pool.reposBegin();
          itr != pool.reposEnd();
          ++itr )
    {
      cout << "repo: " << itr->name() << endl;
      // is this repo in users repos?
      bool included = ( find(_pimpl->_repos.begin(), _pimpl->_repos.end(), itr->name()) != _pimpl->_repos.end() );
      // only look in user repos filter if the filter is not empty
      // in this case we search in all
      if ( _pimpl->_repos.empty() || included  )
      {
        // now search in all names
        //for ( vector<string>::const_iterator itn = _pimpl->_names.begin();
        //      itn != _pimpl->_names.end();
        //      ++itn )
        //{
          repo_search( itr->get(), 0, 0, term.c_str(), _pimpl->_flags, Impl::repo_search_cb, (void*) (this));
        //}
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

