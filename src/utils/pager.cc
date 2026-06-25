/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <errno.h>
#include <sys/wait.h> //for wait()
#include <iterator>
#include <vector>     // Added for argv handling

#include <zypp-core/base/Logger.h>
#include <zypp-core/base/String.h>
#include <zypp/TmpPath.h>
#include <zypp-core/Pathname.h>
#include <zypp/PathInfo.h>

#include "../main.h"
#include "Zypper.h"

#include "pager.h"

// ---------------------------------------------------------------------------

static std::string pager_help_exit( const std::string & pager )
{
  std::string ret;
  if ( pager.size() >= 4 && pager.substr( pager.size()-4, 4 ) == "less" )
    ret = str::form(_("Press '%c' to exit the pager."), 'q' );
  return ret;
}

// ---------------------------------------------------------------------------

static std::string pager_help_navigation( const std::string & pager )
{
  std::string ret;
  if ( pager.rfind("less") != std::string::npos )
    ret = _("Use arrows or pgUp/pgDown keys to scroll the text by lines or pages.");
  else if ( pager.rfind("more") != std::string::npos )
    ret = _("Use the Enter or Space key to scroll the text by lines or pages.");
  return ret;
}

// ---------------------------------------------------------------------------

static bool show_in_pager( const std::string & pager, const Pathname & file )
{
  if ( Zypper::instance().config().non_interactive )
    return true;

  // Safely tokenize the pager command to handle whitespaces and flags (e.g., "less -R")
  std::vector<std::string> args;
  std::istringstream iss(pager);
  std::string token;
  while (iss >> token)
  {
    args.push_back(token);
  }

  if (args.empty())
  {
    WAR << "Pager command is empty" << endl;
    return false;
  }

  // Preserve the binary path/name for the hint helpers before adding the target file
  std::string pager_binary = args[0];

  // Append target file path safely as a standalone argument
  args.push_back(file.asString());

  // Convert std::vector<std::string> to a null-terminated char* array for execvp
  std::vector<char*> argv;
  for (auto &arg : args)
  {
    argv.push_back(&arg[0]);
  }
  argv.push_back(nullptr);

  pid_t pid;
  switch( pid = fork() )
  {
  case -1:
    WAR << "fork failed" << endl;
    return false;

  case 0:
    // Execute directly without a shell interpreter wrap
    execvp( argv[0], argv.data() );
    WAR << "execvp failed with " << strerror(errno) << endl;
    // exit, cannot return false here, because this is another process
    exit(ZYPPER_EXIT_ERR_BUG);

  default:
    DBG << "Executed pager process (pid: " << pid << ")" << endl;

    // wait until pager exits
    int status = 0;
    int ret;
    do
    {
      ret = waitpid( pid, &status, 0 );
    }
    while ( ret == -1 && errno == EINTR );

    if ( WIFEXITED (status) )
    {
      status = WEXITSTATUS( status );
      if ( status )
      {
        DBG << "Pid " << pid << " exited with status " << status << endl;
        return false;
      }
      else
        DBG << "Pid " << pid << " successfully completed" << endl;
    }
    else if ( WIFSIGNALED (status) )
    {
      status = WTERMSIG( status );
      WAR << "Pid " << pid << " was killed by signal " << status
          << " (" << strsignal(status);
      if ( WCOREDUMP (status) )
        WAR << ", core dumped";
      WAR << ")" << endl;
      return false;
    }
    else
    {
      ERR << "Pid " << pid << " exited with unknown error" << endl;
      return false;
    }
  }
  return true;
}

// ---------------------------------------------------------------------------

bool show_text_in_pager( const std::string & text, const std::string & intro )
{
  std::string pager( "more" );	// basic posix default, must be in PATH
  const char* envpager = ::getenv("PAGER");
  if ( envpager && *envpager )
    pager = envpager;

  // Extract just the binary name/path token for accurate help text hints
  std::string pager_binary;
  std::istringstream iss(pager);
  iss >> pager_binary;

  filesystem::TmpFile tfile;
  std::ofstream os( tfile.path().c_str() );

  // intro
  if (!intro.empty())
    os << intro << endl;

  // navigation hint - pass the exact binary name token
  std::string help( pager_help_navigation( pager_binary ) );
  if ( !help.empty() )
    os << "(" << help << ")" << endl << endl;

  // the text
  os << text;

  // exit hint - pass the exact binary name token
  help = pager_help_exit( pager_binary );
  if ( !help.empty() )
    os << endl << endl << "(" << help << ")";
  os.close();

  return show_in_pager( pager, tfile );
}

// ---------------------------------------------------------------------------

bool show_file_in_pager( const Pathname & file, const std::string & intro )
{
  std::string pager( "more" );	// basic posix default, must be in PATH
  const char* envpager = ::getenv("PAGER");
  if ( envpager && *envpager )
    pager = envpager;

  // Extract just the binary name/path token for accurate help text hints
  std::string pager_binary;
  std::istringstream iss(pager);
  iss >> pager_binary;

  filesystem::TmpFile tfile;
  std::ofstream os( tfile.path().c_str() );

  // intro
  if ( !intro.empty() )
    os << intro << endl;

  // navigation hint - pass the exact binary name token
  std::string help( pager_help_navigation( pager_binary ) );
  if ( !help.empty() )
    os << "(" << help << ")" << endl << endl;

  // the text
  std::ifstream is( file.c_str() );
  if ( is.good() )
    os << is.rdbuf();
  else
  {
    cerr << "ERR reading the file" << endl;
    is.close();
    return false;
  }
  is.close();

  // exit hint - pass the exact binary name token
  help = pager_help_exit( pager_binary );
  if ( !help.empty() )
    os << endl << endl << "(" << help << ")";
  os.close();

  return show_in_pager( pager, tfile );
}

// vim: set ts=2 sts=2 sw=2 et ai:

