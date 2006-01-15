#include <iostream>
#include <ctime>

#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>

#include <zypp/Range.h>
#include <zypp/Edition.h>

using namespace std;
using namespace zypp;

template<class _Compare>
  void allCompare( const Edition & lhs, const Edition & rhs,
                   _Compare compare )
  {
    MIL << "===============" << endl;
#define CMP(O) DBG << compare(lhs,rhs) << '\t' << lhs << '\t' << Rel::O << '\t' << rhs << "\t==> " << compareByRel( Rel::O, lhs, rhs, compare ) << endl
    CMP( NONE );
    CMP( ANY );
    CMP( LT );
    CMP( LE );
    CMP( EQ );
    CMP( GE );
    CMP( GT );
    CMP( NE );
#undef CMP
  }

template<class _Compare>
  void allRange( const Edition & lhs, const Edition & rhs)
  {
    typedef Range<Edition,_Compare> Range;

    MIL << "===============" << endl;
#define CMP(L,R) DBG << Rel::L<<' '<<lhs<< '\t' <<Rel::R<<' '<<rhs << '\t' << overlaps( Range(Rel::L,lhs), Range(Rel::R,rhs) ) << endl

    CMP( NONE,	NONE );
    CMP( ANY,	ANY );
    CMP( LT,	LT );
    CMP( LE ,	LE  );
    CMP( EQ,	EQ );
    CMP( GE,	GE );
    CMP( GT,	GT );
    CMP( NE,	NE );
#undef CMP
  }
/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  Edition l( "1.0" );
  Edition r( "1.0","1" );

  allRange<Edition::Compare>( l, r );
  allRange<Edition::Match>( l, r );

#if 0
  Edition::Range any;
  DBG << any << endl;

  Edition l( "1.0" );
  Edition r( "2.0" );

#define R(O,E) Edition::Range( Rel::O, E )

#define NONE(E) R(NONE,E)
#define ANY(E) R(ANY,E)
#define LT(E) R(LT,E)
#define LE(E) R(LE,E)
#define EQ(E) R(EQ,E)
#define GE(E) R(GE,E)
#define GT(E) R(GT,E)
#define NE(E) R(NE,E)

#define OV(L,R) DBG << #L << " <> " << #R << " ==> " << Edition::Range::overlaps( L, R ) << endl

  ERR << "Omitting Rel::NE" << endl;

#define OVALL( L )  \
  DBG << "----------------------------" << endl; \
  OV( L, NONE(r) ); \
  OV( L, ANY(r) );  \
  OV( L, LT(r) );   \
  OV( L, LE(r) );   \
  OV( L, EQ(r) );   \
  OV( L, GE(r) );   \
  OV( L, GT(r) );   \
  DBG << "----------------------------" << endl;

  OVALL( NONE(l) );
  OVALL( ANY(l) );
  OVALL( LT(l) );
  OVALL( LE(l) );
  OVALL( EQ(l) );
  OVALL( GE(l) );
  OVALL( GT(l) );

  // same for l > r and l == r
#endif
  INT << "===[END]============================================" << endl;
  return 0;
}
