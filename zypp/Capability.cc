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

#include "zypp/Arch.h"
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
    /** Build \ref Capability from data. No parsing required.
    */
    sat::detail::IdType relFromStr( ::_Pool * pool_r,
                                    const Arch & arch_r,
                                    const std::string & name_r,
                                    Rel op_r,
                                    const Edition & ed_r,
                                    const ResKind & kind_r )
    {
      // First build the name, non-packages prefixed by kind
      sat::detail::IdType nid( sat::detail::noId );
      if ( ! kind_r || kind_r == ResKind::package )
      {
        nid = IdString( name_r ).id();
      }
      else if ( kind_r == ResKind::srcpackage )
      {
        // map 'kind srcpackage' to 'arch src', the pseudo architecture
        // satsolver uses.
        nid = IdString( name_r ).id();
        nid = ::rel2id( pool_r, nid, IdString(ARCH_SRC).id(), REL_ARCH, /*create*/true );
      }
      else
      {
        nid = IdString( str::form( "%s:%s",
                        kind_r.c_str(),
                        name_r.c_str() ) ).id();
      }


      // Extend name by architecture, if provided and not a srcpackage
      if ( ! arch_r.empty() && kind_r != ResKind::srcpackage )
      {
        nid = ::rel2id( pool_r, nid, arch_r.id(), REL_ARCH, /*create*/true );
      }

      // Extend 'op edition', if provided
      if ( op_r != Rel::ANY && ed_r != Edition::noedition )
      {
        nid = ::rel2id( pool_r, nid, ed_r.id(), op_r.bits(), /*create*/true );
      }

      return nid;
    }

   /** Build \ref Capability from data, just parsing name for '[.arch]' and detect
    * 'kind srcpackage' (will be mapped to arch \c src).
    */
    sat::detail::IdType relFromStr( ::_Pool * pool_r,
                                    const std::string & name_r, Rel op_r, const Edition & ed_r,
                                    const ResKind & kind_r )
    {
      static const Arch srcArch( IdString(ARCH_SRC).asString() );
      static const std::string srcKindPrefix( ResKind::srcpackage.asString() + ':' );

      // check for an embedded 'srcpackage:foo' to be mapped to 'foo' and 'ResKind::srcpackage'.
      if ( kind_r.empty() && str::hasPrefix( name_r, srcKindPrefix ) )
      {
        return relFromStr( pool_r, Arch_empty, name_r.substr( srcKindPrefix.size() ), op_r, ed_r, ResKind::srcpackage );
      }

      Arch arch( Arch_empty );
      std::string name( name_r );

      std::string::size_type asep( name_r.rfind( "." ) );
      if ( asep != std::string::npos )
      {
        Arch ext( name_r.substr( asep+1 ) );
        if ( ext.isBuiltIn() || ext == srcArch )
        {
          arch = ext;
          name.erase( asep );
        }
      }

      return relFromStr( pool_r, arch, name, op_r, ed_r, kind_r );
    }

    /** Full parse from string, unless Capability::PARSED.
    */
    sat::detail::IdType relFromStr( ::_Pool * pool_r,
                                    const std::string & str_r, const ResKind & kind_r,
                                    Capability::CtorFlag flag_r )
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

      return relFromStr( pool_r, name, op, ed, kind_r ); // parses for name[.arch]
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

  ///////////////////////////////////////////////////////////////////
  // Ctor from <name[.arch] op edition>.
  ///////////////////////////////////////////////////////////////////

  Capability::Capability( const std::string & name_r, const std::string & op_r, const std::string & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), name_r, Rel(op_r), Edition(ed_r), prefix_r ) )
  {}
  Capability::Capability( const std::string & name_r, Rel op_r, const std::string & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), name_r, op_r, Edition(ed_r), prefix_r ) )
  {}
  Capability::Capability( const std::string & name_r, Rel op_r, const Edition & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), name_r, op_r, ed_r, prefix_r ) )
  {}

  ///////////////////////////////////////////////////////////////////
  // Ctor from <arch name op edition>.
  ///////////////////////////////////////////////////////////////////

  Capability::Capability( const std::string & arch_r, const std::string & name_r, const std::string & op_r, const std::string & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), Arch(arch_r), name_r, Rel(op_r), Edition(ed_r), prefix_r ) )
  {}
  Capability::Capability( const std::string & arch_r, const std::string & name_r, Rel op_r, const std::string & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), Arch(arch_r), name_r, op_r, Edition(ed_r), prefix_r ) )
  {}
  Capability::Capability( const std::string & arch_r, const std::string & name_r, Rel op_r, const Edition & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), Arch(arch_r), name_r, op_r, ed_r, prefix_r ) )
  {}
  Capability::Capability( const Arch & arch_r, const std::string & name_r, const std::string & op_r, const std::string & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), arch_r, name_r, Rel(op_r), Edition(ed_r), prefix_r ) )
  {}
  Capability::Capability( const Arch & arch_r, const std::string & name_r, Rel op_r, const std::string & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), arch_r, name_r, op_r, Edition(ed_r), prefix_r ) )
  {}
  Capability::Capability( const Arch & arch_r, const std::string & name_r, Rel op_r, const Edition & ed_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), arch_r, name_r, op_r, ed_r, prefix_r ) )
  {}

  const char * Capability::c_str() const
  { return( _id ? ::dep2str( myPool().getPool(), _id ) : "" ); }

  CapMatch Capability::_doMatch( sat::detail::IdType lhs,  sat::detail::IdType rhs )
  {
#warning MIGRATE TO SAT
#warning TESTCASE
    if ( lhs == rhs )
      return CapMatch::yes;

    CapDetail l( lhs );
    CapDetail r( rhs );

    switch ( l.kind() )
    {
      case CapDetail::NOCAP:
        return( r.kind() == CapDetail::NOCAP ); // NOCAP matches NOCAP only
        break;
      case CapDetail::EXPRESSION:
        return CapMatch::irrelevant;
        break;
      case CapDetail::NAMED:
      case CapDetail::VERSIONED:
        break;
    }

    switch ( r.kind() )
    {
      case CapDetail::NOCAP:
        return CapMatch::no; // match case handled above
        break;
      case CapDetail::EXPRESSION:
        return CapMatch::irrelevant;
        break;
      case CapDetail::NAMED:
      case CapDetail::VERSIONED:
        break;
    }
    // comparing two simple caps:
    if ( l.name() != r.name() )
      return CapMatch::no;

    // if both are arch restricted they must match
    if ( l.arch() != r.arch()
         && ! ( l.arch().empty() || r.arch().empty() ) )
      return CapMatch::no;

    // isNamed matches ANY edition:
    if ( l.isNamed() || r.isNamed() )
      return CapMatch::yes;

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
    return str << obj.detail();
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
    // : _kind( NOCAP ), _lhs( id_r ), _rhs( 0 ), _flag( 0 ), _archIfSimple( 0 )

    if ( _lhs == sat::detail::emptyId || _lhs == sat::detail::noId )
      return; // NOCAP

    if ( ! ISRELDEP(_lhs) )
    {
      // this is name without arch!
      _kind = NAMED;
      return;
    }

    ::Reldep * rd = GETRELDEP( myPool().getPool(), _lhs );
    _lhs  = rd->name;
    _rhs  = rd->evr;
    _flag = rd->flags;

    if ( Rel::isRel( _flag ) )
    {
      _kind = VERSIONED;
      // Check for name.arch...
      if ( ! ISRELDEP(_lhs) )
        return; // this is name without arch!
      rd = GETRELDEP( myPool().getPool(), _lhs );
      if ( rd->flags != CAP_ARCH )
        return; // this is not name.arch
      // This is name.arch:
      _lhs = rd->name;
      _archIfSimple = rd->evr;
    }
    else if ( rd->flags == CAP_ARCH )
    {
      _kind = NAMED;
      // This is name.arch:
      _lhs = rd->name;
      _archIfSimple = rd->evr;
    }
    else
    {
      _kind = EXPRESSION;
      return;
    }
    // map back satsolvers pseudo arch 'src' to kind srcpackage
    if ( _archIfSimple == ARCH_SRC )
    {
      _lhs = IdString( (ResKind::srcpackage.asString() + ":" + IdString(_lhs).c_str()).c_str() ).id();
      _archIfSimple = 0;
    }
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CapDetail & obj )
  {
    static const char archsep = '.';
    switch ( obj.kind() )
    {
      case CapDetail::NOCAP:
        return str << "<NoCap>";
        break;
      case CapDetail::NAMED:
        str << obj.name();
        if ( obj.hasArch() )
          str << archsep << obj.arch();
        return str;
        break;
      case CapDetail::VERSIONED:
        str << obj.name();
        if ( obj.hasArch() )
          str << archsep << obj.arch();
        return str << " " << obj.op() << " " << obj.ed();
        break;
      case CapDetail::EXPRESSION:
        switch ( obj.capRel() )
        {
          case CapDetail::REL_NONE:
          case CapDetail::CAP_AND:
          case CapDetail::CAP_OR:
          case CapDetail::CAP_WITH:
          case CapDetail::CAP_ARCH:
            return str << obj.lhs().detail() << " " << obj.capRel() << " " << obj.rhs().detail();
            break;
          case CapDetail::CAP_NAMESPACE:
            return str << obj.lhs().detail() << "(" << obj.rhs().detail() << ")";
        }
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
      case CapDetail::CAP_AND:       return str << "&"; break; // AND
      case CapDetail::CAP_OR:        return str << "|"; break; // OR
      case CapDetail::CAP_WITH:      return str << "+"; break; // WITH
      case CapDetail::CAP_NAMESPACE: return str << "NAMESPACE"; break;
      case CapDetail::CAP_ARCH:      return str << "ARCH"; break;
   }
    return str << "UnknownCapRel";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
