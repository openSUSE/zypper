#include "TestSetup.h"

#include "utils/flags/zyppflags.h"
#include "utils/flags/flagtypes.h"
#include <boost/optional.hpp>

using namespace zypp;
using namespace ZyppFlags;

TestSetup test;	// prepare zypper and assert we have an OutputWriter

std::ostream& operator<<(std::ostream& ostr, std::vector<int> const& r) {
  ostr << "{";
  for ( int i : r ) {
    ostr << " " << i;
  }
  ostr << "}";
  return ostr;
}

namespace boost { namespace test_tools { namespace tt_detail {
template<>
struct print_log_value< std::vector<int> > {
void operator()( std::ostream& os,
    std::vector<int> const& ts)
{
    ::operator<<(os,ts);
}
};
}}}

BOOST_AUTO_TEST_CASE( simpleOptions )
{
  bool noArgOption;
  int  requiredArgOption;
  int  optionalArgOption;
  std::vector<int> containerArg;

  auto resetVals = [ & ] () {
    noArgOption       = false;
    requiredArgOption = 0;
    optionalArgOption = 0;
    containerArg.clear();
  };

  CommandGroup grp {{{
    { "noArg", 'a', NoArgument, BoolCompatibleType ( noArgOption, StoreTrue, boost::optional<bool>( noArgOption ) ), "Help text 1" },
    { "reqArg", 'b', RequiredArgument, IntType( &requiredArgOption ), "Help text 2" },
    { "optionalArg", 'c', OptionalArgument, IntType( &optionalArgOption, 22 )},
    { "repeatableArg", 'd', RequiredArgument | Repeatable, GenericContainerType( containerArg ) }
  }}};

  {
    //test basic functionalityoptionalArg
    resetVals();
    const char *testArgs[] {
      "command",
      "--noArg",
      "--reqArg", "10",
      "--optionalArg",
      "--repeatableArg", "1",
      "--repeatableArg", "2",
      "--repeatableArg", "3"
    };

    std::vector<int> expectedSet{ 1, 2, 3};

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *), ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( noArgOption, true );
    BOOST_CHECK_EQUAL ( requiredArgOption, 10 );
    BOOST_CHECK_EQUAL ( optionalArgOption, 22 );
    BOOST_CHECK_EQUAL ( containerArg, expectedSet );

  }

  {
    //write optional value explictely
    resetVals();
    const char *testArgs[] {
      "command",
      "--optionalArg=42"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( noArgOption, false );
    BOOST_CHECK_EQUAL ( requiredArgOption, 0 );
    BOOST_CHECK_EQUAL ( optionalArgOption, 42 );
  }

  {
    //write optional value explictely , short option
    resetVals();
    const char *testArgs[] {
      "command",
      "-c42"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( noArgOption, false );
    BOOST_CHECK_EQUAL ( requiredArgOption, 0 );
    BOOST_CHECK_EQUAL ( optionalArgOption, 42 );
  }

  {
    //missing argument for a flag that requires arguments
    resetVals();
    const char *testArgs[] {
      "command",
      "--reqArg"
    };

    BOOST_REQUIRE_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ), MissingArgumentException );
  }

  {
    //uknown long flag
    resetVals();
    const char *testArgs[] {
      "command",
      "--unknownFlag"
    };

    BOOST_REQUIRE_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ), UnknownFlagException );
  }

  {
    //uknown short flag
    resetVals();
    const char *testArgs[] {
      "command",
      "-z"
    };

    BOOST_REQUIRE_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char* const* )testArgs, { grp } ), UnknownFlagException );
  }

#if 0 // Test disabled due to bsc#1123865: don't throw, just warn
  {
    //flag repeated, but is not repeatable
    resetVals();
    const char *testArgs[] {
      "command",
       "--reqArg", "10",
       "--reqArg", "10"
    };

    BOOST_REQUIRE_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char* const* )testArgs, { grp } ), FlagRepeatedException );
  }
#endif
}

BOOST_AUTO_TEST_CASE( hooks )
{

  std::vector<int> values;
  std::vector<int> expectedResult { 1, 2, 3};
  std::string valInPreHook;
  std::string valInCB;
  std::string valInPostHook;
  std::vector<int> notSeenInvoked;
  std::vector<int> notSeenExcpected { 1, 2 };

  CommandGroup grp {{{
        { "arg", 'a', RequiredArgument,
              std::move(CallbackVal( [&] ( const CommandOption &, const boost::optional<std::string> &in) {
                values.push_back( 2);
                valInCB = *in;
              }).before( [&]( const CommandOption &, const boost::optional<std::string> &in ) {
                  values.push_back( 1 );
                  valInPreHook = *in;
                  return true;
                }).after( [&] ( const CommandOption &, const boost::optional<std::string> &in ) {
                    values.push_back( 3 );
                    valInPostHook = *in;
                  }))
    }, {
      "neverUsed", 'b', NoArgument, std::move( NoValue().notSeen( [ &notSeenInvoked ](){
          notSeenInvoked.push_back( 1 );
      } ))
    }, std::move( CommandOption (
        "neverUsed2", 'c', NoArgument, std::move( NoValue().notSeen( [ &notSeenInvoked ](){
          notSeenInvoked.push_back( 2 );
        } ))
      ).setDependencies( { "neverUsed" } )
    )
  }}};

  {
    const char *testArgs[] {
      "command",
       "--arg", "test"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( values, expectedResult );
    BOOST_CHECK_EQUAL ( valInPreHook, "test" );
    BOOST_CHECK_EQUAL ( valInCB, "test" );
    BOOST_CHECK_EQUAL ( valInPostHook, "test" );
    BOOST_CHECK_EQUAL ( notSeenInvoked, notSeenExcpected );
  }

  //notSeen must be invoked in the right order even if there are no options given
  {
    notSeenInvoked.clear();
    const char *testArgs[] {
      "command"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( notSeenInvoked, notSeenExcpected );
  }
}

BOOST_AUTO_TEST_CASE( priority )
{
  std::vector <int> values;
  CommandGroup grp {
    {
      std::move( CommandOption (
         "arg1", 'a', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 1 );
          })
      ).setPriority( 3 ) ),
      std::move( CommandOption (
         "arg2", 'b', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 2 );
          })
      ).setPriority( 2 ) ),
      std::move( CommandOption (
         "arg3", 'c', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 3 );
          })
      ).setPriority( 1 ) ),
      std::move( CommandOption (
         "arg4", 'd', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 4 );
          })
      ).setPriority( -10 ) )
    }
  };

  {
    std::vector <int> expected { 1, 2, 3, 4 };
    values.clear();

    const char *testArgs[] {
      "command",
       "--arg3", "--arg4", "--arg1", "--arg2"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( values, expected );
  }

  {
    std::vector <int> expected { 2, 3 };
    values.clear();

    const char *testArgs[] {
      "command",
       "--arg3", "--arg2"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( values, expected );
  }
}

BOOST_AUTO_TEST_CASE( dependencies )
{
  /*

    +-------arg8---->arg9-+
   /                      |
  v                       |
 arg7-+                   |
  \   |                   |
  v   |                   |
 arg6 |                   |
   \  |                   |
   v  v                   |
   arg4  arg5             |
       \/                 |
       v                  v
      arg1       arg2   arg3
  */
  std::vector <int> values;
  CommandGroup grp {
    {
      std::move( CommandOption (
         "arg1", 'a', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 1 );
          })
      ).setPriority( 3 ) ),
      std::move( CommandOption (
         "arg2", 'b', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 2 );
          })
      ).setPriority( 1 ) ),
      std::move( CommandOption (
         "arg3", 'c', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 3 );
          })
      ).setPriority( 2 ) ),
      std::move( CommandOption (
         "arg4", 'd', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 4 );
          })
      ).setDependencies( { "arg1" } ) ),
      std::move( CommandOption (
         "arg5", 'e', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 5 );
          })
      ).setDependencies( { "arg1" } ) ),
      std::move( CommandOption (
         "arg6", 'f', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 6 );
          })
      ).setDependencies( { "arg4" } ) ),
      std::move( CommandOption (
         "arg7", 'g', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 7 );
          })
      ).setDependencies( { "arg4", "arg6" } ) ),
      std::move( CommandOption (
         "arg8", 'h', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 8 );
          })
      ).setDependencies( { "arg3", "arg7", "arg9" } ) ),
      std::move( CommandOption (
         "arg9", 'i', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 9 );
          })
      ).setDependencies( { "arg3", "arg7" } )  )
    }
  };

  const char *testArgs[] {
    "command",
     "--arg6", "--arg1", "--arg8", "--arg4", "--arg7", "--arg5", "--arg3", "--arg2", "--arg9"
    ""
  };

  std::vector <int> expected { 1, 3, 2, 4, 5, 6, 7, 9, 8 };

  BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
  BOOST_CHECK_EQUAL ( values, expected );
}

BOOST_AUTO_TEST_CASE( circular_dependencies )
{
  /*

    arg1 --> arg2 --> arg3 --> arg4
     ^                          |
     |                          |
     +--------------------------+

  */
  std::vector <int> values;
  CommandGroup grp {
    {
      std::move( CommandOption (
         "arg1", 'a', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 1 );
          })
      ).setPriority( 3 ) )
       .setDependencies( { "arg2" } ),
      std::move( CommandOption (
         "arg2", 'b', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 2 );
          })
      ).setPriority( 1 ) )
       .setDependencies( { "arg3" } ),
      std::move( CommandOption (
         "arg3", 'c', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 3 );
          })
      ).setPriority( 2 ) )
       .setDependencies( { "arg4" } ),
      std::move( CommandOption (
         "arg4", 'd', NoArgument, CallbackVal(  [&] ( const CommandOption &, const boost::optional<std::string> & ) {
            values.push_back( 4 );
          })
      ).setDependencies( { "arg1" } ) )
    }
  };

  const char *testArgs[] {
    "command",
     "--arg1", "--arg4", "--arg3", "--arg2"
    ""
  };

  std::vector <int> expected { 1, 2, 3, 4 };

  BOOST_REQUIRE_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ), ZyppFlagsException ) ;
}

BOOST_AUTO_TEST_CASE( types )
{
  int integer = 0;
  std::string str;
  bool boolVal = false;
  int counter = 0;
  std::set<ResKind> kindSet;
  std::vector<std::string> strVec;
  zypp::Date date;

  CommandGroup grp {{{
    { "intArg", 'i', RequiredArgument, IntType( &integer )},
    { "strArg", 's', RequiredArgument, StringType( &str )},
    { "boolArg", 'b', NoArgument, BoolCompatibleType( boolVal, StoreTrue )},
    { "counterArg", 'c', NoArgument | Repeatable, CounterType( &counter, boost::optional<int>(), 5 )},
    { "kindSetArg", 'k', RequiredArgument | Repeatable, KindSetType( &kindSet) },
    { "strvecArg", 'v', RequiredArgument | Repeatable, StringVectorType( &strVec ) },
    { "dateArg", 'd', RequiredArgument, GenericValueType( date )}
  }}};

  {
    const char *testArgs[] {
      "command",
       "--intArg", "10"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( integer, 10 );
  }

  {
    const char *testArgs[] {
      "command",
       "--intArg", "error"
    };

    BOOST_REQUIRE_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ), InvalidValueException );
  }

  {
    const char *testArgs[] {
      "command",
       "--strArg", "test"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( str, "test" );
  }

  {
    const char *testArgs[] {
      "command",
       "--boolArg"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( boolVal, true );
  }

  {
    const char *testArgs[] {
      "command",
       "--counterArg",
       "--counterArg",
       "--counterArg"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( counter, 3 );
  }

  {
    const char *testArgs[] {
      "command",
      "--counterArg","--counterArg",
      "--counterArg","--counterArg",
      "--counterArg","--counterArg"
    };

    BOOST_REQUIRE_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ), ZyppFlagsException );
    //check that the counter was hit before increasing past the maximum value
    BOOST_CHECK_EQUAL ( counter, 5 );
  }

  {

    std::set<ResKind> expected {
      ResKind::package,
      ResKind::pattern,
      ResKind::product,
      ResKind::patch,
      ResKind::srcpackage,
      ResKind::application
    };

    const char *testArgs[] {
      "command",
      "--kindSetArg", "package",
      "--kindSetArg", "pattern",
      "--kindSetArg", "product",
      "--kindSetArg", "patch",
      "--kindSetArg", "srcpackage",
      "--kindSetArg", "application",
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( kindSet, expected );
  }

  {
    const char *testArgs[] {
      "command",
      "--kindSetArg", "unknown"
    };

    BOOST_REQUIRE_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ), InvalidValueException );
  }

  {
    std::vector<std::string> expected {
      "str1",
      "str2",
      "str3"
    };

    const char *testArgs[] {
      "command",
      "--strvecArg","str1",
      "--strvecArg","str2",
      "--strvecArg","str3"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    //check that the counter where hit at the 6th iteration
    BOOST_CHECK_EQUAL_COLLECTIONS( expected.begin(), expected.end(), strVec.begin(), strVec.end() );
  }

  {
    const char *testArgs[] {
      "command",
       "--dateArg", "2018-12-10"
    };

    BOOST_CHECK_NO_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ) );
    BOOST_CHECK_EQUAL ( Date( "2018-12-10", "%F" ), date );
  }

  {
    const char *testArgs[] {
      "command",
       "--dateArg", "2018-15-12"
    };

    BOOST_REQUIRE_THROW( parseCLI( sizeof(testArgs) / sizeof(char *),  ( char *const* )testArgs, { grp } ), InvalidValueException );
  }
}
