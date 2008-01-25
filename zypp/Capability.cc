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

    CapDetail l( lhs );
    if ( ! l.isSimple() )
      return false;

    CapDetail r( rhs );
    if ( r.isSimple() )
      return false;

    if ( l.name() != r.name() )
      return false;

    if ( l.isNamed() || r.isNamed() )
      return true;

    // both are versioned:
    return overlaps( Edition::MatchRange( l.op(), l.ed() ),
                     Edition::MatchRange( r.op(), r.ed() ) );
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

  std::ostream & dumpOn( std::ostream & str, const Capability & obj )
  {
    return str << obj.detail();
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CapDetail
  //
  ///////////////////////////////////////////////////////////////////

  void CapDetail::_init()
  {
    // : _kind( NOCAP ), _lhs( id_r ), _rhs( 0 ), _flag( 0 )

    if ( !_lhs )
      return; // NOCAP

    if ( ! ISRELDEP(_lhs) )
    {
      _kind = NAMED;
      return;
    }

    ::Reldep * rd = GETRELDEP( myPool().getPool(), _lhs );
    _lhs  = rd->name;
    _rhs  = rd->evr;
    _flag = rd->flags;

    _kind = Rel::isRel( _flag ) ? VERSIONED : EXPRESSION;
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CapDetail & obj )
  {
    switch ( obj.kind() )
    {
      case CapDetail::NOCAP:
        return str << "<NoCap>";
        break;
      case CapDetail::NAMED:
        return str << obj.name();
        break;
      case CapDetail::VERSIONED:
        return str << obj.name() << " " << obj.op() << " " << obj.ed();
        break;
      case CapDetail::EXPRESSION:
        return str << obj.lhs().detail() << " " << obj.capRel() << " " << obj.rhs().detail();
        break;
    }
    return str <<  "<UnknownCap>";
  }

  std::ostream & operator<<( std::ostream & str, CapDetail::Kind obj )
  {
    switch ( obj )
    {
      case CapDetail::NOCAP:      return str << "NoCap"; break;
      case CapDetail::NAMED:      return str << "NamedCap"; break;
      case CapDetail::VERSIONED:  return str << "VersionedCap"; break;
      case CapDetail::EXPRESSION: return str << "CapExpression"; break;
    }
    return str << "UnknownCap";
  }

  std::ostream & operator<<( std::ostream & str, CapDetail::CapRel obj )
  {
    switch ( obj )
    {
      case CapDetail::REL_NONE:      return str << "NoCapRel"; break;
      case CapDetail::CAP_AND:       return str << "AND"; break;
      case CapDetail::CAP_OR:        return str << "OR"; break;
      case CapDetail::CAP_WITH:      return str << "WITH"; break;
      case CapDetail::CAP_NAMESPACE: return str << "NAMESPACE"; break;
   }
    return str << "UnknownCapRel";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
