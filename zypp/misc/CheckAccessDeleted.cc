/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/misc/CheckAccessDeleted.cc
 *
*/
#include <iostream>
#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/Exception.h"

#include "zypp/PathInfo.h"
#include "zypp/ExternalProgram.h"

#include "zypp/misc/CheckAccessDeleted.h"

using std::endl;

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::misc"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////
    //
    // lsof output lines are a sequence of NUL terminated fields,
    // where the 1st char determines the fiels type.
    //
    // (pcuL) pid command userid loginname
    // (ftkn).filedescriptor type linkcount filename
    //
    /////////////////////////////////////////////////////////////////
    /** Add \c cache to \c data if the process is accessing deleted files.
     * \c pid string in \c cache is the proc line \c (pcuLR), \c iles
     * are lready in place. Always clear the \c cache.files!
    */
    inline void addDataIf( std::vector<CheckAccessDeleted::ProcInfo> & data_r, CheckAccessDeleted::ProcInfo & cache_r )
    {
      if ( cache_r.files.empty() )
        return;

      // at least one file access so keep it:
      data_r.push_back( CheckAccessDeleted::ProcInfo() );
      CheckAccessDeleted::ProcInfo & pinfo( data_r.back() );

      std::string pline;
      cache_r.pid.swap( pline );
      cache_r.files.swap( pinfo.files ); // clears cache.files

      for_( ch, pline.begin(), pline.end() )
      {
        switch ( *ch )
        {
          case 'p':
            pinfo.pid = &*(ch+1);
            break;
          case 'R':
            pinfo.ppid = &*(ch+1);
            break;
          case 'u':
            pinfo.puid = &*(ch+1);
            break;
          case 'L':
            pinfo.login = &*(ch+1);
            break;
          case 'c':
            pinfo.command = &*(ch+1);
            break;
        }
        if ( *ch == '\n' ) break;		// end of data
        do { ++ch; } while ( *ch != '\0' );	// skip to next field
      }

      if ( pinfo.command.size() == 15 )
      {
        // the command name might be truncated, so we check against /proc/<pid>/exe
        Pathname command( filesystem::readlink( Pathname("/proc")/pinfo.pid/"exe" ) );
        if ( ! command.empty() )
          pinfo.command = command.basename();
      }

      MIL << " Take " << pinfo << endl;
    }

    /** Add line to cache if it refers to a deleted file.
     * Either the link count \c(k) os \c 0, or if no link cout is present,
     * the filedescriptor/type \c (ft) is set to \c DEL.
    */
    inline void addCacheIf( CheckAccessDeleted::ProcInfo & cache_r, const std::string & line_r )
    {
      bool takeLine = false;
      const char * name = "";

      for_( ch, line_r.begin(), line_r.end() )
      {
        switch ( *ch )
        {
          case 'k':
             if ( ! takeLine && *(ch+1) == '0' && *(ch+2) == '\0' )
               takeLine = true;
           break;
          case 'f':
          case 't':
            if ( ! takeLine && *(ch+1) == 'D' && *(ch+2) == 'E' && *(ch+3) == 'L' && *(ch+4) == '\0' )
              takeLine = true;
            break;
          case 'n':
             name = &*(ch+1);
             break;
        }
        if ( *ch == '\n' ) break;		// end of data
        do { ++ch; } while ( *ch != '\0' );	// skip to next field
      }
      if ( takeLine )
        cache_r.files.push_back( name );
    }
    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  CheckAccessDeleted::size_type CheckAccessDeleted::check()
  {
    _data.clear();
    std::vector<ProcInfo> data;

    static const char* argv[] =
    {
      "lsof", "-n", "-FpcuLRftkn0", NULL
    };
    ExternalProgram prog( argv, ExternalProgram::Discard_Stderr );

    CheckAccessDeleted::ProcInfo cache;
    for( std::string line = prog.receiveLine(); ! line.empty(); line = prog.receiveLine() )
    {
      if ( line[0] == 'p' )
      {
        addDataIf( data, cache );
        cache.pid = line; //
      }
      else
      {
        addCacheIf( cache, line );
      }
    }
    addDataIf( data, cache );

    int ret = prog.close();
    if ( ret != 0 )
    {
      Exception err( str::form("Executing 'lsof' failed (%d).", ret) );
      err.remember( prog.execError() );
      ZYPP_THROW( err );
    }

    _data.swap( data );
    return _data.size();
  }

  std::string CheckAccessDeleted::findService( const Pathname & command_r )
  {
    ProcInfo p;
    p.command = command_r.basename();
    return p.service();
  }
  std::string CheckAccessDeleted::findService( const char * command_r )
  { return findService( Pathname( command_r ) ); }

  std::string CheckAccessDeleted::findService( const std::string & command_r )
  { return findService( Pathname( command_r ) ); }

  std::string CheckAccessDeleted::findService( pid_t pid_r )
  { return findService( filesystem::readlink( Pathname("/proc")/str::numstring(pid_r)/"exe" ) ); }

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  std::string CheckAccessDeleted::ProcInfo::service() const
  {
    if ( command.empty() )
      return std::string();
    // TODO: This needs to be implemented smarter... be carefull
    // as we don't know whether the target is up.

    static const Pathname initD( "/etc/init.d" );
    { // init.d script with same name
      PathInfo pi( initD/command );
      if ( pi.isFile() && pi.isX() )
        return command;
    }
    { // init.d script with name + 'd'
      std::string alt( command+"d" );
      PathInfo pi( initD/alt );
      if ( pi.isFile() && pi.isX() )
        return alt;
    }
    if ( *command.rbegin() == 'd' )
    { // init.d script with name - trailing'd'
      std::string alt( command );
      alt.erase( alt.size()-1 );
      PathInfo pi( initD/alt );
      WAR <<pi << endl;
      if ( pi.isFile() && pi.isX() )
        return alt;
    }
    return std::string();
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CheckAccessDeleted & obj )
  {
    return dumpRange( str << "CheckAccessDeleted ",
                      obj.begin(),
                      obj.end() );
  }

   /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const CheckAccessDeleted::ProcInfo & obj )
  {
    if ( obj.pid.empty() )
      return str << "<NoProc>";

    return dumpRangeLine( str << obj.command
                              << '<' << obj.pid
                              << '|' << obj.ppid
                              << '|' << obj.puid
                              << '|' << obj.login
                              << '>',
                          obj.files.begin(),
                          obj.files.end() );
  }

 /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
