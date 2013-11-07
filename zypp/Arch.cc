/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Arch.cc
 *
*/
#include <iostream>
#include <list>
#include <inttypes.h>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Tr1hash.h"
#include "zypp/Arch.h"
#include "zypp/Bit.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Arch::CompatEntry
  //
  /** Holds an architecture ID and it's compatible relation.
   * An architecture is compatibleWith, if it's _idBit is set in
   * _compatBits. noarch has ID 0, non builtin archs ID 1 and
   * have to be treated specialy.
  */
  struct Arch::CompatEntry
  {
    /** Bitfield for architecture IDs and compatBits relation.
     * \note Need one bit for each builtin Arch.
     * \todo Migrate to some infinite BitField
    */
    typedef bit::BitField<uint64_t> CompatBits;

    CompatEntry( const std::string & archStr_r,
                 CompatBits::IntT idBit_r = 1 )
    : _idStr( archStr_r )
    , _archStr( archStr_r )
    , _idBit( idBit_r )
    , _compatBits( idBit_r )
    {}

    CompatEntry( IdString archStr_r,
                 CompatBits::IntT idBit_r = 1 )
    : _idStr( archStr_r )
    , _archStr( archStr_r.asString() )
    , _idBit( idBit_r )
    , _compatBits( idBit_r )
    {}

    void addCompatBit( const CompatBits & idBit_r ) const
    {
      if ( idBit_r && ! (_compatBits & idBit_r) )
        {
          _compatBits |= idBit_r;
        }
    }

    /** Return whether \c this is compatible with \a targetEntry_r.*/
    bool compatibleWith( const CompatEntry & targetEntry_r ) const
    {
      switch ( _idBit.value() )
        {
        case 0:
          // this is noarch and always comatible
          return true;
          break;
        case 1:
          // this is a non builtin: self compatible only
          return _archStr == targetEntry_r._archStr;
          break;
        }
      // This is a builtin: compatible if mentioned in targetEntry_r
      return bool( targetEntry_r._compatBits & _idBit );
    }

    /** compare by score, then archStr. */
    int compare( const CompatEntry & rhs ) const
    {
      if ( _idBit.value() != rhs. _idBit.value() )
	return( _idBit.value() < rhs. _idBit.value() ? -1 : 1 );
      return _archStr.compare( rhs._archStr ); // Id 1: non builtin
    }

    bool isBuiltIn() const
    { return( _idBit != CompatBits(1) ); }

    IdString::IdType id() const
    { return _idStr.id(); }

    IdString            _idStr;
    std::string         _archStr; // frequently used by the UI so we keep a reference
    CompatBits          _idBit;
    mutable CompatBits  _compatBits;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Arch::CompatEntry Stream output */
  inline std::ostream & operator<<( std::ostream & str, const Arch::CompatEntry & obj )
  {
    Arch::CompatEntry::CompatBits bit( obj._idBit );
    unsigned bitnum = 0;
    while ( bit )
    {
      ++bitnum;
      bit >>= 1;
    }
    return str << str::form( "%-15s ", obj._archStr.c_str() ) << str::numstring(bitnum,2) << ' '
               << obj._compatBits << ' ' << obj._compatBits.value();
  }

  /** \relates Arch::CompatEntry */
  inline bool operator==( const Arch::CompatEntry & lhs, const Arch::CompatEntry & rhs )
  { return lhs._idStr == rhs._idStr; }
  /** \relates Arch::CompatEntry */
  inline bool operator!=( const Arch::CompatEntry & lhs, const Arch::CompatEntry & rhs )
  { return ! ( lhs == rhs ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

ZYPP_DEFINE_ID_HASHABLE( zypp::Arch::CompatEntry );

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  // Builtin architecture STRING VALUES to be
  // used in defCompatibleWith below!
  //
  // const IdString  _foo( "foo" );
  // const Arch Arch_foo( _foo() );
  //
  // NOTE: Builtin CLASS Arch CONSTANTS are defined below.
  //       You have to change them accordingly in Arch.h.
  //
  // NOTE: Thake care CompatBits::IntT is able to provide one
  //       bit for each architecture.
  //
  #define DEF_BUILTIN(A) \
  namespace { static inline const IdString & _##A () { static IdString __str(#A); return __str; } } \
  const Arch Arch_##A( _##A() )

  DEF_BUILTIN( noarch );

  DEF_BUILTIN( i386 );
  DEF_BUILTIN( i486 );
  DEF_BUILTIN( i586 );
  DEF_BUILTIN( i686 );
  DEF_BUILTIN( athlon );
  DEF_BUILTIN( x86_64 );

  DEF_BUILTIN( pentium3 );
  DEF_BUILTIN( pentium4 );

  DEF_BUILTIN( s390 );
  DEF_BUILTIN( s390x );

  DEF_BUILTIN( ppc );
  DEF_BUILTIN( ppc64 );
  DEF_BUILTIN( ppc64p7 );

  DEF_BUILTIN( ppc64le );

  DEF_BUILTIN( ia64 );

  DEF_BUILTIN( alphaev67 );
  DEF_BUILTIN( alphaev6 );
  DEF_BUILTIN( alphapca56 );
  DEF_BUILTIN( alphaev56 );
  DEF_BUILTIN( alphaev5 );
  DEF_BUILTIN( alpha );

  DEF_BUILTIN( sparc64v );
  DEF_BUILTIN( sparcv9v );
  DEF_BUILTIN( sparc64 );
  DEF_BUILTIN( sparcv9 );
  DEF_BUILTIN( sparcv8 );
  DEF_BUILTIN( sparc );

  DEF_BUILTIN( aarch64 );
  DEF_BUILTIN( armv7tnhl );
  DEF_BUILTIN( armv7thl );
  DEF_BUILTIN( armv7nhl );
  DEF_BUILTIN( armv7hl );
  DEF_BUILTIN( armv7l );
  DEF_BUILTIN( armv6hl );
  DEF_BUILTIN( armv6l );
  DEF_BUILTIN( armv5tejl );
  DEF_BUILTIN( armv5tel );
  DEF_BUILTIN( armv5l );
  DEF_BUILTIN( armv4tl );
  DEF_BUILTIN( armv4l );
  DEF_BUILTIN( armv3l );

  DEF_BUILTIN( sh3 );

  DEF_BUILTIN( sh4 );
  DEF_BUILTIN( sh4a );
#undef DEF_BUILTIN

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CompatSet
    //
    /** Maintain architecture compatibility (Singleton by the way it is used).
     *
     * Povides \ref Arch::CompatEntry for \ref Arch. Defines the
     * compatibleWith relation.
     * \li \c noarch has _idBit 0
     * \li \c nonbuiltin archs have _idBit 1
    */
    struct ArchCompatSet : private base::NonCopyable
    {
      typedef Arch::CompatEntry       CompatEntry;
      typedef CompatEntry::CompatBits CompatBits;

      typedef std::tr1::unordered_set<CompatEntry> Set;
      typedef Set::iterator           iterator;
      typedef Set::const_iterator     const_iterator;

      /** Singleton access. */
      static ArchCompatSet & instance()
      {
        static ArchCompatSet _instance;
        return _instance;
      }

      /** Return the entry related to \a archStr_r.
       * Creates an entry for nonbuiltin archs.
      */
      const Arch::CompatEntry & assertDef( const std::string & archStr_r )
      { return *_compatSet.insert( Arch::CompatEntry( archStr_r ) ).first; }
      /** \overload */
      const Arch::CompatEntry & assertDef( IdString archStr_r )
      { return *_compatSet.insert( Arch::CompatEntry( archStr_r ) ).first; }

      const_iterator begin() const
      { return _compatSet.begin(); }

      const_iterator end() const
      { return _compatSet.end(); }

      struct DumpOnCompare
      {
        int operator()( const CompatEntry & lhs,  const CompatEntry & rhs ) const
        { return lhs._idBit.value() < rhs._idBit.value(); }
      };

      std::ostream & dumpOn( std::ostream & str ) const
      {
        str << "ArchCompatSet:";
        std::list<CompatEntry> ov( _compatSet.begin(), _compatSet.end() );
        ov.sort( DumpOnCompare() );
        for_( it, ov.begin(), ov.end() )
          {
            str << endl << ' ' << *it;
          }
        return str;
      }

    private:
      /** Singleton ctor. */
      ArchCompatSet()
      {
        // _noarch must have _idBit 0.
        // Other builtins have 1-bit set
        // and are initialized done on the fly.
        _compatSet.insert( Arch::CompatEntry( _noarch(), 0 ) );
        ///////////////////////////////////////////////////////////////////
        // Define the CompatibleWith relation:
        //
        // NOTE: Order of definition is significant! (Arch::compare)
	//       - define compatible (less) architectures first!
        //
        defCompatibleWith( _i386(),		_noarch() );
        defCompatibleWith( _i486(),		_noarch(),_i386() );
        defCompatibleWith( _i586(),		_noarch(),_i386(),_i486() );
        defCompatibleWith( _i686(),		_noarch(),_i386(),_i486(),_i586() );
        defCompatibleWith( _athlon(),		_noarch(),_i386(),_i486(),_i586(),_i686() );
        defCompatibleWith( _x86_64(),		_noarch(),_i386(),_i486(),_i586(),_i686(),_athlon() );

        defCompatibleWith( _pentium3(),		_noarch(),_i386(),_i486(),_i586(),_i686() );
        defCompatibleWith( _pentium4(),		_noarch(),_i386(),_i486(),_i586(),_i686(),_pentium3() );

        defCompatibleWith( _ia64(),		_noarch(),_i386(),_i486(),_i586(),_i686() );
        //
        defCompatibleWith( _s390(),		_noarch() );
        defCompatibleWith( _s390x(),		_noarch(),_s390() );
        //
        defCompatibleWith( _ppc(),		_noarch() );
        defCompatibleWith( _ppc64(),		_noarch(),_ppc() );
        defCompatibleWith( _ppc64p7(),		_noarch(),_ppc(),_ppc64() );
        //
        defCompatibleWith( _ppc64le(),		_noarch() );
        //
        defCompatibleWith( _alpha(),		_noarch() );
        defCompatibleWith( _alphaev5(),		_noarch(),_alpha() );
        defCompatibleWith( _alphaev56(),	_noarch(),_alpha(),_alphaev5() );
        defCompatibleWith( _alphapca56(),	_noarch(),_alpha(),_alphaev5(),_alphaev56() );
        defCompatibleWith( _alphaev6(),		_noarch(),_alpha(),_alphaev5(),_alphaev56(),_alphapca56() );
        defCompatibleWith( _alphaev67(),	_noarch(),_alpha(),_alphaev5(),_alphaev56(),_alphapca56(),_alphaev6() );
        //
        defCompatibleWith( _sparc(),		_noarch() );
        defCompatibleWith( _sparcv8(),		_noarch(),_sparc() );
        defCompatibleWith( _sparcv9(),		_noarch(),_sparc(),_sparcv8() );
	defCompatibleWith( _sparcv9v(),		_noarch(),_sparc(),_sparcv8(),_sparcv9() );
	//
        defCompatibleWith( _sparc64(),		_noarch(),_sparc(),_sparcv8(),_sparcv9() );
	defCompatibleWith( _sparc64v(),		_noarch(),_sparc(),_sparcv8(),_sparcv9(),_sparcv9v(),_sparc64() );
        //
        defCompatibleWith( _armv3l(),		_noarch() );
        defCompatibleWith( _armv4l(),		_noarch(),_armv3l() );
        defCompatibleWith( _armv4tl(),		_noarch(),_armv3l(),_armv4l() );
        defCompatibleWith( _armv5l(),		_noarch(),_armv3l(),_armv4l(),_armv4tl() );
        defCompatibleWith( _armv5tel(),		_noarch(),_armv3l(),_armv4l(),_armv4tl(),_armv5l() );
        defCompatibleWith( _armv5tejl(),	_noarch(),_armv3l(),_armv4l(),_armv4tl(),_armv5l(),_armv5tel() );
        defCompatibleWith( _armv6l(),		_noarch(),_armv3l(),_armv4l(),_armv4tl(),_armv5l(),_armv5tel(),_armv5tejl() );
        defCompatibleWith( _armv6hl(),		_noarch() );
        defCompatibleWith( _armv7l(),		_noarch(),_armv3l(),_armv4l(),_armv4tl(),_armv5l(),_armv5tel(),_armv5tejl(),_armv6l() );
        defCompatibleWith( _armv7hl(),		_noarch(),_armv6hl() );
        defCompatibleWith( _armv7nhl(),		_noarch(),_armv7hl() );
        defCompatibleWith( _armv7thl(),		_noarch(),_armv7hl() );
        defCompatibleWith( _armv7tnhl(),	_noarch(),_armv7hl(),_armv7nhl(),_armv7thl() );
        defCompatibleWith( _aarch64(),		_noarch() );
        //
        defCompatibleWith( _sh3(),		_noarch() );
        //
        defCompatibleWith( _sh4(),		_noarch() );
        defCompatibleWith( _sh4a(),		_noarch(),_sh4() );
        //
        ///////////////////////////////////////////////////////////////////
        // dumpOn( USR ) << endl;
      }

    private:
      /** Return the next avialable _idBit.
       * Ctor injects _noarch into the _compatSet, 1 is for
       * nonbuiltin archs, so we can use <tt>size</tt> for
       * buitin archs.
      */
      CompatBits::IntT nextIdBit() const
      {
        if ( CompatBits::size == _compatSet.size() )
        {
          // Provide more bits in CompatBits::IntT
          INT << "Need more than " << CompatBits::size << " bits to encode architectures." << endl;
          ZYPP_THROW( Exception("Need more bits to encode architectures.") );
        }
        CompatBits::IntT nextBit = CompatBits::IntT(1) << (_compatSet.size());
        return nextBit;
      }

      /** Assert each builtin Arch gets an unique _idBit when
       *  inserted into the _compatSet.
      */
      const CompatEntry & assertCompatSetEntry( IdString archStr_r )
      { return *_compatSet.insert( Arch::CompatEntry( archStr_r, nextIdBit() ) ).first; }

      /** Initialize builtin Archs and set _compatBits.
      */
      void defCompatibleWith( IdString targetArch_r,
                              IdString arch0_r,
                              IdString arch1_r = IdString(),
                              IdString arch2_r = IdString(),
                              IdString arch3_r = IdString(),
                              IdString arch4_r = IdString(),
                              IdString arch5_r = IdString(),
                              IdString arch6_r = IdString(),
                              IdString arch7_r = IdString(),
                              IdString arch8_r = IdString(),
                              IdString arch9_r = IdString() )
      {
        const CompatEntry & target( assertCompatSetEntry( targetArch_r ) );
        target.addCompatBit( assertCompatSetEntry( arch0_r )._idBit );
#define _SETARG(N) if ( arch##N##_r.empty() ) return; target.addCompatBit( assertCompatSetEntry( arch##N##_r )._idBit )
        _SETARG(1); _SETARG(2); _SETARG(3); _SETARG(4);
        _SETARG(5); _SETARG(6); _SETARG(7); _SETARG(8); _SETARG(9);
#undef _SETARG
      }

    private:
      Set _compatSet;
    };

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Arch
  //
  ///////////////////////////////////////////////////////////////////

  const Arch Arch_empty ( IdString::Empty );
  // remaining Arch_* constants are defined by DEF_BUILTIN above.

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::Arch
  //	METHOD TYPE : Ctor
  //
  Arch::Arch()
  : _entry( &ArchCompatSet::instance().assertDef( _noarch() ) )
  {}

  Arch::Arch( IdString::IdType id_r )
  : _entry( &ArchCompatSet::instance().assertDef( IdString(id_r) ) )
  {}

  Arch::Arch( const IdString & idstr_r )
  : _entry( &ArchCompatSet::instance().assertDef( idstr_r ) )
  {}

  Arch::Arch( const std::string & str_r )
  : _entry( &ArchCompatSet::instance().assertDef( str_r ) )
  {}

  Arch::Arch( const char * cstr_r )
  : _entry( &ArchCompatSet::instance().assertDef( cstr_r ) )
  {}

  Arch::Arch( const CompatEntry & rhs )
  : _entry( &rhs )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::idStr
  //	METHOD TYPE : IdString
  //
  IdString Arch::idStr() const
  { return _entry->_idStr; }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::asString
  //	METHOD TYPE : const std::string &
  //
  const std::string & Arch::asString() const
  { return _entry->_archStr; }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::isBuiltIn
  //	METHOD TYPE : bool
  //
  bool Arch::isBuiltIn() const
  { return _entry->isBuiltIn(); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::compatibleWith
  //	METHOD TYPE : bool
  //
  bool Arch::compatibleWith( const Arch & targetArch_r ) const
  { return _entry->compatibleWith( *targetArch_r._entry ); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::baseArch
  //	METHOD TYPE : Arch
  //
  Arch Arch::baseArch( ) const
  {
    // check the multilib archs:
    if (Arch_x86_64.compatibleWith(*this))
    {
      return Arch_x86_64;
    }
    if (Arch_sparc64v.compatibleWith(*this))
    {
      return Arch_sparc64v;
    }
    if (Arch_sparc64.compatibleWith(*this))
    {
      return Arch_sparc64;
    }
    if (Arch_ppc64.compatibleWith(*this))
    {
      return Arch_ppc64;
    }
    if (Arch_s390x.compatibleWith(*this))
    {
      return Arch_s390x;
    }
    // Here: no multilib; return arch before noarch
    CompatSet cset( compatSet( *this ) );
    if ( cset.size() > 2 )	// systemArchitecture, ..., basearch, noarch
    {
      return *(++cset.rbegin());
    }
    return *this;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::compare
  //	METHOD TYPE : bool
  //
  int Arch::compare( const Arch & rhs ) const
  { return _entry->compare( *rhs._entry ); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::compatSet
  //	METHOD TYPE : Arch::CompatSet
  //
  Arch::CompatSet Arch::compatSet( const Arch & targetArch_r )
  {
    Arch::CompatSet ret;

    for ( ArchCompatSet::const_iterator it = ArchCompatSet::instance().begin();
          it != ArchCompatSet::instance().end(); ++it )
      {
        if ( it->compatibleWith( *targetArch_r._entry ) )
          {
            ret.insert( Arch(*it) );
          }
      }

    return ret;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
