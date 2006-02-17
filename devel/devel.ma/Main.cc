#include <iostream>
#include <list>
#include <set>

#include "Printing.h"

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>

#include <zypp/Arch.h>

using namespace std;
using namespace zypp;
using namespace zypp::functor;

inline list<Arch> archList()
{
  list<Arch> ret;
  ret.push_back( Arch_noarch );
  ret.push_back( Arch_src );
  ret.push_back( Arch_x86_64 );
  ret.push_back( Arch_athlon );
  ret.push_back( Arch_i686 );
  ret.push_back( Arch_i586 );
  ret.push_back( Arch_i486 );
  ret.push_back( Arch_i386 );
  ret.push_back( Arch_s390x );
  ret.push_back( Arch_s390 );
  ret.push_back( Arch_ppc64 );
  ret.push_back( Arch_ppc );
  ret.push_back( Arch_ia64 );
  ret.push_back( Arch( "unknown" ) );
  ret.push_back( Arch( "unknown2" ) );
  return ret;
}

static list<Arch> archlist( archList() );
static set<Arch>  archset( archlist.begin(), archlist.end() );

inline const char * compResult( int res )
{
  return( res ? ( res < 0 ? "<" : ">" ) : "=" );
}


struct CompatTest
{
  void operator()( const Arch & lhs, const Arch & rhs ) const
  {

    DBG << str::form( "%-10s --> %-10s : %6s : %s",
                      lhs.asString().c_str(),
                      rhs.asString().c_str(),
                      ( lhs.compatibleWith( rhs ) ? "COMPAT" : "no" ),
                      compResult( lhs.compare( rhs ) ) )

        << std::endl;
  }
};

template<class _Iter, class _Function>
  inline void sym_compare( _Iter begin, _Iter end, _Function fnc )
  {
    for ( _Iter l = begin; l != end; ++l )
      for ( _Iter r = begin; r != end; ++r )
        fnc( *l, *r );
  }
template<class _Iter, class _Function>
  inline void sym_compare( _Iter begin, _Iter end )
  {
    sym_compare( begin, end, _Function() );
  }


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  // All archs in test
  print( archlist );

  // set ordering
  print( archset );

  // compatibleWith
  sym_compare( archset.begin(), archset.end(), CompatTest() );


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

