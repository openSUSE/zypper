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
#include <iostream>
#include <sstream>

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"

#include "zypp/sat/detail/PoolImpl.h"

#include "zypp/sat/Pool.h"
#include "zypp/sat/LookupAttr.h"
#include "zypp/sat/AttrMatcher.h"

#include "zypp/CheckSum.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    using detail::noSolvableId;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr::Impl
    //
    ///////////////////////////////////////////////////////////////////
    /**
     * LookupAttr implememtation.
     *
     * Repository and Solvable must not be set at the same time!
     *
     * \note When looking in pool or repo, \ref Solvable \c _solv is
     * somewhat abused to store eiter \c Id \c 0 or \c SOLVID_META, which
     * indicates whether the dataiterator should look into solvable or
     * repository metadata. Remember that all \ref Solvables with an
     * \e invalid \c Id, are treated as <tt>== Solvable::noSolvable</tt>,
     * and in a boolean context evaluate to \c false. Thus \c noSolvable
     * may have different \c Ids.
     */
    class LookupAttr::Impl
    {
      public:
        Impl()
        {}
        Impl( SolvAttr attr_r, Location loc_r )
        : _attr( attr_r ), _solv( loc_r == REPO_ATTR ? SOLVID_META : noSolvableId )
        {}
        Impl( SolvAttr attr_r, Repository repo_r, Location loc_r )
        : _attr( attr_r ), _repo( repo_r ), _solv( loc_r == REPO_ATTR ? SOLVID_META : noSolvableId )
        {}
        Impl( SolvAttr attr_r, Solvable solv_r )
        : _attr( attr_r ), _solv( solv_r )
        {}

      public:
        SolvAttr attr() const
        { return _attr; }

        void setAttr( SolvAttr attr_r )
        { _attr = attr_r; }

        const AttrMatcher & attrMatcher() const
        { return _attrMatcher; }

        void setAttrMatcher( const AttrMatcher & matcher_r )
        {
          matcher_r.compile();
          _attrMatcher = matcher_r;
        }

      public:
        bool pool() const
        { return ! (_repo || _solv); }

        void setPool( Location loc_r )
        {
          _repo = Repository::noRepository;
          _solv = Solvable( loc_r == REPO_ATTR ? SOLVID_META : noSolvableId );
        }

        Repository repo() const
        { return _repo; }

        void setRepo( Repository repo_r, Location loc_r  )
        {
          _repo = repo_r;
          _solv = Solvable( loc_r == REPO_ATTR ? SOLVID_META : noSolvableId );
        }

        Solvable solvable() const
        { return _solv; }

        void setSolvable( Solvable solv_r )
        {
          _repo = Repository::noRepository;
          _solv = solv_r;
        }

        LookupAttr::iterator begin() const
        {
          if ( _attr == SolvAttr::noAttr || sat::Pool::instance().reposEmpty() )
            return end();

          detail::RepoIdType whichRepo = detail::noRepoId; // all repos
          if ( _solv )
            whichRepo = _solv.repository().id();
          else if ( _repo )
            whichRepo = _repo.id();

          detail::DIWrap dip( whichRepo, _solv.id(), _attr.id(), _attrMatcher.searchstring(), _attrMatcher.flags().get() );
          return iterator( dip ); // iterator takes over ownership!
        }

        LookupAttr::iterator end() const
        { return iterator(); }

      private:
        SolvAttr   _attr;
        Repository _repo;
        Solvable   _solv;
        AttrMatcher _attrMatcher;

      private:
        friend Impl * rwcowClone<Impl>( const Impl * rhs );
        /** clone for RWCOW_pointer */
        Impl * clone() const
        { return new Impl( *this ); }
    };

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr
    //
    ///////////////////////////////////////////////////////////////////

    LookupAttr::LookupAttr()
      : _pimpl( new Impl )
    {}

    LookupAttr::LookupAttr( SolvAttr attr_r, Location loc_r )
      : _pimpl( new Impl( attr_r, loc_r ) )
    {}

    LookupAttr::LookupAttr( SolvAttr attr_r, Repository repo_r, Location loc_r )
      : _pimpl( new Impl( attr_r, repo_r, loc_r ) )
    {}

    LookupAttr::LookupAttr( SolvAttr attr_r, Solvable solv_r )
      : _pimpl( new Impl( attr_r, solv_r ) )
    {}

    ///////////////////////////////////////////////////////////////////

    SolvAttr LookupAttr::attr() const
    { return _pimpl->attr(); }

    void LookupAttr::setAttr( SolvAttr attr_r )
    { _pimpl->setAttr( attr_r ); }

    const AttrMatcher & LookupAttr::attrMatcher() const
    { return _pimpl->attrMatcher(); }

    void LookupAttr::setAttrMatcher( const AttrMatcher & matcher_r )
    { _pimpl->setAttrMatcher( matcher_r ); }

    ///////////////////////////////////////////////////////////////////

    bool LookupAttr::pool() const
    { return _pimpl->pool(); }

    void LookupAttr::setPool( Location loc_r )
    { _pimpl->setPool( loc_r ); }

    Repository LookupAttr::repo() const
    { return _pimpl->repo(); }

    void LookupAttr::setRepo( Repository repo_r, Location loc_r )
    { _pimpl->setRepo( repo_r, loc_r ); }

    Solvable LookupAttr::solvable() const
    { return _pimpl->solvable(); }

    void LookupAttr::setSolvable( Solvable solv_r )
    { _pimpl->setSolvable( solv_r ); }

    ///////////////////////////////////////////////////////////////////

    LookupAttr::iterator LookupAttr::begin() const
    { return _pimpl->begin(); }

    LookupAttr::iterator LookupAttr::end() const
    { return _pimpl->end(); }

    bool LookupAttr::empty() const
    { return begin() == end(); }

    LookupAttr::size_type LookupAttr::size() const
    {
      size_type c = 0;
      for_( it, begin(), end() )
        ++c;
      return c;
    }

    ///////////////////////////////////////////////////////////////////

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
    //	CLASS NAME : LookupRepoAttr
    //
    ///////////////////////////////////////////////////////////////////

    LookupRepoAttr::LookupRepoAttr( SolvAttr attr_r, Repository repo_r )
      : LookupAttr( attr_r, repo_r, REPO_ATTR )
    {}

    void LookupRepoAttr::setRepo( Repository repo_r )
    { LookupAttr::setRepo( repo_r, REPO_ATTR ); }

    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : detail::DIWrap
    //
    ///////////////////////////////////////////////////////////////////

    namespace detail
    {
      DIWrap::DIWrap( RepoIdType repoId_r, SolvableIdType solvId_r, IdType attrId_r,
                      const std::string & mstring_r, int flags_r )
      : _dip( new ::Dataiterator )
      , _mstring( mstring_r )
      {
        ::dataiterator_init( _dip, sat::Pool::instance().get(), repoId_r, solvId_r, attrId_r,
                             _mstring.empty() ? 0 : _mstring.c_str(), flags_r );
      }

      DIWrap::DIWrap( RepoIdType repoId_r, SolvableIdType solvId_r, IdType attrId_r,
                      const char * mstring_r, int flags_r )
      : _dip( new ::Dataiterator )
      , _mstring( mstring_r ? mstring_r : "" )
      {
        ::dataiterator_init( _dip, sat::Pool::instance().get(), repoId_r, solvId_r, attrId_r,
                             _mstring.empty() ? 0 : _mstring.c_str(), flags_r );
      }

      DIWrap::DIWrap( const DIWrap & rhs )
        : _dip( 0 )
        , _mstring( rhs._mstring )
      {
        if ( rhs._dip )
        {
          _dip = new ::Dataiterator;
          *_dip = *rhs._dip;
          // now we have to manually clone any allocated regex data matcher.
          ::Datamatcher & matcher( _dip->matcher );
          if ( matcher.match && ( matcher.flags & SEARCH_STRINGMASK ) == SEARCH_REGEX )
          {
            ::datamatcher_init( &matcher, _mstring.c_str(), matcher.flags );
          }
          else if ( matcher.match && matcher.match != _mstring.c_str() )
          {
            //SEC << "**" << rhs._dip << endl;
            SEC << "r " << rhs._dip->matcher.match << endl;
            SEC << "r " << rhs._dip->matcher.flags << endl;
            SEC << "r " << (const void*)rhs._mstring.c_str() << "'" << rhs._mstring << "'" << endl;

            SEC << "t " << matcher.match << endl;
            SEC << "t " << matcher.flags << endl;
            SEC << "t " << (const void*)_mstring.c_str() << "'" << _mstring << "'" <<  endl;
            throw( "this cant be!" );
          }
        }
      }

      DIWrap::~DIWrap()
      {
        if ( _dip )
        {
          ::dataiterator_free( _dip );
          delete _dip;
        }
      }

      std::ostream & operator<<( std::ostream & str, const DIWrap & obj )
      { return str << obj.get(); }
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

    bool LookupAttr::iterator::solvAttrSubEntry() const
    {
      return solvAttrType() == REPOKEY_TYPE_FLEXARRAY;
    }

    ///////////////////////////////////////////////////////////////////
    // Iterate sub-structures.
    ///////////////////////////////////////////////////////////////////

    bool LookupAttr::iterator::subEmpty() const
    { return( subBegin() == subEnd() ); }

    LookupAttr::size_type LookupAttr::iterator::subSize() const
    {
      size_type c = 0;
      for_( it, subBegin(), subEnd() )
        ++c;
      return c;
    }

    LookupAttr::iterator LookupAttr::iterator::subBegin() const
    {
      if ( ! solvAttrSubEntry() )
        return subEnd();

      // remember this position
      ::dataiterator_setpos( _dip.get() );

      // setup the new sub iterator with the remembered position
      detail::DIWrap dip( 0, SOLVID_POS, 0, 0, 0 );
      return iterator( dip ); // iterator takes over ownership!
    }

    LookupAttr::iterator LookupAttr::iterator::subEnd() const
    {
      return iterator();
    }

    LookupAttr::iterator LookupAttr::iterator::subFind( SolvAttr attr_r ) const
    {
      iterator it = subBegin();
      if ( attr_r != sat::SolvAttr::allAttr )
      {
        while ( it != subEnd() && it.inSolvAttr() != attr_r )
          ++it;
      }
      return it;
    }

    LookupAttr::iterator LookupAttr::iterator::subFind( const C_Str & attrname_r ) const
    {
      if ( attrname_r.empty() )
        return subBegin();

      std::string subattr( inSolvAttr().asString() );
      subattr += ":";
      subattr += attrname_r;
      return subFind( SolvAttr( subattr ) );
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
              return asCheckSum().asString();
            }
            break;

          case REPOKEY_TYPE_FLEXARRAY:
            {
              std::ostringstream str;
              str << "{" << endl;
              for_( it, subBegin(), subEnd() )
              {
                str << "  " << it.inSolvAttr() << " = " << it.asString() << endl;
              }
              str << "}" << endl;
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

    LookupAttr::iterator::iterator()
    : iterator_adaptor_( 0 )
    {}

    LookupAttr::iterator::iterator( const iterator & rhs )
    : iterator_adaptor_( 0 )
    , _dip( rhs._dip )
    {
      base_reference() = _dip.get();
    }

    LookupAttr::iterator::iterator( detail::DIWrap & dip_r )
    : iterator_adaptor_( 0 )
    {
      _dip.swap( dip_r ); // take ownership!
      base_reference() = _dip.get();
      increment();
    }

    LookupAttr::iterator::~iterator()
    {}

    LookupAttr::iterator & LookupAttr::iterator::operator=( const iterator & rhs )
    {
      if ( &rhs != this )
      {
        _dip = rhs._dip;
        base_reference() = _dip.get();
      }
      return *this;
    }

    ///////////////////////////////////////////////////////////////////

    bool LookupAttr::iterator::dip_equal( const ::_Dataiterator & lhs, const ::_Dataiterator & rhs ) const
    {
      // Iterator equal is same position in same container.
      // Here: same attribute in same solvable.
      return( lhs.solvid == rhs.solvid && lhs.key->name == rhs.key->name );
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
        _dip.reset();
        base_reference() = 0;
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
          << ">(" <<  IdString(obj.solvAttrType()) << ") = " << obj.asString();
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

std::ostream & operator<<( std::ostream & str, const ::_Dataiterator * obj )
{
  str << "::_Dataiterator(";
  if ( ! obj )
  {
    str << "NULL";
  }
  else
  {
    str << "|" << zypp::Repository(obj->repo);
    str << "|" << zypp::sat::Solvable(obj->solvid);
    str << "|" << zypp::IdString(obj->key->name);
    str << "|" << zypp::IdString(obj->key->type);
    str << "|" << obj->repodataid;
    str << "|" << obj->repoid;
  }
  return str << ")";
}

///////////////////////////////////////////////////////////////////
