#include <ctime>
#include <cerrno>
#include <iostream>
#include <fstream>

#include "Printing.h"
#include <zypp/base/IOStream.h>
#include <zypp/ExternalProgram.h>


using namespace std;
using namespace zypp;

///////////////////////////////////////////////////////////////////

bool systemReadLine( ExternalProgram & processx, string & line )
{
  ExternalProgram * process = &processx;
    line.erase();

    if ( process == NULL )
	return false;

    if ( process->inputFile() )
      {
        process->setBlocking( false );
        FILE * inputfile = process->inputFile();
        int    inputfileFd = ::fileno( inputfile );
        do
        {
          /* Watch inputFile to see when it has input. */
          fd_set rfds;
          FD_ZERO( &rfds );
          FD_SET( inputfileFd, &rfds );

          /* Wait up to 5 seconds. */
          struct timeval tv;
          tv.tv_sec = 5;
          tv.tv_usec = 0;

          int retval = select( inputfileFd+1, &rfds, NULL, NULL, &tv );

          if ( retval == -1 )
            {
              ERR << "select error: " << strerror(errno) << endl;
              if ( errno != EINTR )
                return false;
            }
          else if ( retval )
            {
              // Data is available now.
              size_t linebuffer_size = 0;
              char * linebuffer = 0;
              ssize_t nread = getline( &linebuffer, &linebuffer_size, inputfile );
              DBG << "getline " << nread << " " << ::feof( inputfile ) << " " << ::ferror( inputfile ) << endl;

              if ( nread == -1 )
                {
                  if ( ::feof( inputfile ) )
                    return line.size(); // in case of pending output
                }
              else
                {
                  if ( nread > 0 )
                    {
                      if ( linebuffer[nread-1] == '\n' )
                        --nread;
                      line += string( linebuffer, nread );
                    }

                  if ( ! ::ferror( inputfile ) || ::feof( inputfile ) )
                    return true;
                }
              clearerr( inputfile );
            }
          else
            {
              // No data within time.
              if ( ! process->running() )
                return false;
            }

        } while ( true );
        DBG << "----------------------------------------" << endl;
      }

    return false;
}

int systemStatus( ExternalProgram & process )
{
  int exit_code = process.close();
  process.kill();

  MIL << "EXIT CODE " << exit_code << endl;
  return exit_code;
}
/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "xxx" );
  INT << "===[START]==========================================" << endl;

  const char* args[] = {
    "./xx",
    NULL
  };

  // Launch the program with default locale
  ExternalProgram process( args, ExternalProgram::Discard_Stderr, false, -1, true );
  sleep( 5 );
  string line;
  while ( systemReadLine( process, line ) )
    {
      INT << '{' << line << '}' << endl;
    }
  systemStatus( process );

  INT << "===[END]============================================" << endl << endl;
  return 0;
}
