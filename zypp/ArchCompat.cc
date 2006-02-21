/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ArchCompat.cc
 *
*/

#include <assert.h>
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/ArchCompat.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace arch
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////
      // builtin architecture string values
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
      /////////////////////////////////////////////////////////////////
    } //
    ///////////////////////////////////////////////////////////////////

    /** That's the real ComaptMap singleton (by the way it is used).
     * \li \c noarch has _idBit 0
     * \li \c nonbuiltin archs have _idBit 1
    */
    struct ComaptMapImpl
    {
      typedef Compat::CompatMap         CompatMap;
      typedef CompatMap::const_iterator const_iterator;
      typedef CompatMap::iterator       iterator;
      typedef Compat::Entry             CompatEntry;
      typedef CompatEntry::CompatBits   CompatBits;

      ComaptMapImpl()
      {
        // Arch_noarch must have value 0.
        // Other builtins have 1-bit set
        // and are intiialized done on the fly.
        _compatMap[_noarch] = CompatEntry( 0 );

        ///////////////////////////////////////////////////////////////////
        // Define the CompatibleWith relation:
        //
        defCompatibleWith( _i386,	_noarch );

        defCompatibleWith( _i486,	_noarch );
        defCompatibleWith( _i486,	_i386 );

        defCompatibleWith( _i586,	_noarch );
        defCompatibleWith( _i586,	_i386 );
        defCompatibleWith( _i586,	_i486 );

        defCompatibleWith( _i686,	_noarch );
        defCompatibleWith( _i686,	_i386 );
        defCompatibleWith( _i686,	_i486 );
        defCompatibleWith( _i686,	_i586 );

        defCompatibleWith( _athlon,	_noarch );
        defCompatibleWith( _athlon,	_i386 );
        defCompatibleWith( _athlon,	_i486 );
        defCompatibleWith( _athlon,	_i586 );
        defCompatibleWith( _athlon,	_i686 );

        defCompatibleWith( _x86_64,	_noarch );
        defCompatibleWith( _x86_64,	_i386 );
        defCompatibleWith( _x86_64,	_i486 );
        defCompatibleWith( _x86_64,	_i586 );
        defCompatibleWith( _x86_64,	_i686 );
        defCompatibleWith( _x86_64,	_athlon );

        /////
        defCompatibleWith( _ia64,	_noarch );
        defCompatibleWith( _ia64,	_i386 );
        defCompatibleWith( _ia64,	_i486 );
        defCompatibleWith( _ia64,	_i586 );
        defCompatibleWith( _ia64,	_i686 );

        /////
        defCompatibleWith( _s390,	_noarch );

        defCompatibleWith( _s390x,	_noarch );
        defCompatibleWith( _s390x,	_s390 );

        /////
        defCompatibleWith( _ppc,	_noarch );

        defCompatibleWith( _ppc64,	_noarch );
        defCompatibleWith( _ppc64,	_ppc );
        //
        ///////////////////////////////////////////////////////////////////
      }

      /** The CompatMap. */
      const CompatMap & compatMap() const
      { return _compatMap; }

      /** Return the CompatMap entry related to \a archStr_r.
       * Creates an entry for nonbuiltin archs.
      */
      const_iterator assertDef( const std::string & archStr_r )
      {
        return _compatMap.insert( CompatMap::value_type( archStr_r, CompatEntry( 1 ) )
                                ).first;
      }

    private:
      /** Return the next avialable _idBit.
       * Ctor injects _noarch into the _compatMap, 1 is for
       * nonbuiltin archs, so we use <tt>size</tt> for
       * buitin archs.
      */
      CompatBits::IntT nextIdBit() const
      {
        CompatBits::IntT nextBit = 1 << (_compatMap.size());
        assert( nextBit ); // need more bits in CompatBits::IntT
        return nextBit;
      }

      /** Assert each builtin Arch gets an unique _idBit when
       *  inserted into the _compatMap.
      */
      CompatEntry & assertCompatMapEntry( const std::string & arch_r )
      {
        return _compatMap.insert( CompatMap::value_type( arch_r, CompatEntry( nextIdBit() ) )
                                ).first->second;
      }

      /** Initialize builtin Archs and set _compatBits.
      */
      void defCompatibleWith( const std::string & arch_r, const std::string & compat_r )
      {
        CompatEntry & arch  ( assertCompatMapEntry( arch_r ) );
        CompatEntry & compat( assertCompatMapEntry( compat_r ) );
        arch._compatBits |= compat._idBit;
      }

    private:
      CompatMap _compatMap;
    };

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////
      /** The singleton CompatMap. */
      static ComaptMapImpl theComaptMap;
      /////////////////////////////////////////////////////////////////
    } //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Compat::instance
    //	METHOD TYPE : Compat
    //
    Compat Compat::instance()
    {
      return Compat();
    }

    Compat::const_iterator Compat::begin() const
    { return theComaptMap.compatMap().begin(); }

    Compat::const_iterator Compat::end() const
    { return theComaptMap.compatMap().end(); }

    Compat::const_iterator Compat::assertDef( const std::string & archStr_r )
    { return theComaptMap.assertDef( archStr_r );}

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Compat & obj )
    {
      str << "ArchCompatMap:";
      for ( Compat::const_iterator it = obj.begin(); it != obj.end(); ++it )
        {
          str << endl << ' ' << it->first << '\t' << it->second;
        }
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace arch
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
