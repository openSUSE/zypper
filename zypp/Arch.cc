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
#include <map>

#include "zypp/Arch.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace
{ /////////////////////////////////////////////////////////////////

  /** Dumb Arch compat table
   * \todo improve
   */
  struct CompatTable
  {
      static set<string> _compatTable;

      /** \return Whether \a lhs is compatible with \a rhs. */
      static bool compatible( const zypp::Arch & lhs, const zypp::Arch & rhs )
	  {
	      if ( lhs == zypp::Arch_noarch )
		  return true;

	      if ( _compatTable.empty() )
	      {
		  // initialize
#define DEF_COMPAT(L,R) _compatTable.insert( #L "|" #R )
		  DEF_COMPAT( noarch,	i386 );
		  DEF_COMPAT( noarch,	i486 );
		  DEF_COMPAT( i386,	i486 );
		  DEF_COMPAT( noarch,	i586 );
		  DEF_COMPAT( i386,	i586 );
		  DEF_COMPAT( i486,	i586 );
		  DEF_COMPAT( noarch,	i686 );
		  DEF_COMPAT( i386,	i686 );
		  DEF_COMPAT( i486,	i686 );
		  DEF_COMPAT( i586,	i686 );
		  DEF_COMPAT( noarch,	athlon );
		  DEF_COMPAT( i386,	athlon );
		  DEF_COMPAT( i486,	athlon );
		  DEF_COMPAT( i586,	athlon );
		  DEF_COMPAT( i686,	athlon );
		  DEF_COMPAT( noarch,	x86_64 );
		  DEF_COMPAT( i386,	x86_64 );
		  DEF_COMPAT( i486,	x86_64 );
		  DEF_COMPAT( i586,	x86_64 );
		  DEF_COMPAT( i686,	x86_64 );
		  DEF_COMPAT( athlon,	x86_64 );

		  DEF_COMPAT( noarch,	s390 );
		  DEF_COMPAT( noarch,	s390x );
		  DEF_COMPAT( s390,	s390x );

		  DEF_COMPAT( noarch,	ppc );
		  DEF_COMPAT( noarch,	ppc64 );
		  DEF_COMPAT( ppc,	ppc64 );

		  DEF_COMPAT( noarch,	ia64 );
#undef DEF_COMPAT
	      }

	      return _compatTable.find( lhs.asString()+"|"+rhs.asString() )
		  != _compatTable.end();
	  }
  };

    set<string> CompatTable::_compatTable;

    /////////////////////////////////////////////////////////////////
} // namespace
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
#define DEF_BUILTIN(A) const Arch Arch_##A( #A )

  DEF_BUILTIN( noarch );
  DEF_BUILTIN( src );

  DEF_BUILTIN( x86_64 );
  DEF_BUILTIN( athlon );
  DEF_BUILTIN( i686 );
  DEF_BUILTIN( i586 );
  DEF_BUILTIN( i486 );
  DEF_BUILTIN( i386 );

  DEF_BUILTIN( s390x );
  DEF_BUILTIN( s390 );

  DEF_BUILTIN( ppc64 );
  DEF_BUILTIN( ppc );

  DEF_BUILTIN( ia64 );

#undef DEF_BUILTIN

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

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::Arch
  //	METHOD TYPE : Ctor
  //
  Arch::Arch( const std::string & rhs )
  : _value( rhs )
  , _score( 0 )
  {
      static map<string,int> arch_scores;
      if (arch_scores.empty()) {
	arch_scores["noarch"] = 0;
	arch_scores["src"] = 0;
	arch_scores["i386"] = 1;
	arch_scores["i486"] = 2;
	arch_scores["i586"] = 3;
	arch_scores["i686"] = 4;
	arch_scores["athlon"] = 4;
	arch_scores["x86_64"] = 5;
	arch_scores["ia64"] = 1;
	arch_scores["ppc"] = 1;
	arch_scores["ppc64"] = 2;
	arch_scores["s390"] = 1;
	arch_scores["s390x"] = 2;
      };
      map<string,int>::const_iterator it = arch_scores.find( rhs );
      if (it != arch_scores.end())
	_score = it->second;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Arch::compatibleWith
  //	METHOD TYPE : bool
  //
  bool Arch::compatibleWith( const Arch & rhs ) const
  {
    return _value.empty()
	|| *this == rhs
	|| CompatTable::compatible( *this, rhs );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
