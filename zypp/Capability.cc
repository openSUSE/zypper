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
#include <zypp/base/Logger.h>

#include <zypp/base/String.h>
#include <zypp/base/Regex.h>
#include <zypp/base/Gettext.h>
#include <zypp/base/Exception.h>

#include <zypp/Arch.h>
#include <zypp/Rel.h>
#include <zypp/Edition.h>
#include <zypp/Capability.h>

#include <zypp/sat/detail/PoolImpl.h>
#include <zypp/sat/detail/PoolMember.h>
#include <zypp/sat/Pool.h>
#include <zypp/ResPool.h>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    /// Round Robin string buffer
    template <unsigned TLen = 5>
    struct TempStrings
    {
      /** Reference to the next (cleared) tempstring. */
      std::string & getNext()
      { unsigned c = _next; _next = (_next+1) % TLen; _buf[c].clear(); return _buf[c]; }

    private:
      unsigned _next = 0;
      std::string _buf[TLen];
    };

    /** backward skip whitespace starting at pos_r */
    inline std::string::size_type backskipWs( const std::string & str_r, std::string::size_type pos_r )
    {
      for ( ; pos_r != std::string::npos; --pos_r )
      {
        char ch = str_r[pos_r];
        if ( ch != ' ' && ch != '\t' )
          break;
      }
      return pos_r;
    }

    /** backward skip non-whitespace starting at pos_r */
    inline std::string::size_type backskipNWs( const std::string & str_r, std::string::size_type pos_r )
    {
      for ( ; pos_r != std::string::npos; --pos_r )
      {
        char ch = str_r[pos_r];
        if ( ch == ' ' || ch == '\t' )
          break;
      }
      return pos_r;
    }

    /** Split any 'op edition' from str_r */
    void splitOpEdition( std::string & str_r, Rel & op_r, Edition & ed_r )
    {
      if ( str_r.empty() )
        return;
      std::string::size_type ch( str_r.size()-1 );

      // check whether the one but last word is a valid Rel:
      if ( (ch = backskipWs( str_r, ch )) != std::string::npos )
      {
        std::string::size_type ee( ch );
        if ( (ch = backskipNWs( str_r, ch )) != std::string::npos )
        {
          std::string::size_type eb( ch );
          if ( (ch = backskipWs( str_r, ch )) != std::string::npos )
          {
            std::string::size_type oe( ch );
            ch = backskipNWs( str_r, ch ); // now before 'op'? begin
            if ( op_r.parseFrom( str_r.substr( ch+1, oe-ch ) ) )
            {
              // found a legal 'op'
              ed_r = Edition( str_r.substr( eb+1, ee-eb ) );
              if ( ch != std::string::npos ) // 'op' is not at str_r begin, so skip WS
                ch = backskipWs( str_r, ch );
              str_r.erase( ch+1 );
              return;
            }
          }
        }
      }
      // HERE: Didn't find 'name op edition'
      // As a convenience we check for an embeded 'op' (not surounded by WS).
      // But just '[<=>]=?|!=' and not inside '()'.
      ch = str_r.find_last_of( "<=>)" );
      if ( ch != std::string::npos && str_r[ch] != ')' )
      {
        std::string::size_type oe( ch );

        // do edition first:
        ch = str_r.find_first_not_of( " \t", oe+1 );
        if ( ch != std::string::npos )
          ed_r = Edition( str_r.substr( ch ) );

        // now finish op:
        ch = oe-1;
        if ( str_r[oe] != '=' )	// '[<>]'
        {
          op_r = ( str_r[oe] == '<' ) ? Rel::LT : Rel::GT;
        }
        else
        { // '?='
          if ( ch != std::string::npos )
          {
            switch ( str_r[ch] )
            {
              case '<': --ch; op_r = Rel::LE; break;
              case '>': --ch; op_r = Rel::GE; break;
              case '!': --ch; op_r = Rel::NE; break;
              case '=': --ch; // fall through
              default:        op_r = Rel::EQ; break;
            }
          }
        }

        // finally name:
        if ( ch != std::string::npos ) // 'op' is not at str_r begin, so skip WS
          ch = backskipWs( str_r, ch );
        str_r.erase( ch+1 );
        return;
      }
      // HERE: It's a plain 'name'
    }

    /** Build \ref Capability from data. No parsing required.
    */
    sat::detail::IdType relFromStr( sat::detail::CPool * pool_r,
                                    const Arch & arch_r,
                                    const std::string & name_r,
                                    Rel op_r,
                                    const Edition & ed_r,
                                    const ResKind & kind_r )
    {
      // First build the name, non-packages prefixed by kind
      sat::Solvable::SplitIdent split( kind_r, name_r );
      sat::detail::IdType nid( split.ident().id() );

      if ( split.kind() == ResKind::srcpackage )
      {
        // map 'kind srcpackage' to 'arch src', the pseudo architecture
        // libsolv uses.
        nid = ::pool_rel2id( pool_r, nid, IdString(ARCH_SRC).id(), REL_ARCH, /*create*/true );
      }

      // Extend name by architecture, if provided and not a srcpackage
      if ( ! arch_r.empty() && kind_r != ResKind::srcpackage )
      {
        nid = ::pool_rel2id( pool_r, nid, arch_r.id(), REL_ARCH, /*create*/true );
      }

      // Extend 'op edition', if provided
      if ( op_r != Rel::ANY && ed_r != Edition::noedition )
      {
        nid = ::pool_rel2id( pool_r, nid, ed_r.id(), op_r.bits(), /*create*/true );
      }

      return nid;
    }

   /** Build \ref Capability from data, just parsing name for '[.arch]' and detect
    * 'kind srcpackage' (will be mapped to arch \c src).
    */
    sat::detail::IdType relFromStr( sat::detail::CPool * pool_r,
                                    const std::string & name_r, Rel op_r, const Edition & ed_r,
                                    const ResKind & kind_r )
    {
      static const Arch srcArch( IdString(ARCH_SRC).asString() );
      static const Arch nosrcArch( IdString(ARCH_NOSRC).asString() );
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
        if ( ext.isBuiltIn() || ext == srcArch || ext == nosrcArch )
        {
          arch = ext;
          name.erase( asep );
        }
      }

      return relFromStr( pool_r, arch, name, op_r, ed_r, kind_r );
    }

    /** Full parse from string, unless Capability::PARSED.
    */
    sat::detail::IdType relFromStr( sat::detail::CPool * pool_r,
                                    const Arch & arch_r, // parse from name if empty
                                    const std::string & str_r, const ResKind & kind_r,
                                    Capability::CtorFlag flag_r )
    {
      std::string name( str_r );
      Rel         op;
      Edition     ed;
      if ( flag_r == Capability::UNPARSED )
      {
        splitOpEdition( name, op, ed );
      }

      if ( arch_r.empty() )
        return relFromStr( pool_r, name, op, ed, kind_r ); // parses for name[.arch]
      // else
      return relFromStr( pool_r, arch_r, name, op, ed, kind_r );
    }


    sat::detail::IdType richOrRelFromStr( sat::detail::CPool * pool_r, const std::string & str_r, const ResKind & prefix_r, Capability::CtorFlag flag_r )
    {
      if ( str_r[0] == '(' )
        return sat::detail::PoolMember::myPool().parserpmrichdep( str_r.c_str() );
      return relFromStr( pool_r, Arch_empty, str_r, prefix_r, flag_r );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  const Capability Capability::Null( STRID_NULL );
  const Capability Capability::Empty( STRID_EMPTY );

  /////////////////////////////////////////////////////////////////

  Capability::Capability( const char * str_r, const ResKind & prefix_r, CtorFlag flag_r )
  : _id( richOrRelFromStr( myPool().getPool(), str_r, prefix_r, flag_r ) )
  {}

  Capability::Capability( const std::string & str_r, const ResKind & prefix_r, CtorFlag flag_r )
  : _id( richOrRelFromStr( myPool().getPool(), str_r, prefix_r, flag_r ) )
  {}

  Capability::Capability( const Arch & arch_r, const char * str_r, const ResKind & prefix_r, CtorFlag flag_r )
  : _id( relFromStr( myPool().getPool(), arch_r, str_r, prefix_r, flag_r ) )
  {}

  Capability::Capability( const Arch & arch_r, const std::string & str_r, const ResKind & prefix_r, CtorFlag flag_r )
  : _id( relFromStr( myPool().getPool(), arch_r, str_r, prefix_r, flag_r ) )
  {}

  Capability::Capability( const char * str_r, CtorFlag flag_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), Arch_empty, str_r, prefix_r, flag_r ) )
  {}

  Capability::Capability( const std::string & str_r, CtorFlag flag_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), Arch_empty, str_r, prefix_r, flag_r ) )
  {}

  Capability::Capability( const Arch & arch_r, const char * str_r, CtorFlag flag_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), arch_r, str_r, prefix_r, flag_r ) )
  {}

  Capability::Capability( const Arch & arch_r, const std::string & str_r, CtorFlag flag_r, const ResKind & prefix_r )
  : _id( relFromStr( myPool().getPool(), arch_r, str_r, prefix_r, flag_r ) )
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

  ///////////////////////////////////////////////////////////////////
  // Ctor creating a namespace: capability.
  ///////////////////////////////////////////////////////////////////

  Capability::Capability( ResolverNamespace namespace_r, IdString value_r )
  : _id( ::pool_rel2id( myPool().getPool(), asIdString(namespace_r).id(), (value_r.empty() ? STRID_NULL : value_r.id() ), REL_NAMESPACE, /*create*/true ) )
  {}

  ///////////////////////////////////////////////////////////////////
  // https://rpm-software-management.github.io/rpm/manual/boolean_dependencies.html
  namespace
  {
    inline const char * opstr( int op_r )
    {
      switch ( op_r )
      {
        case REL_GT:               return " > ";
        case REL_EQ:               return " = ";
        case REL_LT:               return " < ";
        case REL_GT|REL_EQ:        return " >= ";
        case REL_LT|REL_EQ:        return " <= ";
        case REL_GT|REL_LT:        return " != ";
        case REL_GT|REL_LT|REL_EQ: return " <=> ";
        case REL_AND:              return " and ";
        case REL_OR:               return " or ";
        case REL_COND:             return " if ";
        case REL_UNLESS:           return " unless ";
        case REL_ELSE:             return " else ";
        case REL_WITH:             return " with ";
        case REL_WITHOUT:          return " without ";
      }
      return "UNKNOWNCAPREL";
    }

    inline bool isBoolOp( int op_r )
    {
      switch ( op_r ) {
        case REL_AND:
        case REL_OR:
        case REL_COND:
        case REL_UNLESS:
        case REL_ELSE:
        case REL_WITH:
        case REL_WITHOUT:
          return true;
      }
      return false;
    }

    inline bool needsBrace( int op_r, int parop_r )
    {
      return ( isBoolOp( parop_r ) || parop_r == 0 ) && isBoolOp( op_r )
      && ( parop_r != op_r || op_r == REL_COND || op_r == REL_UNLESS || op_r == REL_ELSE )
      && not ( ( parop_r == REL_COND || parop_r == REL_UNLESS ) && op_r == REL_ELSE );
    }

    void cap2strHelper( std::string & outs_r, sat::detail::CPool * pool_r, sat::detail::IdType id_r, int parop_r )
    {
      if ( ISRELDEP(id_r) ) {
        ::Reldep * rd = GETRELDEP( pool_r, id_r );
        int op = rd->flags;

        if ( op == CapDetail::CAP_ARCH ) {
          if ( rd->evr == ARCH_SRC || rd->evr == ARCH_NOSRC ) {
            // map arch .{src,nosrc} to kind srcpackage
            outs_r += ResKind::srcpackage.c_str();
            outs_r += ":";
            outs_r += IdString(rd->name).c_str();
            return;
          }
          cap2strHelper( outs_r, pool_r, rd->name, op );
          outs_r += ".";
          cap2strHelper( outs_r, pool_r, rd->evr, op );
          return;
        }

        if ( op == CapDetail::CAP_NAMESPACE ) {
          cap2strHelper( outs_r, pool_r, rd->name, op );
          outs_r += "(";
          cap2strHelper( outs_r, pool_r, rd->evr, op );
          outs_r += ")";
          return;
        }

        if ( op == REL_FILECONFLICT )
        {
          cap2strHelper( outs_r, pool_r, rd->name, op );
          return;
        }

        if ( needsBrace( op, parop_r ) ) {
          outs_r += "(";
          cap2strHelper( outs_r, pool_r, rd->name, op );
          outs_r += opstr( op );
          cap2strHelper( outs_r, pool_r, rd->evr, op );
          outs_r += ")";
          return;
        }

        cap2strHelper( outs_r, pool_r, rd->name, op );
        outs_r += opstr( op );
        cap2strHelper( outs_r, pool_r, rd->evr, op );
      }
      else
        outs_r += IdString(id_r).c_str();
    }
  } // namespace
  ///////////////////////////////////////////////////////////////////

  const char * Capability::c_str() const
  {
    if ( not id() ) return "";
    if ( not ISRELDEP(id()) ) return IdString(id()).c_str();

    static TempStrings<5> tempstrs;   // Round Robin buffer to prolong the lifetime of the returned char*

    std::string & outs { tempstrs.getNext() };
    cap2strHelper( outs, myPool().getPool(), id(), 0 );
    return outs.c_str();
  }

  CapMatch Capability::_doMatch( sat::detail::IdType lhs,  sat::detail::IdType rhs )
  {
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
                 str::regex::nosubs );

    return str::regex_match( name_r, what, filenameRegex );
  }

  Capability Capability::guessPackageSpec( const std::string & str_r, bool & rewrote_r )
  {
    Capability cap( str_r );
    CapDetail detail( cap.detail() );

    // str_r might be the form "libzypp-1.2.3-4.5(.arch)'
    // correctly parsed as name capability by the ctor.
    // TODO: Think about allowing glob char in name - for now don't process
    if ( detail.isNamed() && !::strpbrk( detail.name().c_str(), "*?[{" )
      && ::strrchr( detail.name().c_str(), '-' ) && sat::WhatProvides( cap ).empty() )
    {
      Arch origArch( detail.arch() ); // to support a trailing .arch

      std::string guess( detail.name().asString() );
      std::string::size_type pos( guess.rfind( '-' ) );
      guess[pos] = '=';

      Capability guesscap( origArch, guess );
      detail = guesscap.detail();

      ResPool pool( ResPool::instance() );
      // require name part matching a pool items name (not just provides!)
      if ( pool.byIdentBegin( detail.name() ) != pool.byIdentEnd( detail.name() ) )
      {
        rewrote_r = true;
        return guesscap;
      }

      // try the one but last '-'
      if ( pos )
      {
        guess[pos] = '-';
        if ( (pos = guess.rfind( '-', pos-1 )) != std::string::npos )
        {
          guess[pos] = '=';

          guesscap = Capability( origArch, guess );
          detail = guesscap.detail();

          // require name part matching a pool items name (not just provides!)
          if ( pool.byIdentBegin( detail.name() ) != pool.byIdentEnd( detail.name() ) )
          {
            rewrote_r = true;
            return guesscap;
          }
        }
      }
    }

    rewrote_r = false;
    return cap;
  }

  Capability Capability::guessPackageSpec( const std::string & str_r )
  {
    bool dummy;
    return guessPackageSpec( str_r, dummy );
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
    return str << ( obj ? ::pool_dep2str( sat::Pool::instance().get(), obj.id() ) : "" );
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
    // map back libsolvs pseudo arch 'src' to kind srcpackage
    if ( _archIfSimple == ARCH_SRC || _archIfSimple == ARCH_NOSRC )
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
      {
        std::string outs;
        auto pool = sat::Pool::instance().get();
        auto op = obj.capRel();
        if ( obj.capRel() == CapDetail::CAP_NAMESPACE ) {
          cap2strHelper( outs, pool, obj.lhs().id(), op );
          outs += "(";
          cap2strHelper( outs, pool, obj.rhs().id(), op );
          outs += ")";
        }
        else {
          outs += "(";
          cap2strHelper( outs, pool, obj.lhs().id(), op );
          outs += opstr( op );
          cap2strHelper( outs, pool, obj.rhs().id(), op );
          outs += ")";
        }
        return str << outs;
      }
      break;
    }
    return str <<  "<UnknownCap(" << obj.lhs() << " " << obj.capRel() << " " << obj.rhs() << ")>";
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
      case CapDetail::CAP_AND:       return str << "and"; break;        // &
      case CapDetail::CAP_OR:        return str << "or"; break;         // |
      case CapDetail::CAP_COND:      return str << "if"; break;
      case CapDetail::CAP_UNLESS:    return str << "unless"; break;
      case CapDetail::CAP_ELSE:      return str << "else"; break;
      case CapDetail::CAP_WITH:      return str << "with"; break;       // +
      case CapDetail::CAP_WITHOUT:   return str << "without"; break;    // -
      case CapDetail::CAP_NAMESPACE: return str << "NAMESPACE"; break;
      case CapDetail::CAP_ARCH:      return str << "ARCH"; break;
   }
   return str << "UnknownCapRel("+str::numstring(obj)+")";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
