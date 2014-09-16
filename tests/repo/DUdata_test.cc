#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <string>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/RepoManager.h"
#include "zypp/ResPool.h"
#include "zypp/sat/Pool.h"
#include "zypp/PoolQuery.h"

#include "KeyRingTestReceiver.h"
#include "TestSetup.h"

using boost::unit_test::test_case;

using namespace std;
using namespace zypp;
using namespace zypp::repo;
using namespace zypp::filesystem;

#define TEST_DIR TESTS_SRC_DIR "/repo/susetags/data/dudata"

PoolItem piFind( const std::string & name_r, const std::string & ver_r, bool installed_r = false )
{
  PoolQuery q;
  q.addDependency( sat::SolvAttr::name, name_r, Rel::EQ, Edition(ver_r) );
  q.setStatusFilterFlags( installed_r ? PoolQuery::INSTALLED_ONLY : PoolQuery::UNINSTALLED_ONLY );
  if ( q.size() != 1 )
    ZYPP_THROW(Exception(q.size()?"Ambiguous!":"Missing!"));
  return PoolItem( *q.begin() );
}

typedef std::pair<ByteCount,ByteCount> ByteSet;
namespace std
{
  inline std::ostream & operator<<( std::ostream & str, const ByteSet & obj )
  { return str << "<" << obj.first << "," << obj.second << ">"; }
}

inline ByteSet mkByteSet( const DiskUsageCounter::MountPointSet & mps_r )
{ return ByteSet( mps_r.begin()->commitDiff(), (++mps_r.begin())->commitDiff() ); }

inline ByteSet mkByteSet( int grow_r , int norm_r )
{ return ByteSet( ByteCount( grow_r, ByteCount::K ), ByteCount( norm_r, ByteCount::K ) ); }

inline ByteSet getSize( const DiskUsageCounter & duc_r, const PoolItem & pi_r )
{ return mkByteSet( duc_r.disk_usage( pi_r ) ); }

inline ByteSet getSize( const DiskUsageCounter & duc_r, const ResPool & pool_r )
{ return mkByteSet( duc_r.disk_usage( pool_r ) ); }

inline void XLOG( const DiskUsageCounter & duc_r, const ResPool & pool_r )
{
  for( const auto & pi : pool_r )
  {
    USR << pi << endl;
  }
  WAR << duc_r.disk_usage( pool_r ) << endl;
}

BOOST_AUTO_TEST_CASE(dudata)
{
  //KeyRingTestReceiver rec;
  // rec.answerAcceptUnknownKey(true);
  //rec.answerAcceptUnsignedFile(true);
  // rec.answerImportKey(true);

  Pathname repodir( TEST_DIR );
  TestSetup test( Arch_x86_64 );
  test.loadTargetRepo( repodir/"system" );
  test.loadRepo( repodir/"repo", "repo" );

  ResPool pool( ResPool::instance() );
  PoolItem ins( piFind( "dutest", "1.0", true ) );
  PoolItem up1( piFind( "dutest", "1.0" ) );
  PoolItem up2( piFind( "dutest", "2.0" ) );
  PoolItem up3( piFind( "dutest", "3.0" ) );

  DiskUsageCounter duc( { DiskUsageCounter::MountPoint( "/grow", DiskUsageCounter::MountPoint::Hint_growonly ),
                          DiskUsageCounter::MountPoint( "/norm" ) } );
  //XLOG( duc, pool );

  BOOST_CHECK_EQUAL( getSize( duc, ins ), mkByteSet(  5,  5 ) );
  BOOST_CHECK_EQUAL( getSize( duc, up1 ), mkByteSet( 15, 15 ) );
  BOOST_CHECK_EQUAL( getSize( duc, up2 ), mkByteSet( 45, 45 ) );
  BOOST_CHECK_EQUAL( getSize( duc, up3 ), mkByteSet(  0,  0 ) );

  // delete installed		size  5		       g   n
  ins.status().setTransact( true, ResStatus::USER );
  BOOST_CHECK_EQUAL( getSize( duc, pool ), mkByteSet(  0, -5 ) );
  ins.status().setTransact( false, ResStatus::USER );

  // install known DU		size 15		       g   n
  up1.status().setTransact( true, ResStatus::USER );
  BOOST_CHECK_EQUAL( getSize( duc, pool ), mkByteSet( 15, 15 ) );	// (multi)install (old stays)
  ins.status().setTransact( true, ResStatus::USER );
  BOOST_CHECK_EQUAL( getSize( duc, pool ), mkByteSet( 15, 10 ) );	// update (old goes)
  ins.status().setTransact( false, ResStatus::USER );
  up1.status().setTransact( false, ResStatus::USER );

  // install unknown DU		size 0/installed       g   n
  up3.status().setTransact( true, ResStatus::USER );
  BOOST_CHECK_EQUAL( getSize( duc, pool ), mkByteSet(  5,  0 ) );	// (multi)install (n could be 5 too, but satsolver does not know about multinstall)
  ins.status().setTransact( true, ResStatus::USER );
  BOOST_CHECK_EQUAL( getSize( duc, pool ), mkByteSet(  5,  0 ) );	// update (old goes)
  ins.status().setTransact( false, ResStatus::USER );
  up3.status().setTransact( false, ResStatus::USER );
}
