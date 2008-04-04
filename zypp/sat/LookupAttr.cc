/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/LookupAttr.cc
 *
*/
#include <cstring>
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/sat/detail/PoolImpl.h"

#include "zypp/sat/LookupAttr.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    LookupAttr::iterator LookupAttr::begin() const
    {
      if ( _attr == SolvAttr::noAttr )
        return iterator();

      scoped_ptr< ::_Dataiterator> dip( new ::Dataiterator );
      // needed while LookupAttr::iterator::dip_equal does ::memcmp:
      ::memset( dip.get(), 0, sizeof(::_Dataiterator) );
      bool chain = false;

      if ( _solv )
        ::dataiterator_init( dip.get(), _solv.repository().id(), _solv.id(), _attr.id(), 0, SEARCH_NO_STORAGE_SOLVABLE );
      else if ( _repo )
        ::dataiterator_init( dip.get(), _repo.id(), 0, _attr.id(), 0, SEARCH_NO_STORAGE_SOLVABLE );
      else if ( ! sat::Pool::instance().reposEmpty() )
      {
        ::dataiterator_init( dip.get(), sat::Pool::instance().reposBegin()->id(), 0, _attr.id(), 0, SEARCH_NO_STORAGE_SOLVABLE );
        chain = true;
      }
      else
        return iterator();

      return iterator( dip, chain ); // iterator takes over ownership!
    }

    LookupAttr::iterator LookupAttr::end() const
    {
      return iterator();
    }

    std::ostream & operator<<( std::ostream & str, const LookupAttr & obj )
    {
      if ( obj.attr() == SolvAttr::noAttr )
        return str << "search nothing";

      if ( obj.attr() )
        str << "seach " << obj.attr() << " in ";
      else
        str << "seach ALL in ";

      if ( obj.solvable() )
        return str << obj.solvable();
      if ( obj.repo() )
        return str << obj.repo();
      return str << "pool";
    }

    std::ostream & dumpOn( std::ostream & str, const LookupAttr & obj )
    {
      str << obj << endl;
      for_( it, obj.begin(), obj.end() )
      {
        str << "  " << it << endl;
      }
      return str << "<EndOfSerach>";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr::iterator
    //
    ///////////////////////////////////////////////////////////////////

    Repository LookupAttr::iterator::inRepo() const
    { return Repository( _dip->repo ); }

    Solvable LookupAttr::iterator::inSolvable() const
    { return Solvable( _dip->solvid ); }

    SolvAttr LookupAttr::iterator::inSolvAttr() const
    { return SolvAttr( _dip->key->name ); }

    detail::IdType LookupAttr::iterator::solvAttrType() const
    { return _dip->key->type; }

    void LookupAttr::iterator::nextSkipSolvAttr()
    { ::dataiterator_skip_attribute( _dip.get() ); }

    void LookupAttr::iterator::nextSkipSolvable()
    { ::dataiterator_skip_solvable( _dip.get() ); }

    void LookupAttr::iterator::nextSkipRepo()
    { ::dataiterator_skip_repo( _dip.get() ); }

    ///////////////////////////////////////////////////////////////////
    // internal stuff below
    ///////////////////////////////////////////////////////////////////

    ::_Dataiterator * LookupAttr::iterator::cloneFrom( const ::_Dataiterator * rhs )
    {
      if ( ! rhs )
        return 0;
      ::_Dataiterator * ret( new ::_Dataiterator );
      *ret = *rhs;
      return ret;
    }

    bool LookupAttr::iterator::dip_equal( const ::_Dataiterator & lhs, const ::_Dataiterator & rhs ) const
    {
      // requires ::memset in LookupAttr::begin
      return ::memcmp( &lhs, &rhs, sizeof(::_Dataiterator) ) == 0;
    }

    detail::IdType LookupAttr::iterator::dereference() const
    {
      return _dip ? ::repodata_globalize_id( _dip->data, _dip->kv.id )
                  : detail::noId;
    }

    void LookupAttr::iterator::increment()
    {
      if ( _dip && ! ::dataiterator_step( _dip.get() ) )
      {
        bool haveNext = false;
        if ( _chainRepos )
        {
          Repository nextRepo( inRepo().nextInPool() );
          if ( nextRepo )
          {
            ::dataiterator_jump_to_repo( _dip.get(), nextRepo.get() );
            haveNext = ::dataiterator_step( _dip.get() );
          }
        }
        if ( ! haveNext )
        {
          _dip.reset();
          base_reference() = 0;
        }
      }
    }

    std::ostream & operator<<( std::ostream & str, const LookupAttr::iterator & obj )
    {
      const ::_Dataiterator * dip = obj.get();
      if ( ! dip )
        return str << "EndOfQuery" << endl;

      str << obj.inSolvable()
          << '<' << obj.inSolvAttr()
          << "> = " << obj.solvAttrType()
          << "(" <<  dip->kv.id << ")" << (dip->data && dip->data->localpool ? "*" : "" );
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
