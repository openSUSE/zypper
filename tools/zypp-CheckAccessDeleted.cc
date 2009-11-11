#include <cstdio>
#include <iostream>
#include <zypp/base/Easy.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/Pathname.h>
#include <zypp/misc/CheckAccessDeleted.h>

/** Collect max string size. */
struct TableCol
{
  TableCol( size_t size_r = 0 )
  : size( size_r )
  {}
  TableCol( const std::string & header_r )
  : header( header_r ), size( header_r.size() )
  {}
  void operator()( const std::string & val_r )
  { if ( val_r.size() > size ) size = val_r.size(); }

  std::string header;
  size_t      size;
};

/** Scan to determine column sizes, then print. */
struct ProcInfoTable
{
  ProcInfoTable()
  : pid		( "PID" )
  , ppid	( "PPID" )
  , puid	( "UID" )
  , login	( "LOGIN" )
  , command	( "COMMAND" )
  , service	( "SERVICE" )
  , files	( "FILES" )
  {}

  void scan( const zypp::CheckAccessDeleted::ProcInfo & val_r )
  {
    pid( val_r.pid );
    ppid( val_r.ppid );
    puid( val_r.puid );
    login( val_r.login );
    command( val_r.command );
    service( val_r.service() );
  }

  void printHeader() const
  {
    printRow( pid.header,
              ppid.header,
              puid.header,
              login.header,
              command.header,
              service.header,
              files.header );
  }

  void print( const zypp::CheckAccessDeleted::ProcInfo & val_r ) const
  {
    printRow( val_r.pid,
              val_r.ppid,
              val_r.puid,
              val_r.login,
              val_r.command,
              val_r.service(),
              zypp::str::join( val_r.files, ", " ) );
  }

  void printRow( const std::string & pid_r,
                 const std::string & ppid_r,
                 const std::string & puid_r,
                 const std::string & login_r,
                 const std::string & command_r,
                 const std::string & service_r,
                 const std::string & files_r ) const
  {
    printf( "%*s %*s %*s  %-*s %-*s %-*s %-s\n",
            (int)pid.size, pid_r.c_str(),
            (int)ppid.size, ppid_r.c_str(),
            (int)puid.size, puid_r.c_str(),
            (int)login.size, login_r.c_str(),
            (int)command.size, command_r.c_str(),
            (int)service.size, (service_r.empty() ? " -" : service_r.c_str()),
            files_r.c_str() );
  }

  TableCol pid;
  TableCol ppid;
  TableCol puid;
  TableCol login;
  TableCol command;
  TableCol service;
  TableCol files;
};

int main( int argc, char * argv[] )
{
  if ( argc >= 2 )
  {
    std::string progname( zypp::Pathname::basename( argv[0] ) );
    if ( strcmp( argv[1], "--help" ) == 0 )
    {
      std::cout << "Usage: " << progname << " [--help]" << std::endl;
      std::cout << "List information about all running processe" << std::endl;
      std::cout << "which access deleted executables or libraries." << std::endl;
      std::cout << "  PID     " << "process ID" << std::endl;
      std::cout << "  PPID    " << "parent process ID" << std::endl;
      std::cout << "  UID     " << "process user ID" << std::endl;
      std::cout << "  LOGIN   " << "process login name" << std::endl;
      std::cout << "  COMMAND " << "process command name" << std::endl;
      std::cout << "  SERVICE " << "/etc/init.d/ script that might be used to restart the command (guessed)" << std::endl;
      std::cout << "  FILES   " << "list of deleted executables or libraries accessed" << std::endl;
      return 0;
    }
    std::cerr << progname << ": unexpected argument '" << argv[1] << "'" << std::endl;
    std::cerr << "Try `" << progname << " --help' for more information." << std::endl;
    return 1;
  }

  zypp::CheckAccessDeleted checker(false); // wait for explicit call to check()
  try {
    checker.check( /*verbose*/true );
  }
  catch( const zypp::Exception & err )
  {
    std::cerr << err << std::endl << err.historyAsString();
    return 2;
  }

  ProcInfoTable table;
  for_( it, checker.begin(), checker.end() )
    table.scan( *it );

  table.printHeader();
  for_( it, checker.begin(), checker.end() )
    table.print( *it );

  return 0;
}
