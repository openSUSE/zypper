/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/ExternalDataSource.cc
 */

#define _GNU_SOURCE 1 // for ::getline

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <string>

#include <zypp-core/base/Logger.h>
#include <zypp-core/base/ExternalDataSource.h>
#include <zypp-core/AutoDispose.h>

using std::endl;

namespace zypp {
  namespace externalprogram {

    ExternalDataSource::ExternalDataSource( FILE *ifile, FILE *ofile )
      : inputfile( ifile ),
        outputfile( ofile ),
        linebuffer( 0 ),
        linebuffer_size( 0 )
    {
    }


    ExternalDataSource::~ExternalDataSource()
    {
      if (linebuffer)
    	free( linebuffer );
      close ();
    }


    bool
    ExternalDataSource::send( const char *buffer, size_t length )
    {
      if ( outputfile ) {
    	bool success = fwrite( buffer, length, 1, outputfile ) != 0;
    	fflush( outputfile );
    	return success;
      }
      else
    	return false;
    }


    bool
    ExternalDataSource::send( std::string s )
    {
      DBG << "send (" << s << ")";
      return send( s.data(), s.length() );
    }


    std::string
    ExternalDataSource::receiveUpto( char c )
    {
      if ( inputfile && !feof( inputfile ) )
      {
    	std::ostringstream datas;
	 while ( true )
	 {
	   int readc = fgetc( inputfile );
	   if ( readc == EOF ) break;
	   datas << (char)readc;
	   if ( (char)readc == c ) break;
	 }
	 return datas.str();
      }
      return std::string();
    }

    std::string ExternalDataSource::receiveUpto( char c, io::timeout_type timeout )
    {
      const auto &received = io::receiveUpto( inputFile(), c, timeout );

      if ( received.first == io::Timeout )
        ZYPP_THROW( io::TimeoutException() );

      return received.second;
    }

    size_t
    ExternalDataSource::receive( char *buffer, size_t length )
    {
      if ( inputfile )
    	return fread( buffer, 1, length, inputfile );
      else
    	return 0;
    }

    void ExternalDataSource::setBlocking( bool mode )
    {
      io::setFILEBlocking( inputfile, mode );
    }

    std::string
    ExternalDataSource::receiveLine()
    {
      if ( inputfile )
      {
    	ssize_t nread = getline( &linebuffer, &linebuffer_size, inputfile );
    	if ( nread == -1 )
    	    return "";
    	else
    	    return std::string( linebuffer, nread );
      }
      else
        return "";
    }

    std::string ExternalDataSource::receiveLine( io::timeout_type timeout )
    {
      return receiveUpto( '\n', timeout );
    }

    int
    ExternalDataSource::close()
    {
      if ( inputfile && inputfile != outputfile )
    	fclose( inputfile );
      if ( outputfile )
    	fclose( outputfile );
      inputfile = 0;
      outputfile = 0;
      return 0;
    }

  } // namespace externalprogram
} // namespace zypp

