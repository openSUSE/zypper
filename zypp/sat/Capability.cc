/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Capability.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/base/String.h"
#include "zypp/base/Regex.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/Rel.h"
#include "zypp/Edition.h"
#include "zypp/Capability.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Pool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    sat::detail::IdType relFromStr( ::_Pool * pool_r,
                                    const std::string & name_r, Rel op_r, const Edition & ed_r, const ResKind & kind_r )
    {
      sat::detail::IdType nid( sat::detail::noId );
      if ( ! kind_r || kind_r == ResKind::package )
      {
        nid = IdString( name_r ).id();
      }
      else
      {
        // non-packages prefixed by kind
        nid = IdString( str::form( "%s:%s",
                        kind_r.c_str(),
                                     name_r.c_str() ) ).id();
      }

      if ( op_r != Rel::ANY && ed_r != Edition::noedition )
      {
        nid = ::rel2id( pool_r, nid, ed_r.idStr().id(), op_r.bits(), /*create*/true );
      }

      return nid;
    }

    sat::detail::IdType relFromStr( ::_Pool * pool_r,
                                      const std::string & str_r, const ResKind & kind_r, Capability::CtorFlag flag_r )
    {
      // strval_r has at least two words which could make 'op edition'?
      // improve regex!
      static const str::regex  rx( "(.*[^ \t])([ \t]+)([^ \t]+)([ \t]+)([^ \t]+)" );
      static str::smatch what;

      std::string name( str_r );
      Rel         op;
      Edition     ed;
      if ( flag_r == Capability::UNPARSED
           && str_r.find(' ') != std::string::npos
           && str::regex_match( str_r, what, rx ) )
      {
        try
        {
          Rel     cop( what[3] );
          Edition ced( what[5] );
          name = what[1];
          op = cop;
          ed = ced;
        }
        catch ( Exception & excpt )
        {
          // So they don't make valid 'op edition'
          ZYPP_CAUGHT( excpt );
          DBG << "Trying named relation for: " << str_r << endl;
        }
      }
      //else
      // not a versioned relation

      return relFromStr( pool_r, name, op, ed, kind_r );
    }

    // By now restrict matching to plain 'name [op edition]'
    struct CapHelp : protected sat::detail::PoolMember
    {
      CapHelp( sat::detail::IdType id_r )
      : _name( id_r )
      {
        if ( ISRELDEP(id_r) )
        {
          ::Reldep * rd = GETRELDEP( myPool().getPool(), id_r );
          if ( ! Rel::isRel( rd->flags ) || ISRELDEP(rd->name) || ISRELDEP(rd->evr) )
            _op = Rel::NONE;
          else
          {
            _name = IdString( rd->name );
            _op   = Rel( rd->flags );
            _ed   = Edition( rd->evr );
          }
        }
      }

      IdString _name;
      Rel      _op;
      Edition  _ed;
    };

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  const Capability Capability::Null( STRID_NULL );

  /////////////////////////////////////////////////////////////////

  Capability::Capability( const char * str_r, const ResKind & prefix_r, CtorFlag flag_r )
  : _id( relFromStr( myPool().getPool(), str_r, prefix_r, flag_r ) )
  {}

  Capability::Capability( const std::string & str_r, const ResKind & prefix_r, CtorFlag flag_r )
  : _id( relFromStr( myPool().getPool(), str_r.c_str(), prefix_r, flag_r ) )
  {}

  Capability::Capability( const char * str_r, CtorFlag flag_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), str_r, prefix_r, flag_r ) )
  {}

  Capability::Capability( const std::string & str_r, CtorFlag flag_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), str_r, prefix_r, flag_r ) )
  {}


  Capability::Capability( const std::string & name_r, const std::string & op_r, const std::string & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), name_r, Rel(op_r), Edition(ed_r), prefix_r ) )
  {}

  Capability::Capability( const std::string & name_r, Rel op_r, const std::string & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), name_r, op_r, Edition(ed_r), prefix_r ) )
  {}

  Capability::Capability( const std::string & name_r, Rel op_r, const Edition & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), name_r, op_r, ed_r, prefix_r ) )
  {}

  const char * Capability::c_str() const
  { return ::dep2str( myPool().getPool(), _id ); }

  std::string Capability::string() const
  { return ::dep2str( myPool().getPool(), _id ); }

  bool Capability::_doMatch( sat::detail::IdType lhs,  sat::detail::IdType rhs )
  {
#warning MIGRATE TO SAT
#warning TESTCASE
    if ( lhs == rhs )
      return true;

    CapHelp l( lhs );
    if ( l._op == Rel::NONE )
      return false;

    CapHelp r( rhs );
    if ( r._op == Rel::NONE )
      return false;

    if ( l._name != r._name )
      return false;

    if ( l._op == Rel::ANY || r._op == Rel::ANY )
      return true;

    return overlaps( Edition::MatchRange( l._op, l._ed ),
                     Edition::MatchRange( r._op, r._ed ) );
  }

  bool Capability::isInterestingFileSpec( const char * name_r )
  {
    static       str::smatch what;
    static const str::regex  filenameRegex(
                 "/(s?bin|lib(64)?|etc)/|^/usr/(games/|share/(dict/words|magic\\.mime)$)|^/opt/gnome/games/",
                 str::regex::optimize|str::regex::nosubs );

    return str::regex_match( name_r, what, filenameRegex );
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const Capability & obj )
  {
    return str << obj.c_str();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
