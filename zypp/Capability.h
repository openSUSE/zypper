/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Capability.h
 *
*/
#ifndef ZYPP_SAT_CAPABILITY_H
#define ZYPP_SAT_CAPABILITY_H

#include <iosfwd>
#include <set>

#include "zypp/base/SafeBool.h"

#include "zypp/sat/detail/PoolMember.h"

#include "zypp/IdString.h"
#include "zypp/Edition.h"
#include "zypp/Rel.h"
#include "zypp/ResTraits.h"

#include "zypp/CapMatch.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Capability;
  class CapDetail;

  typedef std::set<Capability> CapabilitySet;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Capability
  //
  /** A sat capability.
   *
   * If a certain \ref ResKind is specified upon construction, the
   * capabilities name part is prefixed accordingly:
   * \code
   * Capability( "foo" )                   ==> 'foo'
   * Capability( "foo", ResKind::package ) ==> 'foo'
   * Capability( "foo", ResKind::pattern ) ==> 'pattern:foo'
   * Capability( "pattern:foo" )           ==> 'pattern:foo'
     * // avoid this:
   * Capability( "pattern:foo", ResKind::pattern ) ==> 'pattern:pattern:foo'
   * \endcode
   */
  class Capability: protected sat::detail::PoolMember,
  private base::SafeBool<Capability>
  {
    public:
      // legacy
      ZYPP_DEPRECATED std::string index() const { return std::string(); }

    public:
      enum CtorFlag { PARSED, UNPARSED };

    public:
      /** Default ctor, \ref Empty capability. */
      Capability() : _id( sat::detail::emptyId ) {}

      /** Ctor from id. */
      explicit Capability( sat::detail::IdType id_r ) : _id( id_r ) {}

      /** Ctor from string.
       * \a str_r is parsed to check whether it contains an <tt>[op edition]</tt> part,
       * unless the \ref PARSED flag is passed to the ctor.
      */
      explicit Capability( const char * str_r, const ResKind & prefix_r = ResKind(), CtorFlag flag_r = UNPARSED );
      /** \overload */
      explicit Capability( const std::string & str_r, const ResKind & prefix_r = ResKind(), CtorFlag flag_r = UNPARSED );
      /** \overload Convenience for parsed (name only) packages. */
      Capability( const char * str_r, CtorFlag flag_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const std::string & str_r, CtorFlag flag_r, const ResKind & prefix_r = ResKind() );

      /** Ctor from <tt>name op edition</tt>. */
      Capability( const std::string & name_r, const std::string & op_r, const std::string & ed_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const std::string & name_r, Rel op_r, const std::string & ed_r, const ResKind & prefix_r = ResKind() );
      /** \overload */
      Capability( const std::string & name_r, Rel op_r, const Edition & ed_r, const ResKind & prefix_r = ResKind() );

    public:
      /** No or Null \ref Capability ( Id \c 0 ). */
      static const Capability Null;

      /** Empty Capability. */
      static const Capability Empty;

    public:
      /** Evaluate in a boolean context <tt>( ! empty() )</tt>. */
      using base::SafeBool<Capability>::operator bool_type;

      /** Whether the \ref Capability is empty.
       * This is true for \ref Null and \ref Empty.
       */
      bool empty() const
      { return( _id == sat::detail::emptyId || _id == sat::detail::noId ); }

    public:
      /** Conversion to <tt>const char *</tt> */
      const char * c_str() const;

      /** Conversion to <tt>std::string</tt> */
      std::string string() const;

      /** \overload */
      std::string asString() const
      { return string(); }

    public:
      /** Helper providing more detailed information about a \ref Capability. */
      CapDetail detail() const;

    public:
      /** \name Match two simple Capabilities
       * \todo check whether we must promote string to Capability in order to match.
       */
      //@{
      static bool matches( const Capability & lhs,  const Capability & rhs )     { return _doMatch( lhs.id(), rhs.id() ); }
      static bool matches( const Capability & lhs,  const IdString & rhs )       { return _doMatch( lhs.id(), rhs.id() ); }
      static bool matches( const Capability & lhs,  const std::string & rhs )    { return _doMatch( lhs.id(), Capability(rhs).id() ); }
      static bool matches( const Capability & lhs,  const char * rhs )           { return _doMatch( lhs.id(), Capability(rhs).id() );}

      static bool matches( const IdString & lhs,    const Capability & rhs )     { return _doMatch( lhs.id(), rhs.id() ); }
      static bool matches( const IdString & lhs,    const IdString & rhs )       { return _doMatch( lhs.id(), rhs.id() ); }
      static bool matches( const IdString & lhs,    const std::string & rhs )    { return _doMatch( lhs.id(), Capability(rhs).id() ); }
      static bool matches( const IdString & lhs,    const char * rhs )           { return _doMatch( lhs.id(), Capability(rhs).id() ); }

      static bool matches( const std::string & lhs, const Capability & rhs )     { return _doMatch( Capability(lhs).id(), rhs.id() );}
      static bool matches( const std::string & lhs, const IdString & rhs )       { return _doMatch( Capability(lhs).id(), rhs.id() ); }
      static bool matches( const std::string & lhs, const std::string & rhs )    { return _doMatch( Capability(lhs).id(), Capability(rhs).id() ); }
      static bool matches( const std::string & lhs, const char * rhs )           { return _doMatch( Capability(lhs).id(), Capability(rhs).id() ); }

      static bool matches( const char * lhs,        const Capability & rhs )     { return _doMatch( Capability(lhs).id(), rhs.id() );}
      static bool matches( const char * lhs,        const IdString & rhs )       { return _doMatch( Capability(lhs).id(), rhs.id() ); }
      static bool matches( const char * lhs,        const std::string & rhs )    { return _doMatch( Capability(lhs).id(), Capability(rhs).id() ); }
      static bool matches( const char * lhs,        const char * rhs )           { return _doMatch( Capability(lhs).id(), Capability(rhs).id() ); }

      bool matches( const Capability & rhs )  const { return _doMatch( id(), rhs.id() ); }
      bool matches( const IdString & rhs )    const { return _doMatch( id(), rhs.id() ); }
      bool matches( const std::string & rhs ) const { return _doMatch( id(), Capability(rhs).id() ); }
      bool matches( const char * rhs )        const { return _doMatch( id(), Capability(rhs).id() ); }
      //@}

      /** \ref matches functor.
       */
      struct Matches: public std::binary_function<Capability,Capability,bool>
      {
        bool operator()( const Capability & lhs, const Capability & rhs ) const
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

    public:
      /** Expert backdoor. */
      sat::detail::IdType id() const
      { return _id; }
    private:
      /** Match two Capabilities */
      static bool _doMatch( sat::detail::IdType lhs,  sat::detail::IdType rhs );
    private:
      friend base::SafeBool<Capability>::operator bool_type() const;
      bool boolTest() const { return ! empty(); }
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

      /** Enum values corresponding with libsatsolver defines.
       * MPL check in PoolImpl.cc
      */
      enum CapRel
      {
        REL_NONE      = 0,
        CAP_AND       = 16,
        CAP_OR        = 17,
        CAP_WITH      = 18,
        CAP_NAMESPACE = 19
      };

    public:
      CapDetail()
      : _kind( NOCAP ), _lhs( 0 ), _rhs( 0 ), _flag( 0 )
      {}
      explicit CapDetail( const Capability & cap_r )
      : _kind( NOCAP ), _lhs( cap_r.id() ), _rhs( 0 ), _flag( 0 )
      { _init(); }
      explicit CapDetail( sat::detail::IdType id_r )
      : _kind( NOCAP ), _lhs( id_r ), _rhs( 0 ), _flag( 0 )
      { _init(); }

    public:
      Kind kind()         const { return _kind; }
      bool isNull()       const { return _kind == NOCAP; }
      bool isNamed()      const { return _kind == NAMED; }
      bool isVersioned()  const { return _kind == VERSIONED; }
      bool isSimple()     const { return _kind & (NAMED|VERSIONED); }
      bool isExpression() const { return _kind == EXPRESSION; }

      /** \name Is simple: <tt>name [op edition]</tt> */
      //@{
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
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates CapDetail Stream output */
  std::ostream & operator<<( std::ostream & str, const CapDetail & obj );

  /** \relates CapDetail Stream output */
  std::ostream & operator<<( std::ostream & str, CapDetail::Kind obj );

  /** \relates CapDetail Stream output */
  std::ostream & operator<<( std::ostream & str, CapDetail::CapRel obj );

  ///////////////////////////////////////////////////////////////////

  inline CapDetail Capability::detail() const
  { return CapDetail( _id ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_CAPABILITY_H
