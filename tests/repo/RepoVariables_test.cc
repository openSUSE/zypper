#include <boost/test/auto_unit_test.hpp>
#include <iostream>
#include <list>
#include <map>

#include "zypp/ZConfig.h"
#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/base/ValueTransform.h"
#include "zypp/repo/RepoVariables.h"

using std::cout;
using std::endl;
using namespace zypp;
using namespace boost::unit_test;

#define DATADIR (Pathname(TESTS_SRC_DIR) +  "/repo/yum/data")

typedef std::list<std::string> ListType;

namespace std {
  std::ostream & operator<<( std::ostream & str, const ListType & obj )
  {
    str << "[";
    for ( const auto & el : obj )
      str << " " << el;
    return str << " ]";
  }
}

// A plain functor
struct PlainTransformator
{
  std::string operator()( const std::string & value_r ) const
  { return "{"+value_r+"}"; }
};

// plain functor + required std::unary_function typedefs
struct FncTransformator : public PlainTransformator, public std::unary_function<const std::string &, std::string>
{};


BOOST_AUTO_TEST_CASE(value_transform)
{
  using zypp::base::ValueTransform;
  using zypp::base::ContainerTransform;

  typedef ValueTransform<std::string, FncTransformator> ReplacedString;
  typedef ContainerTransform<ListType, FncTransformator> ReplacedStringList;

  ReplacedString r( "val" );
  BOOST_CHECK_EQUAL( r.raw(), "val" );
  BOOST_CHECK_EQUAL( r.transformed(),	"{val}" );

  r.raw() = "new";
  BOOST_CHECK_EQUAL( r.raw(), "new" );
  BOOST_CHECK_EQUAL( r.transformed(),	"{new}" );

  ReplacedStringList rl;
  BOOST_CHECK_EQUAL( rl.empty(), true );
  BOOST_CHECK_EQUAL( rl.size(), 0 );
  BOOST_CHECK_EQUAL( rl.raw(), ListType() );
  BOOST_CHECK_EQUAL( rl.transformed(), ListType() );

  rl.raw().push_back("a");
  rl.raw().push_back("b");
  rl.raw().push_back("c");

  BOOST_CHECK_EQUAL( rl.empty(), false );
  BOOST_CHECK_EQUAL( rl.size(), 3 );
  BOOST_CHECK_EQUAL( rl.raw(), ListType({ "a","b","c" }) );
  BOOST_CHECK_EQUAL( rl.transformed(), ListType({ "{a}", "{b}", "{c}" }) );

  BOOST_CHECK_EQUAL( rl.transformed( rl.rawBegin() ), "{a}" );
}

void helperGenRepVarExpandResults()
{
  // Generate test result strings for RepVarExpand:
  //   ( STRING, REPLACED_all_vars_undef, REPLACED_all_vars_defined )
  // Crefully check whether new stings are correct before
  // adding them to the testccse.
  std::map<std::string,std::string> vartable;
  std::map<std::string,std::pair<std::string,std::string>> result;
  bool varsoff = true;

  auto varLookup = [&vartable,&varsoff]( const std::string & name_r )->const std::string *
  {
    if ( varsoff )
      return nullptr;
    std::string & val( vartable[name_r] );
    if ( val.empty() )
    { val = "["+name_r+"]"; }
    return &val;
  };

  for ( auto && value : {
    ""
    , "$"
    , "$${}"
    , "$_:"
    , "$_A:"
    , "$_A_:"
    , "$_A_B:"
    , "${_A_B}"
    , "\\${_A_B}"	// no escape on level 0
    , "${_A_B\\}"	// no close brace
    , "${C:-a$Bba}"
    , "${C:+a$Bba}"
    , "${C:+a${B}ba}"
    , "${C:+a\\$Bba}"	// escape on level > 0; no var $Bba
    , "${C:+a$Bba\\}"	// escape on level > 0; no close brace C
    , "${C:+a${B}ba}"
    , "${C:+a\\${B}ba}"	// escape on level > 0; no var ${B}
    , "${C:+a${B\\}ba}"	// escape on level > 0; no close brace B
    , "${C:+a\\${B\\}ba}"
    , "__${D:+\\$X--{${E:-==\\$X{o\\}==}  }--}__\\${B}${}__"
    , "__${D:+\\$X--{${E:-==\\$X{o\\}==}\\}--}__\\${B}${}__"
  } ) {
    varsoff = true;
    result[value].first = repo::RepoVarExpand()( value, varLookup );
    varsoff = false;
    result[value].second = repo::RepoVarExpand()( value, varLookup );
  }

  for ( const auto & el : result )
  {
#define CSTR(STR) str::form( "%-40s", str::gsub( "\""+STR+"\"", "\\", "\\\\" ).c_str() )
    cout << "RepVarExpandTest( " << CSTR(el.first) << ", " << CSTR(el.second.first) << ", " << CSTR(el.second.second) << " );" << endl;
  }
}

void RepVarExpandTest( const std::string & string_r, const std::string & allUndef_r, const std::string & allDef_r )
{
  std::map<std::string,std::string> vartable;
  bool varsoff = true;

  auto varLookup = [&vartable,&varsoff]( const std::string & name_r )->const std::string *
  {
    if ( varsoff )
      return nullptr;
    std::string & val( vartable[name_r] );
    if ( val.empty() )
    { val = "["+name_r+"]"; }
    return &val;
  };

  varsoff = true;
  BOOST_CHECK_EQUAL( repo::RepoVarExpand()( string_r, varLookup ), allUndef_r );
  varsoff = false;
  BOOST_CHECK_EQUAL( repo::RepoVarExpand()( string_r, varLookup ), allDef_r );
}

BOOST_AUTO_TEST_CASE(RepVarExpand)
{ //              ( STRING                                  , REPLACED_all_vars_undef                 , REPLACED_all_vars_defined                )
  RepVarExpandTest( ""                                      , ""                                      , ""                                       );
  RepVarExpandTest( "$"                                     , "$"                                     , "$"                                      );
  RepVarExpandTest( "$${}"                                  , "$${}"                                  , "$${}"                                   );
  RepVarExpandTest( "$_:"                                   , "$_:"                                   , "[_]:"                                   );
  RepVarExpandTest( "$_A:"                                  , "$_A:"                                  , "[_A]:"                                  );
  RepVarExpandTest( "$_A_:"                                 , "$_A_:"                                 , "[_A_]:"                                 );
  RepVarExpandTest( "$_A_B:"                                , "$_A_B:"                                , "[_A_B]:"                                );
  RepVarExpandTest( "${C:+a$Bba\\}"                         , "${C:+a$Bba\\}"                         , "${C:+a[Bba]\\}"                         );
  RepVarExpandTest( "${C:+a$Bba}"                           , ""                                      , "a[Bba]"                                 );
  RepVarExpandTest( "${C:+a${B\\}ba}"                       , "${C:+a${B\\}ba}"                       , "${C:+a${B\\}ba}"                        );
  RepVarExpandTest( "${C:+a${B}ba}"                         , ""                                      , "a[B]ba"                                 );
  RepVarExpandTest( "${C:+a\\$Bba}"                         , ""                                      , "a$Bba"                                  );
  RepVarExpandTest( "${C:+a\\${B\\}ba}"                     , ""                                      , "a${B}ba"                                );
  RepVarExpandTest( "${C:+a\\${B}ba}"                       , "ba}"                                   , "a${Bba}"                                );
  RepVarExpandTest( "${C:-a$Bba}"                           , "a$Bba"                                 , "[C]"                                    );
  RepVarExpandTest( "${_A_B\\}"                             , "${_A_B\\}"                             , "${_A_B\\}"                              );
  RepVarExpandTest( "${_A_B}"                               , "${_A_B}"                               , "[_A_B]"                                 );
  RepVarExpandTest( "\\${_A_B}"                             , "\\${_A_B}"                             , "\\[_A_B]"                               );
  RepVarExpandTest( "__${D:+\\$X--{${E:-==\\$X{o\\}==}  }--}__\\${B}${}__", "__--}__\\${B}${}__"      , "__$X--{[E]  --}__\\[B]${}__"            );
  RepVarExpandTest( "__${D:+\\$X--{${E:-==\\$X{o\\}==}\\}--}__\\${B}${}__", "____\\${B}${}__"         , "__$X--{[E]}--__\\[B]${}__"              );
}

BOOST_AUTO_TEST_CASE(replace_text)
{
  /* check RepoVariablesStringReplacer */
  ZConfig::instance().setSystemArchitecture(Arch("i686"));
  ::setenv( "ZYPP_REPO_RELEASEVER", "13.2", 1 );

  repo::RepoVariablesStringReplacer replacer1;
  BOOST_CHECK_EQUAL( replacer1(""),		"" );
  BOOST_CHECK_EQUAL( replacer1("$"),		"$" );
  BOOST_CHECK_EQUAL( replacer1("$arc"),		"$arc" );
  BOOST_CHECK_EQUAL( replacer1("$arch"),	"i686" );

  BOOST_CHECK_EQUAL( replacer1("$archit"),	"$archit" );
  BOOST_CHECK_EQUAL( replacer1("${rc}it"),	"${rc}it" );
  BOOST_CHECK_EQUAL( replacer1("$arch_it"),	"$arch_it" );

  BOOST_CHECK_EQUAL( replacer1("$arch-it"),	"i686-it" );
  BOOST_CHECK_EQUAL( replacer1("$arch it"),	"i686 it" );
  BOOST_CHECK_EQUAL( replacer1("${arch}it"),	"i686it" );

  BOOST_CHECK_EQUAL( replacer1("${arch}it$archit $arch"),	"i686it$archit i686" );
  BOOST_CHECK_EQUAL( replacer1("X${arch}it$archit $arch-it"),	"Xi686it$archit i686-it" );

  BOOST_CHECK_EQUAL( replacer1("${releasever}"),	"13.2" );
  BOOST_CHECK_EQUAL( replacer1("${releasever_major}"),	"13" );
  BOOST_CHECK_EQUAL( replacer1("${releasever_minor}"),	"2" );

  BOOST_CHECK_EQUAL(replacer1("http://foo/$arch/bar"), "http://foo/i686/bar");

  /* check RepoVariablesUrlReplacer */
  repo::RepoVariablesUrlReplacer replacer2;

//   // first of all url with {} must be accepted:
  BOOST_CHECK_NO_THROW( Url("ftp://site.org/${arch}/?arch=${arch}") );
  BOOST_CHECK_NO_THROW( Url("ftp://site.org/${arch:-noarch}/?arch=${arch:-noarch}") );
  BOOST_CHECK_NO_THROW( Url("ftp://site.org/${arch:+somearch}/?arch=${arch:+somearch}") );

  BOOST_CHECK_EQUAL(replacer2(Url("ftp://user:secret@site.org/$arch/")).asCompleteString(),
		    "ftp://user:secret@site.org/i686/");

  BOOST_CHECK_EQUAL(replacer2(Url("http://user:my$arch@site.org/$basearch/")).asCompleteString(),
		    "http://user:my$arch@site.org/i386/");

  BOOST_CHECK_EQUAL(replacer2(Url("http://site.org/update/?arch=$arch")).asCompleteString(),
		    "http://site.org/update/?arch=i686");

  BOOST_CHECK_EQUAL(replacer2(Url("http://site.org/update/$releasever/?arch=$arch")).asCompleteString(),
		    "http://site.org/update/13.2/?arch=i686");
}

// vim: set ts=2 sts=2 sw=2 ai et:
