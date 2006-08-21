
#include <boost/test/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/regex.hpp>

#include "zypp/parser/yum/YUMParser.h"
#include "zypp/base/Logger.h"
#include "zypp/PathInfo.h"

using namespace zypp;
using namespace zypp::parser;
using namespace zypp::parser::yum;
using namespace std;
using namespace boost;
using namespace boost::unit_test;
using namespace boost::test_tools;

typedef std::list<Pathname> PathnameList;


void parse( std::ostream &str, const std::string &type )
{
    if ( type == "repomd" )
    {
      YUMRepomdParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             str << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if ( type == "primary")
    {
      YUMPrimaryParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             str << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if ( type == "group")
    {
      YUMGroupParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             str << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if ( type == "pattern" )
    {
      YUMPatternParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             str << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if ( type == "filelist" )
    {
      YUMFileListParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             str << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if ( type == "other" )
    {
      YUMOtherParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             str << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if ( type == "patch" )
    {
      YUMPatchParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             str << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if ( type == "patches" )
    {
      YUMPatchesParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             str << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
    else if ( type == "product" )
    {
      YUMProductParser iter(cin,"");
      for (;
           !iter.atEnd();
           ++iter) {
             str << **iter;
           }
      if (iter.errorStatus())
        throw *iter.errorStatus();
    }
}

void test_blah()
{
  PathnameList files;
  readdir( files, "tests/" );
  
  boost::regex rxInput("^yum-(.+)-(.+)\\.xml$");
  
  for ( PathnameList::const_iterator it = files.begin(); it != files.end(); ++it )
  { 
    Pathname file = *it;
    //std::cout << "processing: " << file << std::endl;
    boost::smatch what;
    if(boost::regex_match( file.basename(), what, rxInput, boost::match_extra))
    {
      //std::cout << what.size() << " matches" << std::endl;
      std::cout << "processing input file: " << file << std::endl;
      try
      {
        Pathname out_filename ( std::string("tests/yum-") + std::string(what[1]) + "-" + std::string(what[2]) + std::string(".out"));
        std::cout << "output file: " << out_filename << std::endl;
        output_test_stream output( out_filename.asString(), false);
        parse( output, std::string(what[1]) );
        
        BOOST_CHECK( output.match_pattern() );
      }
      catch (XMLParserError& err) {
        BOOST_FAIL( err.msg() << " " << err.position() );
      }
      
    }
  }
}

test_suite*
init_unit_test_suite( int, char* [] )
{
  test_suite* test= BOOST_TEST_SUITE( "YUMParser" );
  test->add( BOOST_TEST_CASE( &test_blah ), 0 /* expected zero error */ );
  return test;
}
