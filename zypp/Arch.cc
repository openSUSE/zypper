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
#include <set>

#include "zypp/base/Logger.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/Arch.h"
#include "zypp/Bit.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Arch::CompatEntry
  //
  /** Holds an architecture ID and it's compatible relation.
   * An architecture is compatibleWith, if it's _idBit is set in
   * _compatBits. noarch has ID 0, non builtin archs id 1 and
   * have to be treated specialy.
  */
  struct Arch::CompatEntry
  {
    /** Bitfield for architecture IDs and compatBits relation.
     * \note Need one bit for each builtin Arch.
    */
    typedef bit::BitField<uint16_t> CompatBits;

    CompatEntry( const std::string & archStr_r,
                 CompatBits::IntT idBit_r = CompatBits::IntT(1) )
    : _archStr( archStr_r )
    , _idBit( idBit_r )
    , _compatBits( idBit_r )
    , _compatScore( idBit_r ? 1 : 0 ) // number of compatible archs
    {}

    void addCompatBit( const CompatBits & idBit_r ) const
    {
      if ( idBit_r && ! (_compatBits & idBit_r) )
        {
          _compatBits |= idBit_r;
          ++_compatScore;
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
      // This is a builtin: comatible if mentioned in targetEntry_r
      return targetEntry_r._compatBits & _idBit;
    }

    /** compare by score, then archStr. */
    int compare( const CompatEntry & rhs ) const
    {
      if ( _compatScore != rhs._compatScore )
        return( _compatScore < rhs._compatScore ? -1 : 1 );
      return _archStr.compare( rhs._archStr );
    }

    std::string         _archStr;
    CompatBits          _idBit;
    mutable CompatBits  _compatBits;
    mutable unsigned    _compatScore;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Arch::CompatEntry Stream output */
  inline std::ostream & operator<<( std::ostream & str, const Arch::CompatEntry & obj )
  {
    return str << obj._archStr << '\t' << obj._idBit << ' '
               << obj._compatBits << ' ' << obj._compatScore;
  }

  /** \relates Arch::CompatEntry ComaptSet ordering.
   * \note This is purely based on _archStr, as required by class ComaptSet.
  */
  inline bool operator<( const Arch::CompatEntry & lhs, const Arch::CompatEntry & rhs )
  { return lhs._archStr < rhs._archStr; }

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    // builtin architecture STRING VALUES
#define DEF_BUILTIN(A) const std::string  _##A( #A )

    DEF_BUILTIN( noarch );

    DEF_BUILTIN( i386 );
    DEF_BUILTIN( i486 );
    DEF_BUILTIN( i586 );
    DEF_BUILTIN( i686 );
    DEF_BUILTIN( athlon );
    DEF_BUILTIN( x86_64 );

    DEF_BUILTIN( ia64 );

    DEF_BUILTIN( s390 );
    DEF_BUILTIN( s390x );

    DEF_BUILTIN( ppc );
    DEF_BUILTIN( ppc64 );

#undef DEF_BUILTIN

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

      typedef std::set<CompatEntry>   Set;
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
      {
        return *_compatSet.insert( Arch::CompatEntry( archStr_r )
                                 ).first;
      }

      const_iterator begin() const
      { return _compatSet.begin(); }

      const_iterator end() const
      { return _compatSet.end(); }

      std::ostream & dumpOn( std::ostream & str ) const
      {
        str << "ArchCompatSet:";
        for ( const_iterator it = _compatSet.begin(); it != _compatSet.end(); ++it )
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
        // and are intiialized done on the fly.
        _compatSet.insert( Arch::CompatEntry( _noarch, 0 ) );
        ///////////////////////////////////////////////////////////////////
        // Define the CompatibleWith relation:
        //
        defCompatibleWith( _noarch,	_i386 );

        defCompatibleWith( _noarch,	_i486 );
        defCompatibleWith( _i386,	_i486 );

        defCompatibleWith( _noarch,	_i586 );
        defCompatibleWith( _i386,	_i586 );
        defCompatibleWith( _i486,	_i586 );

        defCompatibleWith( _noarch,	_i686 );
        defCompatibleWith( _i386,	_i686 );
        defCompatibleWith( _i486,	_i686 );
        defCompatibleWith( _i586,	_i686 );

        defCompatibleWith( _noarch,	_athlon );
        defCompatibleWith( _i386,	_athlon );
        defCompatibleWith( _i486,	_athlon );
        defCompatibleWith( _i586,	_athlon );
        defCompatibleWith( _i686,	_athlon );

        defCompatibleWith( _noarch,	_x86_64 );
        defCompatibleWith( _i386,	_x86_64 );
        defCompatibleWith( _i486,	_x86_64 );
        defCompatibleWith( _i586,	_x86_64 );
        defCompatibleWith( _i686,	_x86_64 );
        defCompatibleWith( _athlon,	_x86_64 );

        /////
        defCompatibleWith( _noarch,	_ia64 );
        defCompatibleWith( _i386,	_ia64 );
        defCompatibleWith( _i486,	_ia64 );
        defCompatibleWith( _i586,	_ia64 );
        defCompatibleWith( _i686,	_ia64 );

        /////
        defCompatibleWith( _noarch,	_s390 );

        defCompatibleWith( _noarch,	_s390x );
        defCompatibleWith( _s390,	_s390x );

        /////
        defCompatibleWith( _noarch,	_ppc );

        defCompatibleWith( _noarch,	_ppc64 );
        defCompatibleWith( _ppc,	_ppc64 );
        //
        ///////////////////////////////////////////////////////////////////
      }

    private:
      /** Return the next avialable _idBit.
       * Ctor injects _noarch into the _compatSet, 1 is for
       * nonbuiltin archs, so we can use <tt>size</tt> for
       * buitin archs.
      */
      CompatBits::IntT nextIdBit() const
      {
        CompatBits::IntT nextBit = 1 << (_compatSet.size());
        assert( nextBit ); // need more bits in CompatBits::IntT
        return nextBit;
      }

      /** Assert each builtin Arch gets an unique _idBit when
       *  inserted into the _compatSet.
      */
      const CompatEntry & assertCompatSetEntry( const std::string & archStr_r )
      {
        return *_compatSet.insert( Arch::CompatEntry( archStr_r, nextIdBit() )
                                 ).first;
      }

      /** Initialize builtin Archs and set _compatBits.
      */
      void defCompatibleWith( const std::string & arch_r, const std::string & targetArch_r )
      {
        const CompatEntry & arch  ( assertCompatSetEntry( arch_r ) );
        const CompatEntry & target( assertCompatSetEntry( targetArch_r ) );
        target.addCompatBit( arch._idBit );
      }

    private:
      Set _compatSet;
    };

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////

  static const string canonical_arch (const string & arch);

  //---------------------------------------------------------------------------
  // architecture stuff

  static const string
  canonical_arch (const string & arch)
  {
    typedef struct { char *from; char *to; } canonical;
    // convert machine string to known_arch
    static canonical canonical_archs[] = {
      { "noarch",  "noarch" },
      { "unknown", "unknown" },
      { "any",     "any" },
      { "all",     "any" },
      { "i386",    "i386" },
      { "ix86",    "i386" }, /* OpenPKG uses this */
      { "i486",    "i486" },
      { "i586",    "i586" },
      { "i686",    "i686" },
      { "x86_64",  "x86_64" },
      { "ia32e",   "ia32e" },
      { "athlon",  "athlon" },
      { "ppc",     "ppc" },
      { "ppc64",   "ppc64" },
      { "s390",    "s390" },
      { "s390x",   "s390x" },
      { "ia64",    "ia64" },
      { "sparc",   "sparc" },
      { "sun4c",   "sparc" },
      { "sun4d",   "sparc" },
      { "sun4m",   "sparc" },
      { "sparc64", "sparc64" },
      { "sun4u",   "sparc64" },
      { "sparcv9", "sparc64" },
      { 0 }
    };

    for (canonical *ptr = canonical_archs; ptr->from; ptr++) {
      if (arch == ptr->from) {
        return ptr->to;
      }
    }

    return "canonical";
  }

  //---------------------------------------------------------------------------

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Arch
  //
  ///////////////////////////////////////////////////////////////////

  const Arch Arch_noarch( _noarch );

  const Arch Arch_x86_64( _x86_64 );
  const Arch Arch_athlon( _athlon );
  const Arch Arch_i686  ( _i686 );
  const Arch Arch_i586  ( _i586 );
  const Arch Arch_i486  ( _i486 );
  const Arch Arch_i386  ( _i386 );

  const Arch Arch_s390x ( _s390x );
  const Arch Arch_s390  ( _s390 );

  const Arch Arch_ppc64 ( _ppc64 );
  const Arch Arch_ppc   ( _ppc );

  const Arch Arch_ia64  ( _ia64 );

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::Arch
  //	METHOD TYPE : Ctor
  //
  Arch::Arch()
  : _entry( &ArchCompatSet::instance().assertDef( _noarch ) )
  { assert( _entry ); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::Arch
  //	METHOD TYPE : Ctor
  //
  Arch::Arch( const std::string & rhs )
  : _entry( &ArchCompatSet::instance().assertDef( rhs ) )
  { assert( _entry ); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::Arch
  //	METHOD TYPE : Ctor
  //
  Arch::Arch( const CompatEntry & rhs )
  : _entry( &rhs )
  { assert( _entry ); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::asString
  //	METHOD TYPE : const std::string &
  //
  const std::string & Arch::asString() const
  { return _entry->_archStr; }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::compatibleWith
  //	METHOD TYPE : bool
  //
  bool Arch::compatibleWith( const Arch & targetArch_r ) const
  { return _entry->compatibleWith( *targetArch_r._entry ); }

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
