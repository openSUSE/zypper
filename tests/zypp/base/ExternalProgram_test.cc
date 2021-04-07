#include "TestSetup.h"
#include "TestTools.h"
#include <zypp/ExternalProgram.h>
#include <zypp/TmpPath.h>

#include <chrono>
#include <thread>
#include <sys/types.h>
#include <sys/wait.h>

#define BOOST_TEST_MODULE ExternalProgram
#define DATADIR (Pathname(TESTS_SRC_DIR) + "/zypp/base/data/ExternalProgram")

using zypp::ExternalProgram;

BOOST_AUTO_TEST_CASE( Basic )
{
  const char *argv[] = {
    "bash",
    "-c",
    "echo \"Hello\";sleep 1;echo \"I'm the second line\";exit 123;",
    nullptr
  };
  ExternalProgram proc( argv, ExternalProgram::Normal_Stderr );
  BOOST_CHECK( proc.running() );

  const auto pid = proc.getpid();
  BOOST_REQUIRE ( pid != -1 );
  std::string line = proc.receiveLine();
  BOOST_REQUIRE_EQUAL( line, std::string("Hello\n") );
  line = proc.receiveLine();
  BOOST_REQUIRE_EQUAL( line, std::string("I'm the second line\n") );
  int exitCode = proc.close();
  BOOST_REQUIRE( !proc.running() );
  BOOST_REQUIRE_EQUAL( proc.getpid(), -1 );
  BOOST_REQUIRE_EQUAL( exitCode, 123 );
  BOOST_REQUIRE_LT   ( ::getpgid( pid ) , 0 );
}

BOOST_AUTO_TEST_CASE( StderrToStdout )
{
  const char *argv[] = {
    "bash",
    "-c",
    "echo \"Hello\";sleep 1;echo \"I'm the second line\" >&2;",
    nullptr
  };
  ExternalProgram proc( argv, ExternalProgram::Stderr_To_Stdout );
  BOOST_CHECK( proc.running() );
  BOOST_REQUIRE ( proc.getpid() != -1 );
  std::string line = proc.receiveLine();
  BOOST_REQUIRE_EQUAL( line, std::string("Hello\n") );
  line = proc.receiveLine();
  BOOST_REQUIRE_EQUAL( line, std::string("I'm the second line\n") );
  proc.close();
  BOOST_REQUIRE( !proc.running() );
  BOOST_REQUIRE_EQUAL( proc.getpid(), -1 );
}

BOOST_AUTO_TEST_CASE( SetEnv )
{
  const char *argv[] = {
    "bash",
    "-c",
    "echo \"${ZYPPENV}\"",
    nullptr
  };
  ExternalProgram proc( argv, ExternalProgram::Environment{ std::make_pair("ZYPPENV", "Hello from env") }, ExternalProgram::Normal_Stderr );
  std::string line = proc.receiveLine();
  BOOST_REQUIRE_EQUAL( line, std::string("Hello from env\n") );
  proc.close();
}

// environment of the parent has to be in the child process as well
// some code paths rely on that
BOOST_AUTO_TEST_CASE( ParentEnv )
{
  const char *argv[] = {
    "bash",
    "-c",
    "echo \"${PARENTENV}${ZYPPENV}\"",
    nullptr
  };

  setenv( "PARENTENV", "Hello from", 0 );

  ExternalProgram proc( argv, ExternalProgram::Environment{ std::make_pair("ZYPPENV", " env") }, ExternalProgram::Normal_Stderr );
  std::string line = proc.receiveLine();
  BOOST_REQUIRE_EQUAL( line, std::string("Hello from env\n") );
  proc.close();

  unsetenv( "PARENTENV" );
}

// weird feature to redirect stdout and stderr
BOOST_AUTO_TEST_CASE( RedirectStdoutAndStderrWithChdir )
{
  zypp::filesystem::TmpDir dir;

  const zypp::Pathname stdoutPath( dir.path()/"stdout" );
  const zypp::Pathname stderrPath( dir.path()/"stderr" );

  const std::string stdoutRedir( zypp::str::form(">%s", stdoutPath.c_str() ) );
  const std::string chDir( zypp::str::form("#%s", dir.path().c_str() ) );
  zypp::AutoFD stdErrRedir( ::open( stderrPath.c_str() , O_CREAT | O_RDWR, 0666 ) );

  BOOST_REQUIRE( *stdErrRedir != -1 );

  const char *argv[] = {
    chDir.c_str(),
    stdoutRedir.c_str(),
    "bash",
    "-c",
    "echo \"Hello on stdout\";echo \"Hello on stderr\" >&2;echo \"Hello on file in chdir\" > chdirfile",
    nullptr
  };

  ExternalProgram proc (argv, ExternalProgram::Stderr_To_FileDesc, false, *stdErrRedir );

  int exitCode = proc.close();
  BOOST_REQUIRE( !proc.running() );
  BOOST_REQUIRE_EQUAL( exitCode, 0 );

  ::close( *stdErrRedir );
  *stdErrRedir = -1;

  BOOST_REQUIRE_EQUAL( TestTools::readFile( stdoutPath ), "Hello on stdout\n");
  BOOST_REQUIRE_EQUAL( TestTools::readFile( stderrPath ), "Hello on stderr\n");
  BOOST_REQUIRE_EQUAL( TestTools::readFile( dir.path() / "chdirfile" ), "Hello on file in chdir\n");
}

BOOST_AUTO_TEST_CASE( LongRunningEcho )
{
  const auto executable = DATADIR/"echo.sh";
  const char *argv[] = {
    executable.c_str(),
    nullptr
  };

  ExternalProgram proc (argv, ExternalProgram::Normal_Stderr );
  BOOST_REQUIRE( proc.running() );
  BOOST_REQUIRE( proc.send( "Hello" ) );
  BOOST_REQUIRE( proc.send( " my friend\n" ) );
  BOOST_REQUIRE( proc.send( "How are you today\n" ) );
  BOOST_REQUIRE_EQUAL( proc.receiveLine(), "Hello my friend\n" );
  BOOST_REQUIRE_EQUAL( proc.receiveLine(), "How are you today\n" );

  BOOST_REQUIRE( proc.running() );
  BOOST_REQUIRE( proc.send( "q\n" ) );
  const auto res = proc.close();
  BOOST_REQUIRE_EQUAL( res, 0 );
}

BOOST_AUTO_TEST_CASE( SendSignal )
{
  const auto executable = DATADIR/"echo.sh";
  const char *argv[] = {
    executable.c_str(),
    nullptr
  };

  ExternalProgram proc (argv, ExternalProgram::Normal_Stderr );
  BOOST_REQUIRE( proc.running() );
  proc.kill( SIGTERM );
  const auto exitCode = proc.close();
  BOOST_REQUIRE_EQUAL( exitCode, SIGTERM+128 );
}

BOOST_AUTO_TEST_CASE( CleanerThread_default )
{
  pid_t pid = -1;
  {
    ExternalProgram proc( "bash -c 'sleep 2'", ExternalProgram::Normal_Stderr );
    BOOST_CHECK( proc.running() );
    pid = proc.getpid();
  }
  std::this_thread::sleep_for( std::chrono::seconds(4) );
  // check if the process is really gone, can't use waitpid because some ExternalProgram
  // implementations do spawn processes from a helper thread which means they are not our direct children
  BOOST_CHECK_EQUAL( ::getpgid( pid ), -1 );
  BOOST_CHECK_EQUAL( errno, ESRCH );
}

BOOST_AUTO_TEST_CASE( ReadTimeout )
{
  static const char* argv[] = { "sleep", "2", NULL };
  ExternalProgram prog( argv, ExternalProgram::Discard_Stderr );
  BOOST_CHECK_THROW( prog.receiveLine( 100 ), io::TimeoutException );
  BOOST_CHECK_EQUAL( prog.receiveLine( 3000 ), "" );
  BOOST_CHECK_EQUAL( prog.close(), 0 );
}
