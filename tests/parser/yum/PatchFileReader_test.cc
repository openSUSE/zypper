#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/base/Easy.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"
#include "zypp/Pathname.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/parser/yum/PatchFileReader.h"

using namespace std;
using namespace boost::unit_test;
using namespace zypp;
using namespace zypp::parser::yum;

struct Check_fetchmsttfonts
{
  bool operator()( const data::Patch_Ptr & data_r )
  {
    BOOST_CHECK( data_r );
    if ( data_r )
    {
      BOOST_CHECK_EQUAL( data_r->name, "fetchmsttfonts.sh" );
      BOOST_CHECK_EQUAL( data_r->edition, Edition("4347", "0") );
      BOOST_CHECK_EQUAL( data_r->arch, Arch_noarch );

      BOOST_CHECK_EQUAL( data_r->summary.text().size(), 41 );
      BOOST_CHECK_EQUAL( data_r->description.text().size(), 296 );
      BOOST_CHECK_EQUAL( data_r->licenseToConfirm.text().size(), 4911 );

      BOOST_CHECK_EQUAL( data_r->id, "fetchmsttfonts.sh-4347" );
      BOOST_CHECK_EQUAL( data_r->timestamp, Date(20070919) );
      BOOST_CHECK_EQUAL( data_r->category, "optional" );

      BOOST_CHECK_EQUAL( data_r->atoms.size(), 1 );
      if ( data_r->atoms.size() == 1 )
      {
        data::Script_Ptr scr( dynamic_pointer_cast<data::Script>( *data_r->atoms.begin() ) );
        if ( scr )
        {
          BOOST_CHECK_EQUAL( scr->name, "fetchmsttfonts.sh-4347-patch-fetchmsttfonts.sh-2" );
          BOOST_CHECK_EQUAL( scr->edition, Edition("4347", "1") );
          BOOST_CHECK_EQUAL( scr->arch, Arch_noarch );

          BOOST_CHECK_EQUAL( scr->doScript.size(), 3718);
          BOOST_CHECK_EQUAL( scr->undoScript.size(), 0 );
        }
        else
        {
          BOOST_CHECK_MESSAGE( false, "Atom is not a Script" );
        }
      }
    }
    return true;
  }
};

void patch_read_test( const Pathname & file_r )
{
  cout << "Testing : " << file_r << endl;
  PatchFileReader( file_r, Check_fetchmsttfonts() );
}

test_suite * init_unit_test_suite( int argc, char * argv[] )
{
  Pathname patchfile;
  --argc; ++argv;
  if ( argc )
  {
    patchfile = argv[0];
    if ( PathInfo( patchfile ).isDir() )
      patchfile /= "patch-fetchmsttfonts.sh-4347.xml";
  }
  else
  {
    patchfile = TESTS_SRC_DIR;
    patchfile /= "parser/yum/data/patch-fetchmsttfonts.sh-4347.xml";
  }

  test_suite * test = BOOST_TEST_SUITE( "PatchFileReader" );
  test->add( BOOST_PARAM_TEST_CASE( &patch_read_test, &patchfile, &patchfile+1 ) );

  return test;
}
