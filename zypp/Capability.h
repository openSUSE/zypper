/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Capability.h
 *
*/
#ifndef ZYPP_CAPABILITY_H
#define ZYPP_CAPABILITY_H

#include <iosfwd>

#include "zypp/APIConfig.h"
#include "zypp/sat/detail/PoolMember.h"

#include "zypp/IdString.h"
#include "zypp/Edition.h"
#include "zypp/Rel.h"
#include "zypp/ResTraits.h"
#include "zypp/ResolverNamespace.h"
#include "zypp/CapMatch.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Capability;
  class CapDetail;
  class Arch;

  typedef std::tr1::unordered_set<Capability> CapabilitySet;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Capability
  //
  /** A sat capability.
   *
   * A Capability: <tt>"name[.arch] [op edition]"</tt>
   *
   * If a certain \ref ResKind is specified upon construction, the
   * capabilities name part is prefixed, unless it already conatins a
   * well known kind spec. If no \ref ResKind is specified, it's assumed
   * you refer to a package or the name is already prefixed:
   * \code
   * Capability( "foo" )                   ==> 'foo'
   * Capability( "foo", ResKind::package ) ==> 'foo'
   * Capability( "foo", ResKind::pattern ) ==> 'pattern:foo'
   * Capability( "pattern:foo" )           ==> 'pattern:foo'
   * // in doubt an explicit name prefix wins:
   * Capability( "pattern:foo", ResKind::package ) ==> 'pattern:foo'
   * Capability( "package:foo", ResKind::pattern ) ==> 'foo'
   * \endcode
   */
  class Capability: protected sat::detail::PoolMember
  {
    public:
      enum CtorFlag { PARSED, UNPARSED };

    public:
      /** Default ctor, \ref Empty capability. */
      Capability() : _id( sat::detail::emptyId ) {}

      /** Ctor from id. */
      explicit Capability( sat::detail::IdType id_r ) : _id( id_r ) {}

      /** \name Ctors parsing a Capability: <tt>"name[.arch] [op edition]"</tt> or <tt>( arch, "name [op edition]")</tt>
      */
      //@{
      /** Ctor from string.
       * \a str_r is parsed to check whether it contains an <tt>[op edition]</tt> part,
       * unless the \ref PARSED flag is passed to the ctor. In that case <tt>"name[.arch]"</tt>
       * is assumed.
      */
      explicit Capability( const char * str_r, const ResKind & prefix_r = ResKind(), CtorFlag flag_r = UNPARSED );
      /** \overload */
      explicit Capability( const std::string & str_r, const ResKind & prefix_r = ResKind(), CtorFlag flag_r = UNPARSED );
      /** \overload Explicitly specify the \c arch. */
      Capability( const Arch & arch_r, const char * str_r, const ResKind & prefix_r = ResKind(), CtorFlag flag_r = UNPARSED );
      /** \overload Explicitly specify the \c arch. */
      Capability( const Arch & arch_r, const std::string & str_r, const ResKind & prefix_r = ResKind(), CtorFlag flag_r = UNPARSED );

      /** \overload Convenience for parsed (name only, no <tt>"[op edition]</tt>) packages: <tt>Capability( "glibc", PARSED ); */
      Capability( const char * str_r, CtorFlag flag_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const std::string & str_r, CtorFlag flag_r, const ResKind & prefix_r = ResKind() );
      /** \overload Explicitly specify the \c arch. */
      Capability( const Arch & arch_r, const char * str_r, CtorFlag flag_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const Arch & arch_r, const std::string & str_r, CtorFlag flag_r, const ResKind & prefix_r = ResKind() );
      //@}


      /** \name Ctors parsing a broken down Capability: <tt>( "name[.arch]", op, edition )</tt>
      */
      //@{
      /** Ctor from <tt>name[.arch] op edition</tt>. */
      Capability( const std::string & name_r, const std::string & op_r, const std::string & ed_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const std::string & name_r, Rel op_r, const std::string & ed_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const std::string & name_r, Rel op_r, const Edition & ed_r, const ResKind & prefix_r = ResKind() );
      //@}

      /** \name Ctors taking a broken down Capability: <tt>( arch, name, op, edition )</tt>
      */
      //@{
      /** Ctor from <tt>arch name op edition</tt>. */
      Capability( const std::string & arch_r, const std::string & name_r, const std::string & op_r, const std::string & ed_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const std::string & arch_r, const std::string & name_r, Rel op_r, const std::string & ed_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const std::string & arch_r, const std::string & name_r, Rel op_r, const Edition & ed_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const Arch & arch_r, const std::string & name_r, const std::string & op_r, const std::string & ed_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const Arch & arch_r, const std::string & name_r, Rel op_r, const std::string & ed_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const Arch & arch_r, const std::string & name_r, Rel op_r, const Edition & ed_r, const ResKind & prefix_r = ResKind() );
      //@}

      /** \name Ctor creating a namespace: capability.
       * An empty \a value_r (std::string or IdString) will also be mapped to IdString::Null,
       * creating a namespace: capability which in most contexts matches all members of this namespace.
       */
      //@{
      Capability( ResolverNamespace namespace_r, IdString value_r = IdString::Null );
      Capability( ResolverNamespace namespace_r, const char * value_r )		: Capability( namespace_r, IdString(value_r) ) {}
      Capability( ResolverNamespace namespace_r, const std::string & value_r )	: Capability( namespace_r, IdString(value_r) ) {}
      //@}
    public:
      /** No or Null \ref Capability ( Id \c 0 ). */
      static const Capability Null;

      /** Empty Capability. */
      static const Capability Empty;

    public:
      /** Evaluate in a boolean context <tt>( ! empty() )</tt>. */
      explicit operator bool() const
      { return ! empty(); }

      /** Whether the \ref Capability is empty.
       * This is true for \ref Null and \ref Empty.
       */
      bool empty() const
      { return( _id == sat::detail::emptyId || _id == sat::detail::noId ); }

    public:
      /** Conversion to <tt>const char *</tt> */
      const char * c_str() const;

      /** \overload */
      std::string asString() const
      { return c_str(); }

    public:
      /** Helper providing more detailed information about a \ref Capability. */
      CapDetail detail() const;

    public:
      /** \name Match two simple capabilities.
       *
       * Two simple capabilities match if they have the same \c name
       * and their \c edition ranges overlap. Where no edition matches
       * ANY edition. \see \ref Edition::match.
       *
       * If a capability expression is involved, \ref matches returns
       * \ref CapMatch::irrelevant.
       */
      //@{
      static CapMatch matches( const Capability & lhs,  const Capability & rhs )     { return _doMatch( lhs.id(), rhs.id() ); }
      static CapMatch matches( const Capability & lhs,  const IdString & rhs )       { return _doMatch( lhs.id(), rhs.id() ); }
      static CapMatch matches( const Capability & lhs,  const std::string & rhs )    { return _doMatch( lhs.id(), Capability(rhs).id() ); }
      static CapMatch matches( const Capability & lhs,  const char * rhs )           { return _doMatch( lhs.id(), Capability(rhs).id() );}

      static CapMatch matches( const IdString & lhs,    const Capability & rhs )     { return _doMatch( lhs.id(), rhs.id() ); }
      static CapMatch matches( const IdString & lhs,    const IdString & rhs )       { return _doMatch( lhs.id(), rhs.id() ); }
      static CapMatch matches( const IdString & lhs,    const std::string & rhs )    { return _doMatch( lhs.id(), Capability(rhs).id() ); }
      static CapMatch matches( const IdString & lhs,    const char * rhs )           { return _doMatch( lhs.id(), Capability(rhs).id() ); }

      static CapMatch matches( const std::string & lhs, const Capability & rhs )     { return _doMatch( Capability(lhs).id(), rhs.id() );}
      static CapMatch matches( const std::string & lhs, const IdString & rhs )       { return _doMatch( Capability(lhs).id(), rhs.id() ); }
      static CapMatch matches( const std::string & lhs, const std::string & rhs )    { return _doMatch( Capability(lhs).id(), Capability(rhs).id() ); }
      static CapMatch matches( const std::string & lhs, const char * rhs )           { return _doMatch( Capability(lhs).id(), Capability(rhs).id() ); }

      static CapMatch matches( const char * lhs,        const Capability & rhs )     { return _doMatch( Capability(lhs).id(), rhs.id() );}
      static CapMatch matches( const char * lhs,        const IdString & rhs )       { return _doMatch( Capability(lhs).id(), rhs.id() ); }
      static CapMatch matches( const char * lhs,        const std::string & rhs )    { return _doMatch( Capability(lhs).id(), Capability(rhs).id() ); }
      static CapMatch matches( const char * lhs,        const char * rhs )           { return _doMatch( Capability(lhs).id(), Capability(rhs).id() ); }

      CapMatch matches( const Capability & rhs )  const { return _doMatch( id(), rhs.id() ); }
      CapMatch matches( const IdString & rhs )    const { return _doMatch( id(), rhs.id() ); }
      CapMatch matches( const std::string & rhs ) const { return _doMatch( id(), Capability(rhs).id() ); }
      CapMatch matches( const char * rhs )        const { return _doMatch( id(), Capability(rhs).id() ); }
      //@}

      /** \ref matches functor.
       */
      struct Matches: public std::binary_function<Capability,Capability,CapMatch>
      {
        CapMatch operator()( const Capability & lhs, const Capability & rhs ) const
        { return Capability::matches( lhs, rhs ); }
      };

    public:
      /** Test for a filename that is likely being REQUIRED.
       * Files below \c /bin , \c /sbin ,  \c /lib etc. Scanning a
       * packages filelist, an \e interesting filename might be worth
       * being remembered in PROVIDES.
       */
      static bool isInterestingFileSpec( const IdString & name_r )    { return isInterestingFileSpec( name_r.c_str() ); }
      static bool isInterestingFileSpec( const std::string & name_r ) { return isInterestingFileSpec( name_r.c_str() ); }
      static bool isInterestingFileSpec( const char * name_r );

      /** \ref Capability parser also guessing \c "libzypp-1.2.3-4.5.x86_64" formats.
       *
       * The argument might be in the form \c "libzypp-devel-1.2.3.x86_64".
       * Passed to the Capability ctor, this would correctly be parsed as name
       * capability, because actually the edition part had to be separated by a
       * \c '=', and the architecture had to be appended to the name.
       * So this is how it actually had to look like: \c "libzypp-devel.x86_64=1.2.3"
       *
       * Obviously we have to guess if, and where to split name and edition. In
       * fact \c "devel" could also be the version and \c "1.2.3" would be the
       * release then.
       *
       * Assuming this Capability should be provided by some package in
       * the \ref ResPool, we check this. If unprovided, we substitute the last,
       * (or one but last) \c '-' by a \c '='. If the name part (without version)
       * of the resulting Capability matches a package name (not provides!) in
       * the \ref ResPool, this Capability is returned.
       *
       * Otherwise we return the Capability originally created from
       * \a str_r.
       *
       * \note: As this method will access the global pool, the returned
       * result depends on the pools content.
       */
      static Capability guessPackageSpec( const std::string & str_r );
      /** \overload Taking an additional bool indicating whether \c str_r made
       * a valid \ref Capability (\c true) or the result was was guessed by
       * rewiting a \c '-' to \c '='. (\c false).
       */
      static Capability guessPackageSpec( const std::string & str_r, bool & rewrote_r );

    public:
      /** Expert backdoor. */
      sat::detail::IdType id() const
      { return _id; }
    private:
      /** Match two Capabilities */
      static CapMatch _doMatch( sat::detail::IdType lhs,  sat::detail::IdType rhs );
    private:
      sat::detail::IdType _id;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Capability Stream output */
  std::ostream & operator<<( std::ostream & str, const Capability & obj );

  /** \relates Capability Detailed stream output */
  std::ostream & dumpOn( std::ostream & str, const Capability & obj );

  /** \relates Capability */
  inline bool operator==( const Capability & lhs, const Capability & rhs )
  { return lhs.id() == rhs.id(); }

  /** \relates Capability */
  inline bool operator!=( const Capability & lhs, const Capability & rhs )
  { return lhs.id() != rhs.id(); }

  /** \relates Capability Arbitrary order. */
  inline bool operator<( const Capability & lhs, const Capability & rhs )
  { return lhs.id() < rhs.id(); }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CapDetail
  //
  /** Helper providing more detailed information about a \ref Capability.
   *
   * Capabilities are classified to be either \c SIMPLE:
   * \code
   *   name[.arch] [op edition]
   *   with op := <|<=|=|>=|>|!=
   * \endcode
   * or formed by some \c EXPRESSION:
   * \code
   *   left_cap op right_cap
   *   with op := AND|OR|WITH|NAMESPACE
   * \endcode
   */
  class CapDetail: protected sat::detail::PoolMember
  {
    public:
      enum Kind
      {
        NOCAP      = 0x00,
        NAMED      = 0x01,
        VERSIONED  = 0x02,
        EXPRESSION = 0x04
      };

      /** Enum values corresponding with libsolv defines.
       * \note MPL check in PoolImpl.cc
      */
      enum CapRel
      {
        REL_NONE      = 0,
        CAP_AND       = 16,
        CAP_OR        = 17,
        CAP_WITH      = 18,
        CAP_NAMESPACE = 19,
        CAP_ARCH      = 20
      };

    public:
      CapDetail()
      : _kind( NOCAP ), _lhs( 0 ), _rhs( 0 ), _flag( 0 ), _archIfSimple( 0 )
      {}
      explicit CapDetail( const Capability & cap_r )
      : _kind( NOCAP ), _lhs( cap_r.id() ), _rhs( 0 ), _flag( 0 ), _archIfSimple( 0 )
      { _init(); }
      explicit CapDetail( sat::detail::IdType id_r )
      : _kind( NOCAP ), _lhs( id_r ), _rhs( 0 ), _flag( 0 ), _archIfSimple( 0 )
      { _init(); }

    public:
      Kind kind()         const { return _kind; }
      bool isNull()       const { return _kind == NOCAP; }
      bool isNamed()      const { return _kind == NAMED; }
      bool isVersioned()  const { return _kind == VERSIONED; }
      bool isSimple()     const { return _kind & (NAMED|VERSIONED); }
      bool isExpression() const { return _kind == EXPRESSION; }

      /** \name Is simple: <tt>name[.arch] [op edition]</tt> */
      //@{
      bool     hasArch()  const { return _archIfSimple; }
      IdString arch()     const { return _archIfSimple ? IdString( _archIfSimple ) : IdString(); }
      IdString name()     const { return isSimple()    ? IdString( _lhs ) : IdString(); }
      Rel      op()       const { return isVersioned() ? Rel( _flag )     : Rel::ANY; }
      Edition  ed()       const { return isVersioned() ? Edition( _rhs )  : Edition(); }
      //@}

      /** \name Is expression <tt>cap op cap</tt> */
      //@{
      Capability lhs()    const { return isExpression() ? Capability( _lhs ) : Capability::Null; }
      CapRel     capRel() const { return isExpression() ? CapRel(_flag)      : REL_NONE; }
      Capability rhs()    const { return isExpression() ? Capability( _rhs ) : Capability::Null; }
     //@}

    private:
      void _init();
    private:
      Kind                _kind;
      sat::detail::IdType _lhs;
      sat::detail::IdType _rhs;
      unsigned            _flag;
      sat::detail::IdType _archIfSimple;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates CapDetail Stream output */
  std::ostream & operator<<( std::ostream & str, const CapDetail & obj );

  /** \relates CapDetail Stream output */
  std::ostream & operator<<( std::ostream & str, CapDetail::Kind obj );

  /** \relates CapDetail Stream output */
  std::ostream & operator<<( std::ostream & str, CapDetail::CapRel obj );

  ///////////////////////////////////////////////////////////////////

  inline CapDetail Capability::detail() const { return CapDetail( _id ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

ZYPP_DEFINE_ID_HASHABLE( ::zypp::Capability );

#endif // ZYPP_CAPABILITY_H
