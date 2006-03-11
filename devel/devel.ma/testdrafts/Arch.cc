#include <iostream>
#include <list>
#include <set>

#include "Printing.h"

#include <zypp/base/LogControl.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>

#include <zypp/Arch.h>
#include <zypp/Bit.h>
#include <zypp/RelCompare.h>

using namespace std;
using namespace zypp;
using namespace zypp::functor;

inline const char * compResult( int res )
{
  return( res ? ( res < 0 ? "<" : ">" ) : "=" );
}

template<class _Iter, class _Function>
  inline void sym_compare( _Iter begin, _Iter end, _Function fnc )
  {
    for ( _Iter l = begin; l != end; ++l )
      for ( _Iter r = begin; r != end; ++r )
        fnc( *l, *r );
  }

template<class _LIter, class _RIter, class _BinaryFunction>
  inline _BinaryFunction
  nest_for_earch( _LIter lbegin, _LIter lend,
                  _RIter rbegin, _RIter rend,
                  _BinaryFunction fnc )
  {
    for ( ; lbegin != lend; ++lbegin )
      for ( _RIter r = rbegin; r != rend; ++r )
        fnc( *lbegin, *r );
    return fnc;
  }

template<class _Iter, class _BinaryFunction>
  inline _BinaryFunction
  nest_for_earch( _Iter begin, _Iter end,
                  _BinaryFunction fnc )
  { return nest_for_earch( begin, end, begin, end, fnc ); }


template<class _Iter, class _Function>
  inline void sym_compare( _Iter begin, _Iter end )
  {
    sym_compare( begin, end, _Function() );
  }

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
  ret.push_back( Arch() );
  ret.push_back( Arch("") );
  return ret;
}

static list<Arch> archlist( archList() );
static set<Arch>  archset( archlist.begin(), archlist.end() );

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

struct OrderByCompare : public std::binary_function<Arch, Arch, bool>
{
  bool operator()( const Arch & lhs, const Arch & rhs ) const
  { return lhs.compare( rhs ) < 0; }
};


void dumpOn( const Arch::CompatSet & s )
{
  SEC << str::join( make_transform_iterator( s.begin(), std::mem_fun_ref(&Arch::asString) ),
                    make_transform_iterator( s.end(), std::mem_fun_ref(&Arch::asString) ) )
  << endl;
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

  // compare order
  typedef set<Arch,OrderByCompare> OrderedArch;

  OrderedArch a( archset.begin(), archset.end() );
  print( a );
  OrderedArch b( archset.rbegin(), archset.rend() );
  print( b );

  dumpOn( Arch::compatSet( Arch_noarch ) );
  dumpOn( Arch::compatSet( Arch_i486 ) );
  dumpOn( Arch::compatSet( Arch_x86_64 ) );
  dumpOn( Arch::compatSet( Arch("Foo") ) );

  typedef set<Arch,CompareByGT<Arch> > AS;
  Arch::CompatSet x( Arch::compatSet(Arch_x86_64) );
  AS rx( x.begin(), x.end() );
  INT << str::join( make_transform_iterator( rx.begin(), std::mem_fun_ref(&Arch::asString) ),
                    make_transform_iterator( rx.end(), std::mem_fun_ref(&Arch::asString) ) )
  << endl;


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

