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
#include <sstream>

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"

#include "zypp/sat/detail/PoolImpl.h"

#include "zypp/sat/LookupAttr.h"
#include "zypp/CheckSum.h"

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

#warning Need to call dataiterator_free
      scoped_ptr< ::_Dataiterator> dip( new ::Dataiterator );
      // needed while LookupAttr::iterator::dip_equal does ::memcmp:
      ::memset( dip.get(), 0, sizeof(::_Dataiterator) );
      bool chain = false;

      if ( _solv )
        ::dataiterator_init( dip.get(), _solv.repository().id(), _solv.id(), _attr.id(), 0, 0 );
      else if ( _repo )
        ::dataiterator_init( dip.get(), _repo.id(), 0, _attr.id(), 0, 0 );
      else if ( ! sat::Pool::instance().reposEmpty() )
      {
        ::dataiterator_init( dip.get(), sat::Pool::instance().reposBegin()->id(), 0, _attr.id(), 0, 0 );
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

    bool LookupAttr::empty() const
    { return begin() == end(); }

    LookupAttr::size_type LookupAttr::size() const
    {
      size_type c = 0;
      for_( it, begin(), end() )
        ++c;
      return c;
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
      return dumpRange( str << obj, obj.begin(), obj.end() );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr::iterator
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // position and moving
    ///////////////////////////////////////////////////////////////////

    Repository LookupAttr::iterator::inRepo() const
    { return _dip ? Repository( _dip->repo ) : Repository::noRepository; }

    Solvable LookupAttr::iterator::inSolvable() const
    { return _dip ? Solvable( _dip->solvid ) : Solvable::noSolvable; }

    SolvAttr LookupAttr::iterator::inSolvAttr() const
    { return _dip ? SolvAttr( _dip->key->name ) : SolvAttr::noAttr; }

    void LookupAttr::iterator::nextSkipSolvAttr()
    { if ( _dip ) ::dataiterator_skip_attribute( _dip.get() ); }

    void LookupAttr::iterator::nextSkipSolvable()
    { if ( _dip ) ::dataiterator_skip_solvable( _dip.get() ); }

    void LookupAttr::iterator::nextSkipRepo()
    { if ( _dip ) ::dataiterator_skip_repo( _dip.get() ); }

    ///////////////////////////////////////////////////////////////////
    // attr value type test
    ///////////////////////////////////////////////////////////////////

    detail::IdType LookupAttr::iterator::solvAttrType() const
    { return _dip ? _dip->key->type : detail::noId; }

    bool LookupAttr::iterator::solvAttrNumeric() const
    {
      switch ( solvAttrType() )
      {
        case REPOKEY_TYPE_U32:
        case REPOKEY_TYPE_NUM:
        case REPOKEY_TYPE_CONSTANT:
          return true;
          break;
      }
      return false;
    }

    bool LookupAttr::iterator::solvAttrString() const
    {
      switch ( solvAttrType() )
      {
        case REPOKEY_TYPE_ID:
        case REPOKEY_TYPE_IDARRAY:
        case REPOKEY_TYPE_CONSTANTID:
        case REPOKEY_TYPE_STR:
        case REPOKEY_TYPE_DIRSTRARRAY:
          return true;
          break;
      }
      return false;
    }

    bool LookupAttr::iterator::solvAttrIdString() const
    {
      switch ( solvAttrType() )
      {
        case REPOKEY_TYPE_ID:
        case REPOKEY_TYPE_IDARRAY:
        case REPOKEY_TYPE_CONSTANTID:
          return true;
          break;
      }
      return false;
    }

    bool LookupAttr::iterator::solvAttrCheckSum() const
    {
      switch ( solvAttrType() )
      {
        case REPOKEY_TYPE_MD5:
        case REPOKEY_TYPE_SHA1:
        case REPOKEY_TYPE_SHA256:
          return true;
          break;
      }
      return false;
    }

    ///////////////////////////////////////////////////////////////////
    // attr value retrieval
    ///////////////////////////////////////////////////////////////////

    int LookupAttr::iterator::asInt() const
    {
      if ( _dip )
      {
        switch ( solvAttrType() )
        {
          case REPOKEY_TYPE_U32:
          case REPOKEY_TYPE_NUM:
          case REPOKEY_TYPE_CONSTANT:
            return _dip->kv.num;
            break;
        }
      }
      return 0;
    }

    unsigned LookupAttr::iterator::asUnsigned() const
    { return asInt(); }

    bool LookupAttr::iterator::asBool() const
    { return asInt(); }


    const char * LookupAttr::iterator::c_str() const
    {
      if ( _dip )
      {
        switch ( solvAttrType() )
        {
          case REPOKEY_TYPE_ID:
          case REPOKEY_TYPE_IDARRAY:
          case REPOKEY_TYPE_CONSTANTID:
            if ( _dip->data && _dip->data->localpool )
              return ::stringpool_id2str( &_dip->data->spool, _dip->kv.id ); // in local pool
            else
              return IdString( _dip->kv.id ).c_str(); // in global pool
            break;

          case REPOKEY_TYPE_STR:
            return _dip->kv.str;
            break;

          case REPOKEY_TYPE_DIRSTRARRAY:
            return ::repodata_dir2str( _dip->data, _dip->kv.id, _dip->kv.str );
            break;
        }
      }
      return 0;
    }

    std::string LookupAttr::iterator::asString() const
    {
      if ( _dip )
      {
        switch ( solvAttrType() )
        {
          case REPOKEY_TYPE_ID:
          case REPOKEY_TYPE_IDARRAY:
          case REPOKEY_TYPE_CONSTANTID:
          case REPOKEY_TYPE_STR:
          case REPOKEY_TYPE_DIRSTRARRAY:
          {
            const char * ret( c_str() );
            return ret ? ret : "";
          }
          break;

          case REPOKEY_TYPE_U32:
          case REPOKEY_TYPE_NUM:
          case REPOKEY_TYPE_CONSTANT:
            return str::numstring( asInt() );
            break;

          case REPOKEY_TYPE_MD5:
          case REPOKEY_TYPE_SHA1:
          case REPOKEY_TYPE_SHA256:
          {
            std::ostringstream str;
            str << asCheckSum();
            return str.str();
          }
          break;
        }
      }
      return std::string();
    }

    IdString LookupAttr::iterator::idStr() const
    {
      if ( _dip )
      {
        switch ( solvAttrType() )
        {
          case REPOKEY_TYPE_ID:
          case REPOKEY_TYPE_IDARRAY:
          case REPOKEY_TYPE_CONSTANTID:
            return IdString( ::repodata_globalize_id( _dip->data, _dip->kv.id ) );
            break;
        }
      }
      return IdString();
    }

    CheckSum LookupAttr::iterator::asCheckSum() const
    {
      if ( _dip )
      {
        switch ( solvAttrType() )
        {
          case REPOKEY_TYPE_MD5:
            return CheckSum::md5( ::repodata_chk2str( _dip->data, solvAttrType(), (unsigned char *)_dip->kv.str ) );
            break;

          case REPOKEY_TYPE_SHA1:
            return CheckSum::sha1( ::repodata_chk2str( _dip->data, solvAttrType(), (unsigned char *)_dip->kv.str ) );
            break;

          case REPOKEY_TYPE_SHA256:
            return CheckSum::sha256( ::repodata_chk2str( _dip->data, solvAttrType(), (unsigned char *)_dip->kv.str ) );
            break;
        }
      }
      return CheckSum();
    }

    ///////////////////////////////////////////////////////////////////
    // internal stuff below
    ///////////////////////////////////////////////////////////////////

    LookupAttr::iterator::~iterator()
    {}

    LookupAttr::iterator::iterator()
    : iterator_adaptor_( 0 )
    {}

    LookupAttr::iterator::iterator( const iterator & rhs )
    : iterator_adaptor_( cloneFrom( rhs.base() ) )
    , _dip( base() )
    , _chainRepos(rhs._chainRepos)
    {}

    LookupAttr::iterator & LookupAttr::iterator::operator=( const iterator & rhs )
    {
      if ( &rhs != this )
      {
        _dip.reset( cloneFrom( rhs.base() ) );
        base_reference() = _dip.get();
        _chainRepos = rhs._chainRepos;
      }
      return *this;
    }

    LookupAttr::iterator::iterator( scoped_ptr< ::_Dataiterator> & dip_r, bool chain_r )
    : iterator_adaptor_( dip_r.get() )
    , _chainRepos( chain_r )
    {
      _dip.swap( dip_r ); // take ownership!
      increment();
    }

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
        return str << "EndOfQuery";

      if ( obj.inSolvable() )
        str << obj.inSolvable();
      else if ( obj.inRepo() )
        str << obj.inRepo();

      str << '<' << obj.inSolvAttr()
          << ">(" <<  obj.solvAttrType() << ") = " << obj.asString();
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
